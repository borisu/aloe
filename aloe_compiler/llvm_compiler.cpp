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

#define RAISE_LOC(fmt, ...) \
    throw aloe_exception_t("%s:%zu:%zu: error (phase II): " fmt, \
        ctx->ast()->source_id.c_str(), \
        node->line, \
        node->pos,  \
        ##__VA_ARGS__)

compiler_t_ptr
aloe::create_llvm_compiler()
{
    return compiler_t_ptr(new llvmir_compiler_t());
}

llvmir_compiler_t::llvmir_compiler_t()
{
	validate = true;
	no_debug = false;
}

void 
llvmir_compiler_t::set_validate(bool validate)
{
    this->validate = validate;
}

void
llvmir_compiler_t::set_no_debug(bool no_debug)
{
	this->no_debug = no_debug;
}

bool 
llvmir_compiler_t::compile(
    ast_t_ptr ast,
	ostream& out)
{
    LLVMContext ctx;
    Module module(ast->source_id, ctx);
   
    module.addModuleFlag(Module::Warning, "CodeView", 1);
    module.addModuleFlag(Module::Warning, "Dwarf Version", 4);
    module.addModuleFlag(Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);

    IRBuilder<> ir(ctx);

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
    
    compiler_ctx_t_ptr compiler_ctx(new llvm_ctx_modifier_t(&ctx, &module, &ir, &dib, di_file, cu));
    compiler_ctx_t_ptr ast_ctx(new ast_ctx_modifier_t(ast, compiler_ctx));

	di_cache = make_shared<di_cache_t>(dib);

	bool res = false;
    try
    {
        if (ast->root->node_type_id != PROG_NODE)
        {
            throw aloe_exception_t("internal error: expected program node");
        }

        walk_prog(ast_ctx, castptr(prog_node_t, ast->root));
		res = true;

        dib.finalize();

        if (no_debug)
        {
            llvm::StripDebugInfo(module);
        }

		if (validate)
		{
			llvm::verifyModule(module, &llvm::errs());
		}

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
llvmir_compiler_t::emit_ir_type(compiler_ctx_t_ptr ctx, type_node_t_ptr node)
{
    init_dloc(ctx, node);
    return emit_ir_type(ctx, node->type());}

Type*
llvmir_compiler_t::emit_ir_type(compiler_ctx_t_ptr ctx, aloe_type_t_ptr type)
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
	case ALOE_TYPE_ARRAY:
	{
		Type* arr_type = emit_ir_type(ctx, type->arr->elem_type());
		out = ArrayType::get(arr_type, type->arr->size);
		break;
	}
    case ALOE_TYPE_FUNCTION:
    {
        Type* ir_type = emit_ir_type(ctx, type->fun->ret_type());

        std::vector<Type*>  irt_args;
        for (auto var : type->fun->params->v)
        {
            Type* argt = emit_ir_type(ctx, var->var_type());
            irt_args.push_back(argt);
        };

        out  = FunctionType::get(ir_type, irt_args, false);
        break;

    }
	case ALOE_TYPE_LAYOUT:
	{
		std::vector<Type*>  irt_members;
		for (auto field : type->layout->fields->v)
		{
			Type* mt = emit_ir_type(ctx, field->var_type());
			irt_members.push_back(mt);
		};
		out = StructType::create(*ctx->ctx(), irt_members,"layout");
		break;
	}
    default:
    {
        assert(false && "(internal error): unknown type id");
    }
    };

    return out;
}


value_t_ptr
llvmir_compiler_t::emit_fun(compiler_ctx_t_ptr ctx, fun_node_t_ptr node)
{
    if (node->ignore)
    {
        return value_t_ptr();
    }

    value_t_ptr out(new value_t());

    Type *ir_fun_type   = emit_ir_type(ctx, node->fun_type());
	out->di_type        = di_cache->get_dit_type(node->fun_type());
  
    Function* ir_fun =
        Function::Create(ir_sc<FunctionType>(ir_fun_type),
            Function::ExternalLinkage, node->idt->name, ctx->module());
   
    DISubprogram* sp = ctx->di_builder()->createFunction(
        ctx->di_file(),         // Function scope.  
        node->idt->name,         // Function name.
        node->idt->name,         // Mangled function name.
        ctx->di_file(),         // File where this variable is defined.
        node->line,             // Line number.
		ir_sc<DISubroutineType>(di_cache->get_dit_type(node->fun_type())), // type
        node->line,             // scope line
        DINode::FlagZero,
		node->is_defined ? DISubprogram::SPFlagDefinition : DISubprogram::SPFlagZero
     );

    ir_fun->setSubprogram(sp);

    out->ir_value   = ir_fun;
    obj_cache[node]  = out;
	
    if (node->is_defined)
    {
        try
        {
            // new scope for function body
            compiler_ctx_t_ptr new_ctx(new fun_ctx_modifier_t(ir_fun, ctx));

            emit_fun_definition(new_ctx, ir_fun, node);

        }
        catch (...) {
            ctx->builder()->GetInsertBlock()->deleteTrailingDbgRecords();
            throw;
        }
    }
  
    bool is_broken = llvm::verifyFunction(*ir_fun);
        
    if (is_broken && validate) {
		RAISE_LOC("generated IR for function '%s' is broken", node->idt->name.c_str());
    }

    return out;
		
}

void 
llvmir_compiler_t::emit_fun_definition(compiler_ctx_t_ptr ctx, Function* fun, fun_node_t_ptr node)
{
    init_dloc(ctx, node);

    BasicBlock* ir_block = BasicBlock::Create(*ctx->ctx(), node->idt->name, fun);

    ctx->builder()->SetInsertPoint(ir_block);

    for (int i = 0; i < fun->arg_size(); i++)
    {
        Argument* ir_arg = fun->getArg(i);

        auto var = node->fun_type()->fun->params->v[i];
       
        // temporary storage for better debuggability and mutability of parameters
        auto* arg_slot = ctx->builder()->CreateAlloca(ir_arg->getType());
        ctx->builder()->CreateStore(ir_arg, arg_slot);

        if (!var->name.empty())
        {
            //auto var_name = arg_node->id->name;
            //ir_arg->setName(var_name);

            auto arg_dvar = ctx->di_builder()->createParameterVariable(
                get_scope(ctx),
                var->name,
                i + 1,
                ctx->di_file(),
                node->line,
                di_cache->get_dit_type(var->var_type()),
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

        value_t_ptr arg_val(new value_t());
        arg_val->ir_value = arg_slot;
        arg_val->is_lvalue = true;
        arg_val->lval_ir_type = ir_arg->getType();
        arg_val->type = var->var_type();

        obj_cache[var->ref] = arg_val;

    }

    // emit fucntion statements
    for (auto& statement : node->statements)
    {
        switch (statement->node_type_id)
        {
        case EXPRESSION_NODE:
        {
            emit_expr_value(ctx, castptr(expr_node_t, statement));
            break;
        }
        case VAR_NODE:
        {
            emit_var(ctx, castptr(var_node_t, statement));
            break;
        }
        case RETURN_NODE:
        {
            emit_return(ctx, castptr(return_node_t, statement));
            break;
        }
        default:
            break;
        }
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
			RAISE_LOC("function '%s' must return expression of type '%s'",
				node->idt->name.c_str(),
				node->fun_type()->fun->ret_type()->to_str().c_str());
        }
    }

}

void 
llvmir_compiler_t::emit_return(compiler_ctx_t_ptr ctx, return_node_t_ptr node)
{
    init_dloc(ctx, node);

	llvm::ReturnInst* ret_inst = nullptr;

    if (!node->return_expr)
    {
         ret_inst = ctx->builder()->CreateRetVoid();
    }
    else
    {
        value_t_ptr ret_val = emit_expr_value(ctx, node->return_expr);

        ret_inst = ctx->builder()->CreateRet(emit_rvalue(ctx, ret_val));
    }

}

void 
llvmir_compiler_t::walk_prog(compiler_ctx_t_ptr ctx, prog_node_t_ptr node)
{
    init_dloc(ctx, node);

    for (auto& decl : node->statements)
    {
        switch (decl->node_type_id)
        {
        case FUNCTION_NODE:
            emit_fun(ctx, castptr(fun_node_t,decl));
			break;
        case VAR_NODE:
            emit_var(ctx, castptr(var_node_t, decl));
            break;
        default:
			RAISE("unknown top-level declaration node type %d", decl->node_type_id);
        }
    }

}

value_t_ptr
llvmir_compiler_t::emit_expr_identifier(compiler_ctx_t_ptr ctx, identifier_expr_node_t_ptr node)
{
    init_dloc(ctx, node);

	value_t_ptr out(new value_t());

	switch (node->ref()->node_type_id)
    {
        case FUNCTION_NODE:
        case VAR_NODE:
        {
            out =  obj_cache[node->ref()];
            break;
        }
        default:
        {
			RAISE_LOC("identifier '%s' is of unknown type", node->idt->name.c_str());
        }
    }

    return out;
}


value_t_ptr 
llvmir_compiler_t::emit_default(compiler_ctx_t_ptr ctx, aloe_type_t_ptr atype)
{
	value_t_ptr out(new value_t());

    out->is_lvalue = false;
    out->ir_value  = Constant::getNullValue(emit_ir_type(ctx, atype));
    out->di_type   = di_cache->get_dit_type(atype);
   
    return out;

}


void 
llvmir_compiler_t::emit_var(compiler_ctx_t_ptr ctx, var_node_t_ptr node)
{
    init_dloc(ctx, node);
	value_t_ptr out(new value_t());

    Type *ir_var_type  = emit_ir_type(ctx, node->var_type() );
    out->lval_ir_type = ir_var_type;
    out->is_lvalue = true;

	auto init_val = node->initializer ? emit_expr_value(ctx, node->initializer) : emit_default(ctx, node->var_type());

    if (!ctx->curr_fun())
    {
        auto g_var = new GlobalVariable(
            *ctx->module(),
            ir_var_type,
            false,
            GlobalValue::ExternalLinkage,
            ir_sc <Constant>(init_val->ir_value),
			node->idt->name
            );

        auto* di_var = ctx->di_builder()->createGlobalVariableExpression(
            ctx->di_file(),
            node->idt->name,                // name
            node->idt->name,                // linkage name
            ctx->di_file(),               
            node->line,
            di_cache->get_dit_type(node->var_type()),     
            true                    
        );

        g_var->addDebugInfo(di_var);
        out->ir_value = g_var;
        
    }
    else
    {
        auto alloca_inst = ctx->builder()->CreateAlloca(ir_var_type, nullptr, node->idt->name);
        out->ir_value  = alloca_inst;

        auto di_var = ctx->di_builder()->createAutoVariable(
                get_scope(ctx),        
                node->idt->name,        
                ctx->di_file(),
                node->line,           
				di_cache->get_dit_type(node->var_type())
            );

        ctx->di_builder()->insertDeclare(
            alloca_inst,
            di_var,
            ctx->di_builder()->createExpression(),
            llvm::DILocation::get(*ctx->ctx(), node->line, node->pos, get_scope(ctx)),
            ctx->builder()->GetInsertBlock());

	
        ctx->builder()->CreateStore(emit_rvalue(ctx, init_val), out->ir_value);
        
    }

	out->type = node->var_type();
	obj_cache[node] = out;
   
}

value_t_ptr 
llvmir_compiler_t::emit_arithmetic_binary(compiler_ctx_t_ptr ctx, binary_expr_node_t_ptr node)
{
    llvm::DebugLoc dloc = init_dloc(ctx, node);

    value_t_ptr e1 = emit_expr_value(ctx, node->operand1);
    value_t_ptr e2 = emit_expr_value(ctx, node->operand2);
   
	return emit_raw_binary_arithmetic(ctx, node->op_id, e1, e2, node);
}

value_t_ptr 
llvmir_compiler_t::emit_raw_binary_arithmetic(compiler_ctx_t_ptr ctx, expression_op_e op, value_t_ptr op1, value_t_ptr op2, node_t_ptr node)
{
    
    value_t_ptr val(new value_t());

    

    Value* lhs = emit_rvalue(ctx, op1);
    Value* rhs = emit_rvalue(ctx, op2);

    check_ir_type_equal(ctx, lhs, rhs, node);

    
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
		RAISE_LOC("unknown binary operator %d", op);
        break;
    }
    }

  
    val->ir_value  = res;
    val->is_lvalue = false;

    return val;
}

