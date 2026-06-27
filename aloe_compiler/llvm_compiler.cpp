#include "pch.h"
#include "lang\compiler.h"
#include "base\defs.h"
#include "base\scope_guard.h"
#include "lang\aloe_exception.h"
#include "utils.h"
#include "llvm_compiler\compiler.h"
#include "i64_platform.h"
#include "llvm_compiler.h"

using namespace aloe;
using namespace llvm;
using namespace llvm::dwarf;


compiler_ptr_t
aloe::create_llvm_compiler()
{
    return compiler_ptr_t(new llvmir_compiler_t());
}

llvmir_compiler_t::llvmir_compiler_t()
{
	validate = true;
}

void 
llvmir_compiler_t::set_validate(bool validate)
{
    this->validate = validate;
}

bool 
llvmir_compiler_t::compile(
    ast_ptr_t ast,
	ostream& out)
{
    LLVMContext ctx;
    Module module(ast->source_id, ctx);

    DIBuilder dib(module);
    std::filesystem::path pth(ast->source_id);
    DIFile* di_file = dib.createFile(pth.filename().string(), pth.parent_path().string());

    DICompileUnit* cu = dib.createCompileUnit(
        DW_LANG_C,   
        di_file,
        "aloe-frontend",    // producer
        false,              // isOptimized
        "",                 // flags
        0                   // runtime version
    );

    module.addModuleFlag(Module::Warning, "CodeView", 1);
    module.addModuleFlag(Module::Warning, "Dwarf Version", 4);
    module.addModuleFlag(Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);

    IRBuilder<> ir(ctx);
    
    compiler_ctx_ptr_t compiler_ctx(new llvm_ctx_modifier_t(&ctx, &module, &ir, &dib, di_file, cu));
    compiler_ctx_ptr_t ast_ctx(new ast_ctx_modifier_t(ast, compiler_ctx));


	di_cache = make_shared<di_cache_t>(dib);


	bool res = false;
    try
    {
        walk_prog(ast_ctx, ast->prog);
		res = true;

        dib.finalize();

        // Print LLVM IR
        llvm::raw_os_ostream  llvmOs(out);
        module.print(llvmOs, nullptr);
    }
    catch (std::exception& e)
    {
        loginl("%s", e.what());
    }

    return res;
}

Type* 
llvmir_compiler_t::emit_type(compiler_ctx_ptr_t ctx, type_node_ptr_t node)
{
    init_dloc(ctx, node);
    return emit_type(ctx, node->type);
}


Type*
llvmir_compiler_t::emit_type(compiler_ctx_ptr_t ctx, aloe_type_ptr_t type)
{
    Type* out = nullptr;

    switch (type->type_id)
    {
    case ALOE_TYPE_INT:
    {
        out = Type::getInt64Ty(*ctx->ctx());
        
        break;
    }
    case ALOE_TYPE_CHAR:
    {
        out = Type::getInt8Ty(*ctx->ctx());

        break;
    }
    case ALOE_TYPE_VOID:
    {
        out = Type::getVoidTy(*ctx->ctx());

        break;
    }
    case ALOE_TYPE_DOUBLE:
    {
        out = Type::getDoubleTy(*ctx->ctx());

        break;
    }
    case ALOE_TYPE_PTR:
    {
        out = PointerType::getUnqual(*ctx->ctx()); // collapse pointer types, we will preserve the original type in debug info for each level of indirection

        break;
    }
    case ALOE_TYPE_FUNCTION:
    {
        Type* ret_type = emit_type(ctx, type->fun_ret_type);

        std::vector<Type*>  irt_args;
        for (auto p : type->fun_param_types)
        {
            Type* argt = emit_type(ctx, p);
            irt_args.push_back(argt);
        };

        out  = FunctionType::get(ret_type, irt_args, false);
        break;

    }
    default:
    {
        assert(false && "(internal compiler error): unknown type id");
    }
    };

    return out;
}


