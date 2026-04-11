#include "pch.h"
#include "aloe\compiler.h"
#include "aloe\defs.h"
#include "aloe\scope_guard.h"
#include "compiler_exception.h"
#include "ir_compiler.h"
#include "ir_fun.h"
#include "utils.h"

using namespace aloe;
using namespace llvm;
using namespace llvm::dwarf;


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



ir_base_type_ptr_t
llvmir_compiler_t::emit_built_in(compiler_ctx_t* ctx, builtin_node_ptr_t node)
{
    
    ir_base_type_ptr_t out(new ir_base_type_t());
	out->ast_def = node;

    switch (node->bit_type)
    {
    case BIT_INT:
    {
        out->ir_type = Type::getInt32Ty(*ctx->llvm_ctx);
        out->di_type = type_cache.get_dit_type(0, out->ir_type, ctx->llvm_di->createBasicType("int", 32, dwarf::DW_ATE_signed));
		out->type_id = IRT_INTEGER;

        break;
    }
    case BIT_CHAR:
    {
        out->ir_type = Type::getInt8Ty(*ctx->llvm_ctx);
        out->di_type = type_cache.get_dit_type(0, out->ir_type, ctx->llvm_di->createBasicType("char", 8, dwarf::DW_ATE_signed_char));
        out->type_id = IRT_CHAR;
        break;
    }
    case BIT_VOID:
    case BIT_OPAQUE:
    {
        out->ir_type = Type::getVoidTy(*ctx->llvm_ctx);
        out->di_type = type_cache.get_dit_type(0, out->ir_type, nullptr);
        out->type_id = IRT_OPAQUE;

        break;
    }
    case BIT_DOUBLE:
    {
        out->ir_type = Type::getDoubleTy(*ctx->llvm_ctx);
        out->di_type = type_cache.get_dit_type(0, out->ir_type, ctx->llvm_di->createBasicType("double",64,dwarf::DW_ATE_float));
        out->type_id = IRT_OPAQUE;
        break;
    }
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: (error): unknown built int type '%d'",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->bit_type);
    }
    }

    return out;
}

ir_type_ptr_t
llvmir_compiler_t::emit_type(compiler_ctx_t* ctx, type_node_ptr_t node)
{
    ir_base_type_ptr_t base_type;

    switch (node->base_type->type_type_id)
    {
    case TT_BUILTIN:
    {
        base_type = emit_built_in(ctx, PCAST(builtin_node_t, node->base_type->ast_def->target));
        break;
    }
    case TT_FUNCTION:
    {
        base_type = emit_fun_type(ctx, PCAST(fun_type_node_t, node->base_type->ast_def->target));
        break;

    }
    case TT_UNKNOWN:
    default:
    {

        throw  compiler_exeption_t("%s:%zu:%zu: (error): unknown type %d",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->base_type->type_type_id); ;
    }
    };

    ir_type_ptr_t out = init_type_from_base(ctx, base_type, node->ref_count);

    if (!out)
    {
        throw compiler_exeption_t("%s:%zu:%zu: (error): invalid type",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos);
    }
    

    return out;

}

ir_base_type_ptr_t
llvmir_compiler_t::emit_fun_type(compiler_ctx_t* ctx, fun_type_node_ptr_t node)
{
    ir_base_type_ptr_t out(new ir_base_type_t());
	out->type_id = IRT_FUNCTION;
    out->ast_def = node;

    ir_type_ptr_t ret_type = emit_type(ctx, node->ret_type);

    SmallVector<Metadata*, 8>   dit_args;
    dit_args.push_back(ret_type->di_type);

    std::vector<Type*>  irt_args;
    for (auto p : node->var_list->vars_m)
    {
        ir_type_ptr_t argt = emit_type(ctx, p.second->type);
        irt_args.push_back(argt->ir_type);
        dit_args.push_back(argt->di_type);
    };

    out->ir_type = FunctionType::get(ret_type->ir_type, irt_args, false);

    auto di_type = ctx->llvm_di->createSubroutineType(ctx->llvm_di->getOrCreateTypeArray(dit_args));

     out->di_type = type_cache.get_dit_type(
        0,
        out->ir_type,
        ctx->llvm_di->createSubroutineType(ctx->llvm_di->getOrCreateTypeArray(dit_args)));

    return out;
}