value_t_ptr 
llvmir_compiler_t::emit_cmp_binary(compiler_ctx_t_ptr ctx, binary_expr_node_t_ptr node)
{
	init_dloc(ctx, node);

    value_t_ptr val(new value_t());
    auto bn = castptr(binary_expr_node_t, node);

    value_t_ptr e1 = emit_expr_value(ctx, bn->operand1);
    value_t_ptr e2 = emit_expr_value(ctx, bn->operand2);


    Value* lhs = emit_rvalue(ctx, e1);
    Value* rhs = emit_rvalue(ctx, e2);

    check_ir_type_equal(ctx, lhs, rhs, node);


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
		RAISE_LOC("unknown comparison operator %d", node->op_id);
    }
    }
   

    // cmp is i1 - extend to operand integer type for downstream compatibility
    Value* ext = ctx->builder()->CreateZExt(cmp, e1->ir_value->getType());
    
    val->ir_value  = ext;
    val->is_lvalue = false;
	return val;
}

value_t_ptr 
llvmir_compiler_t::emit_assign_arithmetic_binary(compiler_ctx_t_ptr ctx, binary_expr_node_t_ptr node)
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
		RAISE_LOC("unknown binary assign operator %d", node->op_id);
        break;
    }

	value_t_ptr val1 = emit_expr_value(ctx, node->operand1);
	value_t_ptr val2 = emit_expr_value(ctx, node->operand2);
	value_t_ptr val3 = emit_raw_binary_arithmetic(ctx, base_op, val1, val2, node);


    return emit_raw_assign(ctx, val1, val3, node);
   
}