value_ptr_t
llvmir_compiler_t::emit_fun(compiler_ctx_ptr_t ctx, fun_node_ptr_t node)
{
    if (node->ignore)
        value_ptr_t();

    value_ptr_t out(new value_t());

    Type *ir_fun_type   = emit_type(ctx, node->type_node);
	out->di_type        = di_cache->get_dit_type(node->type);
  
    Function* ir_fun =
        Function::Create(ir_sc<FunctionType>(ir_fun_type),
            Function::ExternalLinkage, node->id->name, ctx->module());
   
    DISubprogram* sp = ctx->di_builder()->createFunction(
        ctx->di_file(),         // Function scope.  
        node->id->name,            // Function name.
        node->id->name,            // Mangled function name.
        ctx->di_file(),         // File where this variable is defined.
        node->line,                // Line number.
		ir_sc<DISubroutineType>(di_cache->get_dit_type(node->type)), // type
        node->line,                 // scope line
        DINode::FlagZero,
		node->is_defined ? DISubprogram::SPFlagDefinition : DISubprogram::SPFlagZero
     );

    ir_fun->setSubprogram(sp);

    out->ir_value   = ir_fun;
    id_cache[node]  = out;
	
    if (node->is_defined)
    {
        // new scope for function body
        compiler_ctx_ptr_t new_ctx(new fun_ctx_modifier_t(ir_fun, ctx));
        
		emit_fun_definition(new_ctx, ir_fun, node);
       
    }
  
    bool is_broken = llvm::verifyFunction(*ir_fun);
    
    if (is_broken && validate) {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): generated IR for function '%s' is broken",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos,
            node->id->name.c_str());
    }

    return out;
		
}

void 
llvmir_compiler_t::emit_fun_definition(compiler_ctx_ptr_t ctx, Function* fun, fun_node_ptr_t node)
{
    init_dloc(ctx, node);

    BasicBlock* ir_block = BasicBlock::Create(*ctx->ctx(), node->id->name, fun);

    ctx->builder()->SetInsertPoint(ir_block);

    for (int i = 0; i < fun->arg_size(); i++)
    {
        Argument* ir_arg = fun->getArg(i);

        auto arg_node = node->type_node->fun_params_node->vars_v[i].second;
        

        // temporary storage for better debuggability and mutability of parameters
        auto* arg_slot = ctx->builder()->CreateAlloca(ir_arg->getType());
        ctx->builder()->CreateStore(ir_arg, arg_slot);

        if (arg_node->id)
        {
            auto var_name = arg_node->id->name;
            ir_arg->setName(var_name);

            auto arg_dvar = ctx->di_builder()->createParameterVariable(
                get_scope(ctx),
                arg_node->id->name,
                i + 1,
                ctx->di_file(),
                node->line,
                di_cache->get_dit_type(arg_node->type),
                true
            );

            ctx->di_builder()->insertDeclare(
                arg_slot,
                arg_dvar,
                ctx->di_builder()->createExpression(),
                llvm::DILocation::get(*ctx->ctx(), node->line, node->pos, get_scope(ctx)),
                ir_block
            );
        }


        value_ptr_t arg_val(new value_t());
        arg_val->ir_value = arg_slot;
        arg_val->is_lvalue = true;
        arg_val->lval_type = ir_arg->getType();
        arg_val->aloe_type = arg_node->type;

        id_cache[arg_node] = arg_val;

    }

    // emit fucntion statements
    for (auto& statement : node->exec_block->exec_statements)
    {
        emit_exec_statement(ctx, statement);
    }

    // emit terminator if not present
    auto terminator = ctx->builder()->GetInsertBlock()->getTerminator();
    if (!terminator)
    {
        if (fun->getReturnType()->isVoidTy()) {
            init_dloc(ctx, node->end_of_fun);
            ctx->builder()->CreateRetVoid();
        }
        else
        {
            throw aloe_exception_t("%s:%zu:%zu: error: control reaches end of non-void function '%s'",
                ctx->ast()->source_id.c_str(),
                node->line,
                node->pos,
                node->id->name.c_str());
        }
    }

}

void 
llvmir_compiler_t::emit_return(compiler_ctx_ptr_t ctx, return_node_ptr_t node)
{
    init_dloc(ctx, node);

	llvm::ReturnInst* ret_inst = nullptr;

    if (!node->return_expr)
    {
         ret_inst = ctx->builder()->CreateRetVoid();
    }
    else
    {
        value_ptr_t ret_val = emit_expression(ctx, node->return_expr);

        ret_inst = ctx->builder()->CreateRet(emit_r_value(ctx, ret_val));
    }

}