ir_type_ptr_t 
llvmir_compiler_t::init_type_from_base(compiler_ctx_t* ctx, ir_base_type_ptr_t base_type, size_t ref_count)
{
	ir_type_ptr_t out(new ir_type_t());
	out->ref_count = ref_count;

    if (out->ref_count == 0)
    {
        out->ir_type = base_type->ir_type;
        out->di_type = base_type->di_type;
    }
    else if (out->ref_count > 0)
    {
        out->ir_type = PointerType::getUnqual(*ctx->llvm_ctx); // collapse to pointer type

        for (int i = 1; i <= out->ref_count; i++)
        {
            out->ref_count = i;
            out->di_type = type_cache.get_dit_type(i, out->ir_type, ctx->llvm_di->createPointerType(out->di_type, 64)); // must preserve the type for each level of indirection for debug info
        }

    }
    else
    {
        return nullptr;
    }

    return out;
}

ir_fun_ptr_t
llvmir_compiler_t::emit_fun(compiler_ctx_t* ctx, fun_node_ptr_t node)
{
    if (node->ignore)
        return nullptr;

    
    ir_fun_ptr_t out(new ir_fun_t());

    ir_base_type_ptr_t base_type = emit_fun_type(ctx, node->fun_type);

	out->value->type = init_type_from_base(ctx, base_type, 0);

    out->ast_def = node;

    Function* ir_fun =
        Function::Create( (FunctionType*)base_type->ir_type,
            Function::ExternalLinkage, node->id->name, ctx->llvm_module);

	for (int i = 0; i < ir_fun->arg_size(); i++)
    {
        auto ir_arg = ir_fun->getArg(i);

        auto var_node = node->fun_type->var_list->vars_v[i].second;
        auto var_name = var_node->id->name;

        ir_arg->setName(var_name);

		ir_var_ptr_t var(new ir_var_t());
		var->ast_def        = var_node;
		var->value->ir_value = ir_arg;
		var->value->type     = emit_type(ctx, var_node->type);

		var_cache[var_node] = var;
		
    }
   
    DISubprogram* sp = ctx->llvm_di->createFunction(
        ctx->llvm_di_file,        // Function scope.  
        node->id->name,            // Function name.
        node->id->name,            // Mangled function name.
        ctx->llvm_di_file,        // File where this variable is defined.
        node->line,                // Line number.
        ir_sc<DISubroutineType> (base_type->di_type), // fnType
        node->line,                 // scope line
        DINode::FlagZero,
		node->is_defined ? DISubprogram::SPFlagDefinition : DISubprogram::SPFlagZero
     );

    ir_fun->setSubprogram(sp);

	out->value->ir_value = ir_fun;

    if (node->is_defined)
    {
        BasicBlock* ir_block = BasicBlock::Create(*ctx->llvm_ctx, node->id->name, ir_fun);

        ctx->llvm_ir->SetInsertPoint(ir_block);

        ctx->fun_desc_stack.push(out);

        auto guard = make_scope_exit([&]() { ctx->fun_desc_stack.pop(); }); // walk may throw, make sure to pop the function from stack

        for (auto& statement : node->exec_block->exec_statements)
        {
            emit_exec_statement(ctx, statement);
        }
    }
  
    bool is_broken = llvm::verifyFunction(*ir_fun);

    if (is_broken) {
        throw compiler_exeption_t("%s:%zu:%zu: (error): generated IR for function '%s' is broken",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->id->name.c_str());
    }
   
    fun_cache[node] = out;
    return out;
		
}

void 
llvmir_compiler_t::emit_return(compiler_ctx_t* ctx, return_node_ptr_t node)
{
    ir_value_ptr_t ret_val = emit_expression(ctx, node->return_expr);

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
		((Function*)ctx->fun_desc_stack.top()->value->ir_value)->getSubprogram()
    );

	auto ret_inst = ctx->llvm_ir->CreateRet(ret_val->ir_value);
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
        }
    }

}