value_t_ptr
llvmir_compiler_t::emit_expr_value(compiler_ctx_t_ptr ctx, expr_node_t_ptr node)
{
    init_dloc(ctx, node);

    value_t_ptr val(new value_t());

    switch (node->op_id)
    {
    case expr_literal:
    {
        val = emit_expr_literal(ctx, castptr(literal_expr_node_t, node));
        break;
    }
    case expr_funcall:
    {
        val = emit_expr_fun_call(ctx, castptr(funcall_expr_node_t, node));
        break;
    }
    case expr_identifier:
    {
        val = emit_expr_identifier(ctx, castptr(identifier_expr_node_t, node));
        break;
    }
    case expr_assign:
    {
        val = emit_expr_assign(ctx, castptr(assign_expr_node_t, node));
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
        val = emit_arithmetic_binary(ctx, castptr(binary_expr_node_t, node));
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
        val = emit_cmp_binary(ctx, castptr(binary_expr_node_t, node));
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
        val = emit_assign_arithmetic_binary(ctx, castptr(binary_expr_node_t, node));
        break;
    }

    // comma - evaluate args, return last
    case expr_comma:
    {
		val = emit_expr_comma(ctx, castptr(comma_expr_node_t, node));
        break;
    }
	case expr_preplusplus:
    case expr_preminmin:
    case expr_plus:
    case expr_min:
    {
        val = emit_expr_prefix(ctx, castptr(unary_expr_node_t, node));
        break;
	}
    case expr_sfxplusplus:
    case expr_sfxminmin:
    {
         val = emit_expr_postfix(ctx, castptr(unary_expr_node_t, node));
         break;
	}
    case expr_addressof:
    {
        val = emit_expr_addressof(ctx, castptr(addressof_expr_node_t, node));
        break;
    }
    case expr_deref:
    {
        val = emit_expr_deref(ctx, castptr(deref_expr_node_t, node));
        break;
    }
    case expr_index:
    {
        val = emit_expr_index(ctx, castptr(index_expr_node_t, node));
        break;
    }
    default:
    {
        RAISE_LOC("invalid operation %d", node->op_id);
        break;
    }

    }

    return val;
}
value_t_ptr
llvmir_compiler_t::emit_expr_deref(compiler_ctx_t_ptr ctx, deref_expr_node_t_ptr node)
{
    init_dloc(ctx, node);
    value_t_ptr val(new value_t());

    value_t_ptr operand_val = emit_expr_value(ctx, node->operand);
    

    val->ir_value  = emit_rvalue(ctx, operand_val);
    val->is_lvalue = true;
	val->lval_ir_type = emit_ir_type(ctx, node->expr_type());
	val->di_type   = di_cache->get_dit_type(node->expr_type());


    return val;
}