void 
llvmir_compiler_t::walk_prog(compiler_ctx_ptr_t ctx, prog_node_ptr_t node)
{
    init_dloc(ctx, node);

    for (auto& decl : node->decl_statements)
    {
        switch (decl->node_type_id)
        {
        case FUNCTION_NODE:
            emit_fun(ctx, PCAST(fun_node_t,decl));
			break;
        case VAR_NODE:
            emit_var(ctx, PCAST(var_node_t, decl));
            break;
        }
    }

}

value_ptr_t
llvmir_compiler_t::emit_expr_identifier(compiler_ctx_ptr_t ctx, identifier_expr_node_ptr_t node)
{
    init_dloc(ctx, node);

	value_ptr_t out(new value_t());

	switch (node->ast_def->target->node_type_id)
    {
        case FUNCTION_NODE:
        {
            out = PCAST(value_t, id_cache[node->ast_def->target]);

            break;
        }
        case VAR_NODE:
        {
            out = PCAST(value_t, id_cache[node->ast_def->target]);

            break;
        }
        default:
        {
            throw aloe_exception_t("%s:%zu:%zu: (internal error): identifier '%s' points to unsupported type",
                ctx->ast()->source_id.c_str(),
                node->line,
                node->pos,
				node->id->name.c_str());
        }
    }

    return out;
}

void 
llvmir_compiler_t::emit_exec_statement(compiler_ctx_ptr_t ctx, node_ptr_t node)
{
    init_dloc(ctx, node);

    switch (node->node_type_id)
    {
        case EXPRESSION_NODE:
        {
			emit_expression(ctx, PCAST(expr_node_t,node));
            break;
		}
        case VAR_NODE:
        {
            emit_var(ctx, PCAST(var_node_t, node));
            break;
        }
        case RETURN_NODE:
        {
            emit_return(ctx, PCAST(return_node_t,node));
            break;
        }
    default:
        break;
    }
}

value_ptr_t 
llvmir_compiler_t::emit_default(compiler_ctx_ptr_t ctx, aloe_type_ptr_t type)
{
	value_ptr_t out(new value_t());

    out->is_lvalue = false;
    out->ir_value  = Constant::getNullValue(emit_type(ctx, type));
    out->di_type   = di_cache->get_dit_type(type);
   
    return out;

}


void 
llvmir_compiler_t::emit_var(compiler_ctx_ptr_t ctx, var_node_ptr_t node)
{
    init_dloc(ctx, node);
	value_ptr_t out(new value_t());

    Type *ir_var_type  = emit_type(ctx, node->type_node);
    out->lval_type = ir_var_type;

	auto init_val = node->initializer ? emit_expression(ctx, node->initializer) : emit_default(ctx, node->type);

    if (!ctx->curr_fun())
    {
        out->ir_value = new GlobalVariable(
            *ctx->module(),
            ir_var_type,
            false,
            GlobalValue::ExternalLinkage,
            ir_sc <Constant>(init_val->ir_value),
			node->id->name
            );

        out->is_lvalue = true;
		
    }
    else
    {
        auto alloca_inst = ctx->builder()->CreateAlloca(ir_var_type, nullptr, node->id->name);
        out->ir_value  = alloca_inst;
		out->is_lvalue = true;
	
        ctx->builder()->CreateStore(emit_r_value(ctx, init_val), out->ir_value);
        
    }

	id_cache[node] = out;
   
}

value_ptr_t 
llvmir_compiler_t::emit_arithmetic_binary(compiler_ctx_ptr_t ctx, binary_expr_node_ptr_t node)
{
    llvm::DebugLoc dloc = init_dloc(ctx, node);

    value_ptr_t e1 = emit_expression(ctx, node->operand1);
    value_ptr_t e2 = emit_expression(ctx, node->operand2);
   
	return emit_raw_binary_arithmetic(ctx, node->op_id, e1, e2, node);
}