ir_value_ptr_t
llvmir_compiler_t::emit_expr_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node)
{
	ir_value_ptr_t out(new ir_value_t());

	switch (node->ast_def->target->node_type_id)
    {
        case FUNCTION_NODE:
        {
			// convert function descrption to value for function call
            ir_fun_ptr_t fun_desc = fun_cache[node->ast_def->target];

			out  = fun_desc->value;

            break;
        }
        case VAR_NODE:
        default:
        {
            throw compiler_exeption_t("%s:%zu:%zu: (error): identifier '%s' points to unsupported type",
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
        case RETURN_NODE:
        {
            emit_return(ctx, PCAST(return_node_t,node));
            break;
        }
    default:
        break;
    }
}

ir_var_ptr_t 
llvmir_compiler_t::emit_var(compiler_ctx_t* ctx, var_node_ptr_t node)
{
	ir_var_ptr_t out(new ir_var_t());

    /*out->value->type = emit_type(ctx, node->type);

    if (ctx->fun_desc_stack.empty())
    {
        out->value->ir_value = new GlobalVariable(
            ctx->llvm_module,
            out->value->type->ir_type,
            false,
            GlobalValue::ExternalLinkage,
            out->value->type->ir_type,
            node->id->name,
        );
    }
    else
    {
        out->value->ir_value = ctx->llvm_ir->CreateAlloca(out->value->type->ir_type, nullptr, node->id->name);
    }

    /*DILocalVariable* var = ctx->llvm_di->createAutoVariable(
        Scope,          // usually DISubprogram or lexical block
        node->id->name, // variable name
        File,           // DIFile*
        LineNo,
        intType
    );*/

	var_cache[node] = out;

    return out;
   
}

ir_value_ptr_t
llvmir_compiler_t::emit_expression(compiler_ctx_t* ctx, expr_node_ptr_t node)
{
    ir_value_ptr_t val(new ir_value_t());

    switch (node->op)
    {
    case expr_literal:
    {
        val = emit_expr_literal(ctx, PCAST(literal_expr_node_t,node));
        break;
    }
    case expr_funcall:
    {
        val = emit_expr_fun_call(ctx, PCAST(funcall_expr_node_t,node));
        break;
    }
    case expr_identifier:
    {
		val = emit_expr_identifier(ctx, PCAST(identifier_expr_node_t, node));
        break;
    }
    default:
        break;
    }

    return val;
}

ir_value_ptr_t
llvmir_compiler_t::emit_expr_fun_call(compiler_ctx_t* ctx, funcall_expr_node_ptr_t node)
{
    ir_value_ptr_t val(new ir_value_t());
   
	ir_value_ptr_t calee = emit_expression(ctx, node->function);

    std::vector <Value*> args = {};

	for (auto arg : node->arg_list->args)
    {
        ir_value_ptr_t arg_val = emit_expression(ctx, arg);
        args.push_back(arg_val->ir_value);
    }
       
    auto inst = ctx->llvm_ir->CreateCall(ir_sc<FunctionType>(calee->type->ir_type), calee->ir_value, args);

    llvm::DebugLoc dloc = llvm::DILocation::get(
        *ctx->llvm_ctx,
        node->line,
        node->pos,
        ir_sc<Function>(ctx->fun_desc_stack.top()->value->ir_value)->getSubprogram()
    );

    inst->setDebugLoc(dloc);

    return nullptr;
}

ir_value_ptr_t
llvmir_compiler_t::emit_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node)
{
	return emit_literal(ctx, node->literal);
}

ir_value_ptr_t
llvmir_compiler_t::emit_literal(compiler_ctx_t* ctx, literal_node_ptr_t node)
{
    ir_value_ptr_t val(new ir_value_t());
   
    switch (node->lit_type_id)
    {
    case LIT_INT:
    {
        val->type       = emit_type(ctx, node->type);
	    val->ir_value   = ConstantInt::get(val->type->ir_type, std::get<int>(node->value), true);
		
	    break;
    }
    case LIT_STRING:
    {
        val->type       = emit_type(ctx, node->type);
        val->ir_value   = ctx->llvm_ir->CreateGlobalString(std::get<string>(node->value));
        
        break;
    }
    case LIT_CHAR:
    {
        val->type       = emit_type(ctx, node->type);
        val->ir_value   = ConstantInt::get(val->type->ir_type, std::get<char>(node->value));
        break;
    }
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: (error): unsupported literal type %d", ctx->ast->source_id.c_str(), node->line, node->pos, node->line);
    }
    }

    return val;
        
}