value_t_ptr 
llvmir_compiler_t::emit_expr_addressof(compiler_ctx_t_ptr ctx, addressof_expr_node_t_ptr node)
{
	init_dloc(ctx, node);
    value_t_ptr val(new value_t());

    value_t_ptr operand_val = emit_expr_value(ctx, node->operand);
    check_lvalue(ctx, operand_val, node);

	val = operand_val;
	val->is_lvalue = false; 
    

    return val;

}

value_t_ptr 
llvmir_compiler_t::emit_expr_postfix(compiler_ctx_t_ptr ctx, unary_expr_node_t_ptr node)
{
    init_dloc(ctx, node);

    value_t_ptr val(new value_t());

    value_t_ptr operand_val = emit_expr_value(ctx, node->operand);
    check_lvalue(ctx, operand_val, node);

    value_t_ptr operand_rval(new value_t());
    *operand_rval = *operand_val;

    operand_rval->ir_value = emit_rvalue(ctx, operand_val);
    operand_rval->is_lvalue = false;

    val = operand_rval;

    switch (node->op_id)
    {
    case expr_sfxminmin:
    case expr_sfxplusplus:
    {
        value_t_ptr const_val = emit_constant(ctx, 1, make_shared<aloe_type_t>(ALOE_TYPE_INT), node);

        check_ir_type_equal(ctx, operand_rval->ir_value, const_val->ir_value, node);
        value_t_ptr math_val = emit_raw_binary_arithmetic(ctx, node->op_id == expr_sfxminmin ? expr_sub : expr_add, 
            operand_rval,
            const_val, 
            node);

        check_ir_type_equal(ctx, operand_rval->ir_value, math_val->ir_value, node);
        value_t_ptr assign_val = emit_raw_assign(ctx, operand_val, math_val, node);
        break;
    }
    default:
        RAISE_LOC("unknown postfix operator %d", node->op_id);
        break;
    }

    return val;
}