value_ptr_t 
llvmir_compiler_t::emit_raw_binary_arithmetic(compiler_ctx_ptr_t ctx, expression_op_e op, value_ptr_t op1, value_ptr_t op2, node_ptr_t node)
{
    
    value_ptr_t val(new value_t());

    check_val_type_equality(ctx, op1, op2, node);


    Value* lhs = emit_r_value(ctx, op1);
    Value* rhs = emit_r_value(ctx, op2);
    Value* res = nullptr;


    switch (op)
    {
    case expr_add: res = ctx->builder()->CreateAdd(lhs, rhs); break;
    case expr_sub: res = ctx->builder()->CreateSub(lhs, rhs); break;
    case expr_mult: res = ctx->builder()->CreateMul(lhs, rhs); break;
    case expr_div: res = ctx->builder()->CreateSDiv(lhs, rhs); break;
    case expr_mod: res = ctx->builder()->CreateSRem(lhs, rhs); break;
    case expr_shiftleft: res = ctx->builder()->CreateShl(lhs, rhs); break;
    case expr_shiftright: res = ctx->builder()->CreateAShr(lhs, rhs); break;
    case expr_and: res = ctx->builder()->CreateAnd(lhs, rhs); break;
    case expr_xor: res = ctx->builder()->CreateXor(lhs, rhs); break;
    case expr_or: res = ctx->builder()->CreateOr(lhs, rhs); break;
    default: {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): unknown binary operator %d",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos,
            op);
        break;
    }
    }

  
    val->ir_value  = res;
    val->is_lvalue = false;

    return val;
}

value_ptr_t 
llvmir_compiler_t::emit_cmp_binary(compiler_ctx_ptr_t ctx, binary_expr_node_ptr_t node)
{
	init_dloc(ctx, node);

    value_ptr_t val(new value_t());
    auto bn = PCAST(binary_expr_node_t, node);

    value_ptr_t e1 = emit_expression(ctx, bn->operand1);
    value_ptr_t e2 = emit_expression(ctx, bn->operand2);

    check_val_type_equality(ctx, e1, e2, node);

    Value* lhs = emit_r_value(ctx, e1);
    Value* rhs = emit_r_value(ctx, e2);

    Value* cmp = nullptr;

    switch (node->op_id)
    {
    case expr_less: cmp = ctx->builder()->CreateICmpSLT(lhs, rhs); break;
    case expr_lesseeq: cmp = ctx->builder()->CreateICmpSLE(lhs, rhs); break;
    case expr_more: cmp = ctx->builder()->CreateICmpSGT(lhs, rhs); break;
    case expr_moreeq: cmp = ctx->builder()->CreateICmpSGE(lhs, rhs); break;
    case expr_logicaleq: cmp = ctx->builder()->CreateICmpEQ(lhs, rhs); break;
    case expr_noteq: cmp = ctx->builder()->CreateICmpNE(lhs, rhs); break;
    default: 
    { 
		assert(false && "unknown comparison operator");
    }
    }
   

    // cmp is i1 - extend to operand integer type for downstream compatibility
    Value* ext = ctx->builder()->CreateZExt(cmp, e1->ir_value->getType());
    
    val->ir_value  = ext;
    val->is_lvalue = false;
	return val;
}

value_ptr_t 
llvmir_compiler_t::emit_assign_arithmetic_binary(compiler_ctx_ptr_t ctx, binary_expr_node_ptr_t node)
{
	init_dloc(ctx, node);

	expression_op_e base_op;

    switch (node->op_id)
    {
        // compound-assigns
    case expr_addassign: base_op = expr_add; break;
    case expr_subassign: base_op = expr_sub; break;
    case expr_multassign: base_op = expr_mult; break;
    case expr_divassign: base_op = expr_div; break;
    case expr_modassign: base_op = expr_mod; break;
    case expr_shiftleftassign: base_op = expr_shiftleft; break;
    case expr_shiftrightassign: base_op = expr_shiftright; break;
    case expr_andassign: base_op = expr_and; break;
    case expr_xorassign: base_op = expr_xor; break;
    case expr_orassign: base_op = expr_or; break;
    default:
        throw aloe_exception_t("%s:%zu:%zu: (internal error): unknown binary assign operator %d",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos,
            node->op_id);
        break;
    }

	value_ptr_t val1 = emit_expression(ctx, node->operand1);
	value_ptr_t val2 = emit_expression(ctx, node->operand2);
	value_ptr_t val3 = emit_raw_binary_arithmetic(ctx, base_op, val1, val2, node);


    return emit_raw_assign(ctx, val1, val3, node);
   
}


