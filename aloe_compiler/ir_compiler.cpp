#include "pch.h"
#include "aloe\compiler.h"
#include "aloe\defs.h"
#include "aloe\scope_guard.h"
#include "compiler_exception.h"
#include "ir_compiler.h"
#include "utils.h"

using namespace aloe;
using namespace llvm;
using namespace llvm::dwarf;

#define ALOE_INT_SIZE 64
#define ALOE_PTR_SIZE 64
#define ALOE_CHAR_SIZE 8
#define ALOE_DOUBLE_SIZE 64



compiler_ptr_t
aloe::create_compiler()
{
    return compiler_ptr_t(new llvmir_compiler_t());
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
    
    compiler_ctx_t compiler_ctx;

    compiler_ctx.llvm_ctx       = &ctx;
	compiler_ctx.llvm_ir        = &ir;
	compiler_ctx.llvm_module    = &module;
	compiler_ctx.llvm_di        = &dib;
    compiler_ctx.llvm_di_file   = di_file;

    compiler_ctx.ast = ast;

	bool res = false;
    try
    {
        walk_prog(&compiler_ctx, ast->prog);
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



value_type_ptr_t
llvmir_compiler_t::emit_type(compiler_ctx_t* ctx, type_node_ptr_t node)
{
    value_type_ptr_t out(new value_type_t());

    switch (node->type_type_id)
    {
    case TT_INT:
    {
        out->ir_type = Type::getInt64Ty(*ctx->llvm_ctx);

        auto di_opt = type_cache.get_dit_type(0, out->ir_type);

        out->di_type = di_opt.has_value() ? di_opt.value() :
            type_cache.get_dit_type(0, out->ir_type, ctx->llvm_di->createBasicType("int", ALOE_INT_SIZE, dwarf::DW_ATE_signed));

        break;
    }
    case TT_CHAR:
    {
        out->ir_type = Type::getInt8Ty(*ctx->llvm_ctx);

        auto di_opt = type_cache.get_dit_type(0, out->ir_type);

        out->di_type = di_opt.has_value() ? di_opt.value() :
            type_cache.get_dit_type(0, out->ir_type, ctx->llvm_di->createBasicType("char", ALOE_CHAR_SIZE, dwarf::DW_ATE_signed_char));

        break;
    }
    case TT_VOID:
    {
        out->ir_type = Type::getVoidTy(*ctx->llvm_ctx);

        auto di_opt = type_cache.get_dit_type(0, out->ir_type);

        out->di_type = di_opt.has_value() ? di_opt.value() :
            type_cache.get_dit_type(0, out->ir_type, nullptr);

        break;
    }
    case TT_FLOAT:
    {
        out->ir_type = Type::getDoubleTy(*ctx->llvm_ctx);

        auto di_opt = type_cache.get_dit_type(0, out->ir_type);

        out->di_type = di_opt.has_value() ? di_opt.value() :
            type_cache.get_dit_type(0, out->ir_type, ctx->llvm_di->createBasicType("double", 64, dwarf::DW_ATE_float));

        break;
    }
    case TT_FUNCTION:
    {
        out = emit_fun_type(ctx, PCAST(fun_type_node_t, node));
        break;

    }
    default:
    {

        throw  compiler_exeption_t("%s:%zu:%zu: (internal error): unknown type %d",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->type_type_id); ;
    }
    };

    if (node->ref_count > 0)
    {
        out->ref_count = node->ref_count;
        out->pointee_ir_type = out->ir_type;
        out->pointee_di_type = out->di_type;
		out->ir_type = PointerType::getUnqual(*ctx->llvm_ctx); // collapse pointer types, we will preserve the original type in debug info for each level of indirection

        for (int i = 1; i <= out->ref_count; i++)
        {
			auto di_opt = type_cache.get_dit_type(i, out->ir_type);
            out->di_type = di_opt.has_value() ? di_opt.value() :
                type_cache.get_dit_type(i, out->ir_type, ctx->llvm_di->createPointerType(out->di_type, 64));
        }
    }

    return out;

}

value_type_ptr_t
llvmir_compiler_t::emit_fun_type(compiler_ctx_t* ctx, fun_type_node_ptr_t node)
{
    value_type_ptr_t out(new value_type_t());

    value_type_ptr_t ret_type = emit_type(ctx, node->ret_type);

    SmallVector<Metadata*, 8>   dit_args;
    dit_args.push_back(ret_type->di_type);

    std::vector<Type*>  irt_args;
    for (auto p : node->param_list->vars_m)
    {
        value_type_ptr_t argt = emit_type(ctx, p.second->type);
        irt_args.push_back(argt->ir_type);
        dit_args.push_back(argt->di_type);
    };

    out->ir_type = FunctionType::get(ret_type->ir_type, irt_args, false);

	auto di_opt = type_cache.get_dit_type(0, out->ir_type);

    out->di_type = di_opt.has_value() ? di_opt.value() : type_cache.get_dit_type(
        0,
        out->ir_type,
        ctx->llvm_di->createSubroutineType(ctx->llvm_di->getOrCreateTypeArray(dit_args)));

    return out;
}


void
llvmir_compiler_t::emit_fun(compiler_ctx_t* ctx, fun_node_ptr_t node)
{
    if (node->ignore)
        return;

    value_ptr_t out(new value_t());
	
    out->ssa_type  = emit_fun_type(ctx, node->fun_type);
    out->is_lvalue  = false;

    Function* ir_fun =
        Function::Create(ir_sc<FunctionType>(out->ssa_type->ir_type),
            Function::ExternalLinkage, node->id->name, ctx->llvm_module);

	for (int i = 0; i < ir_fun->arg_size(); i++)
    {
        auto ir_arg = ir_fun->getArg(i);

        auto var_node = node->fun_type->param_list->vars_v[i].second;
        auto var_name = var_node->id->name;

        ir_arg->setName(var_name);

		value_ptr_t var(new value_t());
		var->ir_value = ir_arg;
	
		id_ssa_cache[var_node] = var;
		
    }
   
    DISubprogram* sp = ctx->llvm_di->createFunction(
        ctx->llvm_di_file,        // Function scope.  
        node->id->name,            // Function name.
        node->id->name,            // Mangled function name.
        ctx->llvm_di_file,        // File where this variable is defined.
        node->line,                // Line number.
        ir_sc<DISubroutineType> (out->ssa_type->di_type), // fnType
        node->line,                 // scope line
        DINode::FlagZero,
		node->is_defined ? DISubprogram::SPFlagDefinition : DISubprogram::SPFlagZero
     );

    ir_fun->setSubprogram(sp);

	out->ir_value = ir_fun;

    if (node->is_defined)
    {
        BasicBlock* ir_block = BasicBlock::Create(*ctx->llvm_ctx, node->id->name, ir_fun);
        
        ctx->llvm_ir->SetInsertPoint(ir_block);

        ctx->fun_desc_stack.push(out);

        auto guard = make_scope_exit([&]() { ctx->fun_desc_stack.pop(); }); // emit may throw, make sure to pop the function from stack

        for (auto& statement : node->exec_block->exec_statements)
        {
            emit_exec_statement(ctx, statement);
        }

        auto terminator = ctx->llvm_ir->GetInsertBlock()->getTerminator();
        if (!terminator)
        {
            if (ir_fun->getReturnType()->isVoidTy()) {
                ctx->llvm_ir->CreateRetVoid();
            }
            else
            {
                throw compiler_exeption_t("%s:%zu:%zu: error: control reaches end of non-void function '%s'",
                    ctx->ast->source_id.c_str(),
                    node->line,
                    node->pos,
                    node->id->name.c_str());
            }
        }
    }
  
    bool is_broken = llvm::verifyFunction(*ir_fun);

    if (is_broken) {
        throw compiler_exeption_t("%s:%zu:%zu: (internal error): generated IR for function '%s' is broken",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->id->name.c_str());
    }

    id_ssa_cache[node] = out;
		
}

void 
llvmir_compiler_t::emit_return(compiler_ctx_t* ctx, return_node_ptr_t node)
{
	llvm::ReturnInst* ret_inst = nullptr;

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
        ((Function*)ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
    );

    if (!node->return_expr)
    {
         ret_inst = ctx->llvm_ir->CreateRetVoid();
    }
    else
    {
        value_ptr_t ret_val = emit_expression(ctx, node->return_expr);

        ret_inst = ctx->llvm_ir->CreateRet(emit_r_value(ctx, ret_val, &dloc));
    }

    ret_inst->setDebugLoc(dloc);

}

void 
llvmir_compiler_t::walk_prog(compiler_ctx_t* ctx, prog_node_ptr_t prog)
{
    for (auto& decl : prog->decl_statements)
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
llvmir_compiler_t::emit_expr_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node)
{
	value_ptr_t out(new value_t());

	switch (node->ast_def->target->node_type_id)
    {
        case FUNCTION_NODE:
        {
            out = PCAST(value_t, id_ssa_cache[node->ast_def->target]);

            break;
        }
        case VAR_NODE:
        {
            out = PCAST(value_t, id_ssa_cache[node->ast_def->target]);

            break;
        }
        default:
        {
            throw compiler_exeption_t("%s:%zu:%zu: (internal error): identifier '%s' points to unsupported type",
                ctx->ast->source_id.c_str(),
                node->line,
                node->pos,
				node->id->name.c_str());
        }
    }

    return out;
}

void 
llvmir_compiler_t::emit_exec_statement(compiler_ctx_t* ctx, node_ptr_t node)
{
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
llvmir_compiler_t::emit_default(compiler_ctx_t* ctx, value_type_ptr_t val_type)
{
	value_ptr_t out(new value_t());

    out->is_lvalue = false;
    out->ir_value = Constant::getNullValue(val_type->ir_type);
    out->ssa_type = val_type;
   
    return out;

}


void 
llvmir_compiler_t::emit_var(compiler_ctx_t* ctx, var_node_ptr_t node)
{
    
	value_ptr_t out(new value_t());

    out->ssa_type = emit_type(ctx, node->type);

	auto init_val = node->initializer ? emit_expression(ctx, node->initializer) : emit_default(ctx, out->ssa_type);

    if (ctx->fun_desc_stack.empty())
    {
        out->ir_value = new GlobalVariable(
            *ctx->llvm_module,
            out->ssa_type->ir_type,
            false,
            GlobalValue::ExternalLinkage,
            ir_sc <Constant>(init_val->ir_value),
			node->id->name
            );

        out->is_lvalue = true;
		
    }
    else
    {
        llvm::DebugLoc dloc = llvm::DILocation::get(
            *ctx->llvm_ctx,
            node->line,
            node->pos,
            ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
        );


        auto alloca_inst = ctx->llvm_ir->CreateAlloca(out->ssa_type->ir_type, nullptr, node->id->name);
        out->ir_value  = alloca_inst;
		out->is_lvalue = true;

        auto store_inst = ctx->llvm_ir->CreateStore(emit_r_value(ctx, init_val, &dloc), out->ir_value);
        
        store_inst->setDebugLoc(dloc);
    }

	id_ssa_cache[node] = out;
   
}

value_ptr_t 
llvmir_compiler_t::emit_arithmetic_binary(compiler_ctx_t* ctx, binary_expr_node_ptr_t node)
{

    value_ptr_t e1 = emit_expression(ctx, node->operand1);
    value_ptr_t e2 = emit_expression(ctx, node->operand2);
   
	return emit_raw_binary_arithmetic(ctx, node->op_id, e1, e2, node);
}

value_ptr_t 
llvmir_compiler_t::emit_raw_binary_arithmetic(compiler_ctx_t* ctx, expression_op_e op, value_ptr_t op1, value_ptr_t op2, node_ptr_t node)
{
    value_ptr_t val(new value_t());

    check_ssa_type_equality(ctx, op1, op2, node);

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
        ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
    );

    Value* lhs = emit_r_value(ctx, op1, &dloc);
    Value* rhs = emit_r_value(ctx, op2, &dloc);
    Value* res = nullptr;


    switch (op)
    {
    case expr_add: res = ctx->llvm_ir->CreateAdd(lhs, rhs); break;
    case expr_sub: res = ctx->llvm_ir->CreateSub(lhs, rhs); break;
    case expr_mult: res = ctx->llvm_ir->CreateMul(lhs, rhs); break;
    case expr_div: res = ctx->llvm_ir->CreateSDiv(lhs, rhs); break;
    case expr_mod: res = ctx->llvm_ir->CreateSRem(lhs, rhs); break;
    case expr_shiftleft: res = ctx->llvm_ir->CreateShl(lhs, rhs); break;
    case expr_shiftright: res = ctx->llvm_ir->CreateAShr(lhs, rhs); break;
    case expr_and: res = ctx->llvm_ir->CreateAnd(lhs, rhs); break;
    case expr_xor: res = ctx->llvm_ir->CreateXor(lhs, rhs); break;
    case expr_or: res = ctx->llvm_ir->CreateOr(lhs, rhs); break;
    default: {
        throw compiler_exeption_t("%s:%zu:%zu: (internal error): unknown binary operator %d",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            op);
        break;
    }
    }

    Instruction* I = cast<Instruction>(res);
    if (I)
    {
        I->setDebugLoc(dloc);
    }

    val->ir_value  = res;
    val->ssa_type  = op1->ssa_type;
    val->is_lvalue = false;

    return val;
}

value_ptr_t 
llvmir_compiler_t::emit_cmp_binary(compiler_ctx_t* ctx, binary_expr_node_ptr_t node)
{
    value_ptr_t val(new value_t());
    auto bn = PCAST(binary_expr_node_t, node);

    value_ptr_t e1 = emit_expression(ctx, bn->operand1);
    value_ptr_t e2 = emit_expression(ctx, bn->operand2);

    check_ssa_type_equality(ctx, e1, e2, node);

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
        ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
    );

    Value* lhs = emit_r_value(ctx, e1, &dloc);
    Value* rhs = emit_r_value(ctx, e2, &dloc);

    Value* cmp = nullptr;

    switch (node->op_id)
    {
    case expr_less: cmp = ctx->llvm_ir->CreateICmpSLT(lhs, rhs); break;
    case expr_lesseeq: cmp = ctx->llvm_ir->CreateICmpSLE(lhs, rhs); break;
    case expr_more: cmp = ctx->llvm_ir->CreateICmpSGT(lhs, rhs); break;
    case expr_moreeq: cmp = ctx->llvm_ir->CreateICmpSGE(lhs, rhs); break;
    case expr_logicaleq: cmp = ctx->llvm_ir->CreateICmpEQ(lhs, rhs); break;
    case expr_noteq: cmp = ctx->llvm_ir->CreateICmpNE(lhs, rhs); break;
    default: 
    { 
        throw compiler_exeption_t("%s:%zu:%zu: (internal error): unknown comparison operator %d",
            ctx->ast->source_id.c_str(),
            node->line,
			node->pos,
			node->op_id
        );
        break; 
    }
    }
    Instruction* I = cast<Instruction>(cmp);
    if (I)
    {
        I->setDebugLoc(dloc);
    }

    // cmp is i1 - extend to operand integer type for downstream compatibility
    Value* ext = ctx->llvm_ir->CreateZExt(cmp, e1->ssa_type->ir_type);
    I = cast<Instruction>(ext);
    if (I)
    {
        I->setDebugLoc(dloc);
    }
    
    val->ir_value = ext;
    val->ssa_type = e1->ssa_type;
    val->is_lvalue = false;
	return val;
}

value_ptr_t 
llvmir_compiler_t::emit_assign_arithmetic_binary(compiler_ctx_t* ctx, binary_expr_node_ptr_t node)
{
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
        throw compiler_exeption_t("%s:%zu:%zu: (internal error): unknown binary assign operator %d",
            ctx->ast->source_id.c_str(),
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
llvmir_compiler_t::emit_expression(compiler_ctx_t* ctx, expr_node_ptr_t node)
{
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

    default:

        throw new compiler_exeption_t("%s:%zu:%zu: (internal error): invalid operation %d '%s'",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->op_id,
            node->content.c_str());

        break;

    }

    return val;
}

value_ptr_t llvmir_compiler_t::emit_comma(compiler_ctx_t* ctx, comma_expr_node_ptr_t node)
{
    value_ptr_t val(new value_t());
    value_ptr_t last;
    for (auto& a : node->arg_list->args) {
        last = emit_expression(ctx, a);
    }
    // return rvalue of last
    if (last) {
        llvm::DebugLoc dloc = llvm::DILocation::get(
            *ctx->llvm_ctx,
            node->line,
            node->pos,
            ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
        );
        // if lvalue convert to rvalue
        last->ir_value = emit_r_value(ctx, last, &dloc);
        last->is_lvalue = false;
        val = last;
    }
	return val;

}

value_ptr_t 
llvmir_compiler_t::emit_raw_assign(compiler_ctx_t* ctx, value_ptr_t lhs, value_ptr_t rhs, node_ptr_t node)
{
    value_ptr_t val(new value_t());

    check_lvalue(ctx, lhs, node);
    check_ssa_type_equality(ctx, lhs, rhs, node);

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
        ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
    );
    auto inst = ctx->llvm_ir->CreateStore(emit_r_value(ctx, rhs, &dloc), lhs->ir_value);
    inst->setDebugLoc(dloc);

    val->ir_value  = inst;
    val->ssa_type  = rhs->ssa_type;
    val->is_lvalue = false;

	return val;
}

value_ptr_t 
llvmir_compiler_t::emit_expr_assign(compiler_ctx_t* ctx, assign_expr_node_ptr_t node)
{

    value_ptr_t op1 = emit_expression(ctx, node->operand1);
    value_ptr_t op2 = emit_expression(ctx, node->operand2);
    
    return emit_raw_assign(ctx,  op1, op2, node);
}

Value* 
llvmir_compiler_t::emit_r_value(compiler_ctx_t* ctx, value_ptr_t val, llvm::DebugLoc* dloc)
{
    if (!val->is_lvalue)
        return val->ir_value;

    // load from address
    auto inst = ctx->llvm_ir->CreateLoad(val->ssa_type->ir_type, val->ir_value);
    inst->setDebugLoc(*dloc);

    return  inst;
}

Value*
llvmir_compiler_t::emit_cast(compiler_ctx_t* ctx, value_ptr_t val, value_type_ptr_t target_type, node_ptr_t node)
{
    if (val->is_lvalue)
    {
       throw compiler_exeption_t("%s:%zu:%zu: (internal error): cannot cast lvalue, expected rvalue",
            ctx->ast->source_id.c_str(),
            node->line,
		    node->pos);
    }

    // int, char 
    throw; // TBD
    
}

value_ptr_t
llvmir_compiler_t::emit_expr_fun_call(compiler_ctx_t* ctx, funcall_expr_node_ptr_t node)
{
    value_ptr_t val(new value_t());

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
        ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
    );
   
	value_ptr_t fun_val = emit_expression(ctx, node->fun_expr);

    std::vector <Value*> args = {};

	for (auto arg : node->arg_list->args)
    {
        value_ptr_t arg_val = emit_expression(ctx, arg);

        llvm::DebugLoc arg_dloc = llvm::DILocation::get(
            *ctx->llvm_ctx,
            arg->line,
            arg->pos,
            ir_sc<Function>(ctx->fun_desc_stack.top()->ir_value)->getSubprogram()
        );
        
        args.push_back(emit_r_value(ctx, arg_val, &arg_dloc));
    }

    auto inst = ctx->llvm_ir->CreateCall(ir_sc<FunctionType>(fun_val->ssa_type->ir_type), emit_r_value(ctx, fun_val, &dloc), args);

    inst->setDebugLoc(dloc);

    return nullptr;
}