value_t_ptr 
llvmir_compiler_t::emit_expr_prefix(compiler_ctx_t_ptr ctx, unary_expr_node_t_ptr  node)
{
	init_dloc(ctx, node);

    value_t_ptr val(new value_t());
    value_t_ptr operand_val = emit_expr_value(ctx, node->operand);

    switch (node->op_id)
    {
    case expr_plus:
    {
        value_t_ptr operand_rval(new value_t());
        operand_rval->ir_value = emit_rvalue(ctx, operand_val);
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
		value_t_ptr const_val = emit_constant(ctx, 1 , make_shared<aloe_type_t>(ALOE_TYPE_INT), node);

        value_t_ptr operand_rval(new value_t());
        operand_rval->ir_value = emit_rvalue(ctx, operand_val);
        operand_rval->is_lvalue = false;

        check_ir_type_equal(ctx, operand_rval->ir_value, const_val->ir_value, node);
        value_t_ptr math_val = emit_raw_binary_arithmetic(ctx,  node->op_id == expr_preminmin ? expr_sub : expr_add, operand_rval, const_val, node);

        check_assign_val_type_equality(ctx, operand_val, math_val, node);
        value_t_ptr assign_val = emit_raw_assign(ctx, operand_val, math_val, node);

        val = assign_val;
		
        break;
    }
    default:
        RAISE_LOC("unknown prefix operator %d", node->op_id);
        break;
    }

	return val;

}