value_ptr_t
llvmir_compiler_t::emit_expression(compiler_ctx_ptr_t ctx, expr_node_ptr_t node)
{
    init_dloc(ctx, node);

    value_ptr_t val(new value_t());

    switch (node->op_id)
    {
    case expr_literal:
    {
        val = emit_expr_literal(ctx, PCAST(literal_expr_node_t, node));
        break;
    }
    case expr_funcall:
    {
        val = emit_expr_fun_call(ctx, PCAST(funcall_expr_node_t, node));
        break;
    }
    case expr_identifier:
    {
        val = emit_expr_identifier(ctx, PCAST(identifier_expr_node_t, node));
        break;
    }
    case expr_assign:
    {
        val = emit_expr_assign(ctx, PCAST(assign_expr_node_t, node));
        break;
    }

    // binary arithmetic
    case expr_add:
    case expr_sub:
    case expr_mult:
    case expr_div:
    case expr_mod:
    case expr_shiftleft:
    case expr_shiftright:
    case expr_and:
    case expr_xor:
    case expr_or:
    {
        val = emit_arithmetic_binary(ctx, PCAST(binary_expr_node_t, node));
        break;
    }

    // comparisons -> produce integer value (extend i1 to operand type)
    case expr_less:
    case expr_lesseeq:
    case expr_more:
    case expr_moreeq:
    case expr_logicaleq:
    case expr_noteq:
    {
        val = emit_cmp_binary(ctx, PCAST(binary_expr_node_t, node));
        break;
    }

    // compound-assigns
    case expr_addassign:
    case expr_subassign:
    case expr_multassign:
    case expr_divassign:
    case expr_modassign:
    case expr_shiftleftassign:
    case expr_shiftrightassign:
    case expr_andassign:
    case expr_xorassign:
    case expr_orassign:
    {
        val = emit_assign_arithmetic_binary(ctx, PCAST(binary_expr_node_t, node));
        break;
    }

    // comma - evaluate args, return last
    case expr_comma:
    {
		val = emit_comma(ctx, PCAST(comma_expr_node_t, node));
        break;
    }
	case expr_preplusplus:
    case expr_preminmin:
    case expr_plus:
    case expr_min:
    {
        val = emit_prefix(ctx, PCAST(unary_expr_node_t, node));
        break;
	}
    case expr_sfxplusplus:
    case expr_sfxminmin:
    {
         val = emit_postfix(ctx, PCAST(unary_expr_node_t, node));
         break;
	}
    
    default:

        throw aloe_exception_t("%s:%zu:%zu: (internal error): invalid operation %d",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos,
            node->op_id);

        break;

    }

    return val;
}

value_ptr_t 
llvmir_compiler_t::emit_postfix(compiler_ctx_ptr_t ctx, unary_expr_node_ptr_t node)
{
    init_dloc(ctx, node);

    value_ptr_t val(new value_t());

    value_ptr_t operand_val = emit_expression(ctx, node->operand);
    check_lvalue(ctx, operand_val, node);

    value_ptr_t operand_rval(new value_t());
    *operand_rval = *operand_val;

    operand_rval->ir_value = emit_r_value(ctx, operand_val);
    operand_rval->is_lvalue = false;

    val = operand_rval;

    switch (node->op_id)
    {
    case expr_sfxminmin:
    case expr_sfxplusplus:
    {
        value_ptr_t const_val = emit_constant(ctx, 1, make_shared<aloe_type_t>(ALOE_TYPE_INT));

        check_val_type_equality(ctx, operand_rval, const_val, node);
        value_ptr_t math_val = emit_raw_binary_arithmetic(ctx, node->op_id == expr_sfxminmin ? expr_sub : expr_add, 
            operand_rval,
            const_val, 
            node);

        check_val_type_equality(ctx, operand_rval, math_val, node);
        value_ptr_t assign_val = emit_raw_assign(ctx, operand_val, math_val, node);
        break;
    }
    default:
        throw aloe_exception_t("%s:%zu:%zu: (internal error): unknown prefix operator %d",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos,
            node->op_id);
        break;
    }

    return val;
}