value_ptr_t
llvmir_compiler_t::emit_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node)
{
	return emit_literal(ctx, node->literal);
}

value_ptr_t
llvmir_compiler_t::emit_literal(compiler_ctx_t* ctx, literal_node_ptr_t node)
{
    value_ptr_t val(new value_t());
   
    switch (node->lit_type_id)
    {
    case LIT_INT:
    {
        val->ssa_type  = emit_type(ctx, node->type);
	    val->ir_value   = ConstantInt::get(val->ssa_type->ir_type, std::get<int>(node->value), true);
		val->is_lvalue  = false;

	    break;
    }
    case LIT_STRING:
    {
        val->ssa_type   = emit_type(ctx, node->type);
        val->ir_value   = ctx->llvm_ir->CreateGlobalString(std::get<string>(node->value));
        val->is_lvalue  = false;
        
        break;
    }
    case LIT_CHAR:
    {
        auto type = emit_type(ctx, node->type);
        val->ir_value   = ConstantInt::get(type->ir_type, std::get<char>(node->value));
        val->is_lvalue  = false;
        break;
    }
    case LIT_POINTER_VOID:
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: (internal error): unsupported literal type %d", ctx->ast->source_id.c_str(), node->line, node->pos, node->line);
    }
    }

    return val;
        
}

void 
llvmir_compiler_t::check_ssa_type_equality(compiler_ctx_t *ctx, value_ptr_t v1, value_ptr_t v2, node_ptr_t node)
{
    if (v2->ssa_type->ir_type  != v2->ssa_type->ir_type)
    {
        throw new compiler_exeption_t("%s:%zu:%zu: (internal error): attempt to perform operation on incompatible types '%s'",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->content.c_str());
    }
}

void
llvmir_compiler_t::check_lvalue(compiler_ctx_t* ctx, value_ptr_t v, node_ptr_t node)
{
    if (!v->is_lvalue)
    {
        throw new compiler_exeption_t("%s:%zu:%zu: (internal error): need lvalue for the operation '%s'",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->content.c_str());
    }
}