value_t_ptr llvmir_compiler_t::emit_expr_comma(compiler_ctx_t_ptr ctx, comma_expr_node_t_ptr node)
{
    value_t_ptr val(new value_t());
    value_t_ptr last;
    for (auto& a : node->arg_list->args) {
        last = emit_expr_value(ctx, a);
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
        last->ir_value = emit_rvalue(ctx, last);
        last->is_lvalue = false;
        val = last;
    }
	return val;

}

value_t_ptr 
llvmir_compiler_t::emit_raw_assign(compiler_ctx_t_ptr ctx, value_t_ptr lhs, value_t_ptr rhs, node_t_ptr node)
{
    value_t_ptr val(new value_t());

    
    check_assign_val_type_equality(ctx, lhs, rhs, node);

    val->ir_value  = emit_rvalue(ctx, rhs);
    val->is_lvalue = false;

    ctx->builder()->CreateStore(val->ir_value, lhs->ir_value);
   
	return val;
}

value_t_ptr 
llvmir_compiler_t::emit_expr_assign(compiler_ctx_t_ptr ctx, assign_expr_node_t_ptr node)
{
    init_dloc(ctx, node);

    value_t_ptr op1 = emit_expr_value(ctx, node->operand1);
    value_t_ptr op2 = emit_expr_value(ctx, node->operand2);
    
    return emit_raw_assign(ctx,  op1, op2, node);
}

value_t_ptr
llvmir_compiler_t::emit_expr_index(compiler_ctx_t_ptr ctx, index_expr_node_t_ptr node)
{
    init_dloc(ctx, node);
    value_t_ptr val(new value_t());

    value_t_ptr array_val = emit_expr_value(ctx, node->operand1);
    value_t_ptr index_val = emit_expr_value(ctx, node->operand2);

    
    // get element pointer
    Value* gep_val = ctx->builder()->CreateGEP(
		emit_ir_type(ctx, array_val->type->arr->elem_type()),
        array_val->ir_value,
        emit_rvalue(ctx, index_val)
    );

    val->ir_value = gep_val;
    val->is_lvalue = true;
    val->lval_ir_type = emit_ir_type(ctx, node->expr_type());
    val->di_type = di_cache->get_dit_type(node->expr_type());


    return val;
}

Value* 
llvmir_compiler_t::emit_rvalue(compiler_ctx_t_ptr ctx, value_t_ptr val)
{
    if (!val->is_lvalue)
        return val->ir_value;

    // load from address
    auto inst = ctx->builder()->CreateLoad(val->lval_ir_type, val->ir_value);
   
    return  inst;
}


value_t_ptr
llvmir_compiler_t::emit_expr_fun_call(compiler_ctx_t_ptr ctx, funcall_expr_node_t_ptr node)
{
    init_dloc(ctx, node);

    value_t_ptr val(new value_t());
	value_t_ptr fun_val = emit_expr_value(ctx, node->fun_expr);

    std::vector <Value*> args = {};

	for (auto arg : node->arg_list->args)
    {
        value_t_ptr arg_val = emit_expr_value(ctx, arg);

        args.push_back(emit_rvalue(ctx, arg_val));
    }

    auto inst = ctx->builder()->CreateCall(
        ir_sc<Function>(fun_val->ir_value)->getFunctionType(), emit_rvalue(ctx, fun_val), args);
    
    val->ir_value = inst;

    return nullptr;
}

value_t_ptr
llvmir_compiler_t::emit_expr_literal(compiler_ctx_t_ptr ctx, literal_expr_node_t_ptr node)
{
    init_dloc(ctx, node);

	return emit_literal(ctx, node->literal);
}