value_ptr_t 
llvmir_compiler_t::emit_prefix(compiler_ctx_ptr_t ctx, unary_expr_node_ptr_t  node)
{
	init_dloc(ctx, node);

    value_ptr_t val(new value_t());
    value_ptr_t operand_val = emit_expression(ctx, node->operand);

    switch (node->op_id)
    {
    case expr_plus:
    {
        value_ptr_t operand_rval(new value_t());
        operand_rval->ir_value = emit_r_value(ctx, operand_val);
        operand_rval->is_lvalue = false;

        val = operand_rval;
        break;
    }
    case expr_min:
    {
        Value* neg = ctx->builder()->CreateNeg(val->ir_value);
	
        val->ir_value = neg;
		val->is_lvalue = false;

        break;
    }
    case expr_preplusplus:
    case expr_preminmin:
    {
		check_lvalue(ctx, operand_val, node);
		value_ptr_t const_val = emit_constant(ctx, 1 , make_shared<aloe_type_t>(ALOE_TYPE_INT));

        value_ptr_t operand_rval(new value_t());
        operand_rval->ir_value = emit_r_value(ctx, operand_val);
        operand_rval->is_lvalue = false;

        check_val_type_equality(ctx, operand_rval, const_val, node);
        value_ptr_t math_val = emit_raw_binary_arithmetic(ctx,  node->op_id == expr_preminmin ? expr_sub : expr_add, operand_rval, const_val, node);

        check_assign_val_type_equality(ctx, operand_val, math_val, node);
        value_ptr_t assign_val = emit_raw_assign(ctx, operand_val, math_val, node);

        val = assign_val;
		
        break;
    }
    default:
        throw aloe_exception_t("%s:%zu:%zu: (internal error): unknown prefix operator %d",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos,
            node->op_id);
        break;
    }

	return val;

}

value_ptr_t llvmir_compiler_t::emit_comma(compiler_ctx_ptr_t ctx, comma_expr_node_ptr_t node)
{
    value_ptr_t val(new value_t());
    value_ptr_t last;
    for (auto& a : node->arg_list->args) {
        last = emit_expression(ctx, a);
    }
    // return rvalue of last
    if (last) {
        llvm::DebugLoc dloc = llvm::DILocation::get(
            *ctx->ctx(),
            node->line,
            node->pos,
            ctx->curr_fun()->getSubprogram()
        );
        // if lvalue convert to rvalue
        last->ir_value = emit_r_value(ctx, last);
        last->is_lvalue = false;
        val = last;
    }
	return val;

}

value_ptr_t 
llvmir_compiler_t::emit_raw_assign(compiler_ctx_ptr_t ctx, value_ptr_t lhs, value_ptr_t rhs, node_ptr_t node)
{
    value_ptr_t val(new value_t());

    
    check_assign_val_type_equality(ctx, lhs, rhs, node);

    val->ir_value  = emit_r_value(ctx, rhs);
    val->is_lvalue = false;

    ctx->builder()->CreateStore(val->ir_value, lhs->ir_value);
   
	return val;
}

value_ptr_t 
llvmir_compiler_t::emit_expr_assign(compiler_ctx_ptr_t ctx, assign_expr_node_ptr_t node)
{
    init_dloc(ctx, node);

    value_ptr_t op1 = emit_expression(ctx, node->operand1);
    value_ptr_t op2 = emit_expression(ctx, node->operand2);
    
    return emit_raw_assign(ctx,  op1, op2, node);
}

Value* 
llvmir_compiler_t::emit_r_value(compiler_ctx_ptr_t ctx, value_ptr_t val)
{
    if (!val->is_lvalue)
        return val->ir_value;

    // load from address
    auto inst = ctx->builder()->CreateLoad(val->lval_type, val->ir_value);
   
    return  inst;
}


value_ptr_t
llvmir_compiler_t::emit_expr_fun_call(compiler_ctx_ptr_t ctx, funcall_expr_node_ptr_t node)
{
    init_dloc(ctx, node);

    value_ptr_t val(new value_t());
	value_ptr_t fun_val = emit_expression(ctx, node->fun_expr);

    std::vector <Value*> args = {};

	for (auto arg : node->arg_list->args)
    {
        value_ptr_t arg_val = emit_expression(ctx, arg);

        args.push_back(emit_r_value(ctx, arg_val));
    }

    auto inst = ctx->builder()->CreateCall(
        ir_sc<Function>(fun_val->ir_value)->getFunctionType(), emit_r_value(ctx, fun_val), args);
    
    val->ir_value = inst;

    return nullptr;
}