value_t_ptr
llvmir_compiler_t::emit_literal(compiler_ctx_t_ptr ctx, literal_node_t_ptr node)
{
	init_dloc(ctx, node);

    value_t_ptr val(new value_t());
    
   
    switch (node->lit_type_id)
    {
    case LIT_INT:
    {
        Type * ir_type = emit_ir_type(ctx, node->literal_type());
	    val->ir_value   = ConstantInt::get(ir_type, std::get<int>(node->value), true);
		val->is_lvalue  = false;

	    break;
    }
    case LIT_STRING:
    {
        Type * ir_type  = emit_ir_type(ctx, node->literal_type());
        string s = std::get<string>(node->value);
        val->ir_value   = ctx->builder()->CreateGlobalString(s,"",0,ctx->module());
        val->is_lvalue  = false;
        
        break;
    }
    case LIT_CHAR:
    {
        Type * ir_type = emit_ir_type(ctx, node->literal_type());
        val->ir_value   = ConstantInt::get(ir_type, std::get<char>(node->value));
        val->is_lvalue  = false;
        break;
    }
    case LIT_POINTER_VOID:
    default:
    {
		RAISE_LOC("unsupported literal type %d", node->lit_type_id);
    }
    }

    return val;
        
}

void llvmir_compiler_t::check_assign_val_type_equality(compiler_ctx_t_ptr ctx, value_t_ptr v1, value_t_ptr v2, node_t_ptr node)
{
    check_lvalue(ctx, v1, node);
    if (v1->lval_ir_type != v2->ir_value->getType())
    {
		RAISE_LOC("attempt to perform assignment on incompatible types : %s vs %s",
			type_to_str(v1->lval_ir_type).c_str(),
			type_to_str(v2->ir_value->getType()).c_str());

    }

}

void 
llvmir_compiler_t::check_ir_type_equal(compiler_ctx_t_ptr ctx, Value* v1, Value *v2, node_t_ptr node)
{
	if (v2->getType() != v1->getType())
    {
		RAISE_LOC("attempt to perform operation on incompatible LLVM types : %s vs %s",
			type_to_str(v1->getType()).c_str(),
			type_to_str(v2->getType()).c_str());
    }
}

void
llvmir_compiler_t::check_lvalue(compiler_ctx_t_ptr ctx, value_t_ptr v, node_t_ptr node)
{
    if (!v->is_lvalue)
    {
		RAISE_LOC("attempt to perform operation on non-lvalue expression");
    }
}

value_t_ptr 
llvmir_compiler_t::emit_constant(compiler_ctx_t_ptr ctx, variant<int, float, double, char> var, aloe_type_t_ptr atype, node_t_ptr node)
{
    value_t_ptr val(new value_t());

    switch (atype->type_id)
    {
    case ALOE_TYPE_INT:
    {
        Type * ir_type = emit_ir_type(ctx, atype);
        val->ir_value = ConstantInt::get(ir_type, std::get<int>(var), true);
        val->is_lvalue = false;
        break;
    }
    case ALOE_TYPE_DOUBLE:
    {
        Type * ir_type = emit_ir_type(ctx, atype);
        val->ir_value = ConstantFP::get(ir_type, std::get<double>(var));
        val->is_lvalue = false;
		break;
    }
    default:
    {
		RAISE_LOC("unsupported constant type %d", atype->type_id);
    }
	}
	return val;
}

llvm::DIScope*
llvmir_compiler_t::get_scope(compiler_ctx_t_ptr ctx)
{
    DIScope* scope = ctx->curr_fun() == nullptr ?
        ctx->llvm_cu() :
        ir_sc<DIScope>(ctx->curr_fun()->getSubprogram());

    return scope;
}

llvm::DebugLoc llvmir_compiler_t::init_dloc(compiler_ctx_t_ptr ctx, node_t_ptr node)
{
  
    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->ctx(),
        node->line,
        node->pos,
		get_scope(ctx)
    );
    
    ctx->builder()->SetCurrentDebugLocation(dloc);
	curr_node = node;

    return dloc;
}
        