value_ptr_t
llvmir_compiler_t::emit_expr_literal(compiler_ctx_ptr_t ctx, literal_expr_node_ptr_t node)
{
    init_dloc(ctx, node);

	return emit_literal(ctx, node->literal);
}

value_ptr_t
llvmir_compiler_t::emit_literal(compiler_ctx_ptr_t ctx, literal_node_ptr_t node)
{
	init_dloc(ctx, node);

    value_ptr_t val(new value_t());
    
   
    switch (node->lit_type_id)
    {
    case LIT_INT:
    {
        Type * ir_type = emit_type(ctx, node->type);
	    val->ir_value   = ConstantInt::get(ir_type, std::get<int>(node->value), true);
		val->is_lvalue  = false;

	    break;
    }
    case LIT_STRING:
    {
        Type * ir_type  = emit_type(ctx, node->type);
        string s = std::get<string>(node->value);
        val->ir_value   = ctx->builder()->CreateGlobalString(s,"",0,ctx->module());
        val->is_lvalue  = false;
        
        break;
    }
    case LIT_CHAR:
    {
        Type * ir_type = emit_type(ctx, node->type);
        val->ir_value   = ConstantInt::get(ir_type, std::get<char>(node->value));
        val->is_lvalue  = false;
        break;
    }
    case LIT_POINTER_VOID:
    default:
    {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): unsupported literal type %d", ctx->ast()->source_id.c_str(), node->line, node->pos, node->line);
    }
    }

    return val;
        
}

void llvmir_compiler_t::check_assign_val_type_equality(compiler_ctx_ptr_t ctx, value_ptr_t v1, value_ptr_t v2, node_ptr_t node)
{
    check_lvalue(ctx, v1, node);
    if (v1->lval_type != v2->ir_value->getType())
    {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): attempt to perform assignment on incompatible types",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos);
    }

}

void 
llvmir_compiler_t::check_val_type_equality(compiler_ctx_ptr_t ctx, value_ptr_t v1, value_ptr_t v2, node_ptr_t node)
{
	if (v2->ir_value->getType() != v1->ir_value->getType())
    {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): attempt to perform operation on incompatible types",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos);
    }
}

void
llvmir_compiler_t::check_lvalue(compiler_ctx_ptr_t ctx, value_ptr_t v, node_ptr_t node)
{
    if (!v->is_lvalue)
    {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): need lvalue for the operation",
            ctx->ast()->source_id.c_str(),
            node->line,
            node->pos);
    }
}

value_ptr_t 
llvmir_compiler_t::emit_constant(compiler_ctx_ptr_t ctx, variant<int, float, double, char> var, aloe_type_ptr_t type)
{
    value_ptr_t val(new value_t());

    switch (type->type_id)
    {
    case ALOE_TYPE_INT:
    {
        Type * ir_type = emit_type(ctx, type);
        val->ir_value = ConstantInt::get(ir_type, std::get<int>(var), true);
        val->is_lvalue = false;
        break;
    }
    case ALOE_TYPE_DOUBLE:
    {
        Type * ir_type = emit_type(ctx, type);
        val->ir_value = ConstantFP::get(ir_type, std::get<double>(var));
        val->is_lvalue = false;
		break;
    }
    default:
    {
        throw aloe_exception_t("%s:%zu:%zu: (internal error): unsupported constant type %d",
            ctx->ast()->source_id.c_str(),
            0,
            0,
            type);
    }
	}
	return val;
}

llvm::DIScope*
llvmir_compiler_t::get_scope(compiler_ctx_ptr_t ctx)
{
    DIScope* scope = ctx->curr_fun() == nullptr ?
        ctx->llvm_cu() :
        ir_sc<DIScope>(ctx->curr_fun()->getSubprogram());

    return scope;
}

llvm::DebugLoc llvmir_compiler_t::init_dloc(compiler_ctx_ptr_t ctx, node_ptr_t node)
{
  
    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->ctx(),
        node->line,
        node->pos,
		get_scope(ctx)
    );
    
    ctx->builder()->SetCurrentDebugLocation(dloc);

    return dloc;
}