#include "pch.h"
#include "aloe\compiler.h"
#include "aloe\defs.h"
#include "compiler_exception.h"
#include "ir_compiler.h"
#include "fun_desc.h"

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
llvmir_compiler_t::walk_built_in(compiler_ctx_t* ctx, builtin_node_ptr_t node)
{
    
    ir_base_type_ptr_t out(new ir_base_type_t());
	out->ast_def = node;

    switch (node->bit_type)
    {
    case BIT_INT:
    {
        out->irt = Type::getInt32Ty(*ctx->llvm_ctx);
        out->dit = type_cache.get_dit(0, out->irt, ctx->llvm_di->createBasicType("int", 32, dwarf::DW_ATE_signed));
		out->irtt = IRT_INTEGER;

        break;
    }
    case BIT_CHAR:
    {
        out->irt = Type::getInt8Ty(*ctx->llvm_ctx);
        out->dit = type_cache.get_dit(0, out->irt, ctx->llvm_di->createBasicType("char", 8, dwarf::DW_ATE_signed_char));
        out->irtt = IRT_INTEGER;
        break;
    }
    case BIT_VOID:
    case BIT_OPAQUE:
    {
        out->irt = Type::getVoidTy(*ctx->llvm_ctx);
        out->dit = type_cache.get_dit(0, out->irt, nullptr);
        out->irtt = IRT_OPAQUE;

        break;
    }
    case BIT_DOUBLE:
    {
        out->irt = Type::getDoubleTy(*ctx->llvm_ctx);
        out->dit = type_cache.get_dit(0, out->irt, ctx->llvm_di->createBasicType("double",64,dwarf::DW_ATE_float));
        out->irtt = IRT_OPAQUE;
        break;
    }
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: error: unknown built int type '%d'",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->bit_type);
    }
    }

    return out;
}

ir_type_ptr_t
llvmir_compiler_t::walk_type(compiler_ctx_t* ctx, type_node_ptr_t node)
{
    ir_type_ptr_t out(new ir_type_t());

    switch (node->bt->tt)
    {
    case TT_BUILTIN:
    {
        out->bt = walk_built_in(ctx, static_pointer_cast<builtin_node_t>(node->bt->def));
        break;
    }
    case TT_FUNCTION:
    {
        out->bt = walk_fun_type(ctx, static_pointer_cast<fun_type_node_t>(node->bt->def));
        break;

    }
    case TT_OBJECT:
    case TT_UNKNOWN:
    default:
    {
	
        throw  compiler_exeption_t("%s:%zu:%zu: error: unknown type %d",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
            node->bt->tt); ;
    }
    }

	if (node->ref_count == 0)
    {
        out->irt = out->bt->irt;
        out->dit = out->bt->dit;
    }
    else if (node->ref_count > 0)
    {
        out->irt = PointerType::getUnqual(*ctx->llvm_ctx); // collapse to pointer type

        for (int i = 1; i <= node->ref_count; i++)
        {
            out->ref_count = i;
            out->dit = type_cache.get_dit(i , out->irt, ctx->llvm_di->createPointerType(out->dit, 64)); // must preserve the type for each level of indirection for debug info
        }
       
    }
    else
    {
        throw compiler_exeption_t("%s:%zu:%zu: error: negative reference count '%zu' is not allowed for type",
            ctx->ast->source_id.c_str(),
            node->line,
            node->pos,
			node->ref_count);
    }
    

    return out;

}

ir_base_type_ptr_t
llvmir_compiler_t::walk_fun_type(compiler_ctx_t* ctx, fun_type_node_ptr_t node)
{
    ir_base_type_ptr_t out(new ir_base_type_t());
	out->irtt = IRT_FUNCTION;
    out->ast_def = node;

    ir_type_ptr_t rett = walk_type(ctx, node->ret_type);

    SmallVector<Metadata*, 8>   di_sig;
    di_sig.push_back(rett->dit);

    std::vector<Type*>  args_irt;
    for (auto p : node->var_list->vars_m)
    {
        ir_type_ptr_t argt = walk_type(ctx, p.second->type);
        args_irt.push_back(argt->irt);
        di_sig.push_back(argt->dit);
    };

    out->irt = FunctionType::get(rett->irt, args_irt, false);

    out->dit = (DISubroutineType*)type_cache.get_dit(
        0,
        out->irt,
        ctx->llvm_di->createSubroutineType(ctx->llvm_di->getOrCreateTypeArray(di_sig)));

    return out;
}

fun_desc_ptr_t
llvmir_compiler_t::walk_func(compiler_ctx_t* ctx, fun_node_ptr_t node)
{
    ir_base_type_ptr_t
        fun_type = walk_fun_type(ctx, node->fun_type);

    Function* ir_fun =
        Function::Create( (FunctionType*)fun_type->irt,
            Function::ExternalLinkage, node->id->name, ctx->llvm_module);

    DISubprogram* sp = ctx->llvm_di->createFunction(
        ctx->llvm_di_file,        // Function scope.  
        node->id->name,            // Function name.
        node->id->name,            // Mangled function name.
        ctx->llvm_di_file,        // File where this variable is defined.
        node->line,                // Line number.
        (DISubroutineType*) fun_type->dit, // fnType
        node->line,                 // scope line
        DINode::FlagZero,
		node->is_defined ? DISubprogram::SPFlagDefinition : DISubprogram::SPFlagZero
     );

    ir_fun->setSubprogram(sp);

    fun_desc_ptr_t out(new fun_desc_t());
	out->ir_fun = ir_fun;
	out->type = fun_type;
    out->ast_def = node;
    
    if (node->is_defined)
    {
        BasicBlock* ir_entry = BasicBlock::Create(*ctx->llvm_ctx, node->id->name, ir_fun);

        ctx->llvm_ir->SetInsertPoint(ir_entry);

        ctx->llvm_ir->CreateRet(ConstantInt::get(Type::getInt32Ty(*ctx->llvm_ctx), 0));

        for (auto& statement : node->exec_block->exec_statements)
        {
            walk_exec_statement(ctx, statement);
        }

        bool is_broken = llvm::verifyFunction(*ir_fun);

        if (is_broken) {
            throw compiler_exeption_t("%s:%zu:%zu: error: generated IR for function '%s' is broken",
                ctx->ast->source_id.c_str(),
                node->line,
                node->pos,
                node->id->name.c_str());
        }

    }
  
   
    return out;
		
}

void 
llvmir_compiler_t::walk_prog(compiler_ctx_t* ctx, prog_node_ptr_t prog)
{
    for (auto& decl : prog->decl_statements)
    {
        switch (decl->type)
        {
        case FUNCTION_NODE:
            walk_func(ctx, static_pointer_cast<fun_node_t>(decl));
			break;
        }
    }

}

ir_value_ptr_t
llvmir_compiler_t::walk_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node)
{
    return nullptr;
}

void 
llvmir_compiler_t::walk_exec_statement(compiler_ctx_t* ctx, node_ptr_t node)
{
    switch (node->type)
    {
        case EXPRESSION_NODE:
        {
			walk_expression(ctx, static_pointer_cast<expr_node_t>(node));
            break;
		}
    default:
        break;
    }
}

ir_value_ptr_t

llvmir_compiler_t::walk_expression(compiler_ctx_t* ctx, expr_node_ptr_t node)
{
    ir_value_ptr_t val(new ir_value_t());

    switch (node->op)
    {
    case expr_literal:
    {
        val = walk_expr_literal(ctx, PCAST(literal_expr_node_t,node));
        break;
    }
    case expr_funcall:
    {
        val = walk_fun_call(ctx, PCAST(funcall_expr_node_t,node));
        break;
    }
    case expr_identifier:
    {
		val = walk_identifier(ctx, PCAST(identifier_expr_node_t, node));
        break;
    }
    default:
        break;
    }

    return val;
}



ir_value_ptr_t
llvmir_compiler_t::walk_fun_call(compiler_ctx_t* ctx, funcall_expr_node_ptr_t node)
{

	/*Value* callee = walk_expression(ctx, node->function);

    funcPtr = builder.CreateBitCast(funcPtr, fooPtrType);
	

    ctx->llvm_ir->CreateCall(fooFunc, {
    ConstantInt::get(intTy, 10),
    ConstantInt::get(intTy, 20),
    x
        });;*/

    return nullptr;
}

ir_value_ptr_t
llvmir_compiler_t::walk_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node)
{
	return walk_literal(ctx, node->literal);
}

ir_value_ptr_t
llvmir_compiler_t::walk_literal(compiler_ctx_t* ctx, literal_node_ptr_t node)
{
    ir_value_ptr_t val(new ir_value_t());


    switch (node->lit_type)
    {
    case LIT_INT:
    {
	    val->ir_value   =  ConstantInt::get(val->type->irt, std::get<int>(node->value));
	    break;
    }
    case LIT_STRING:
    {
        val->ir_value = ctx->llvm_ir->CreateGlobalString(std::get<string>(node->value));
        break;
    }
    case LIT_CHAR:
    {
        val->ir_value = ConstantInt::get(val->type->irt, std::get<char>(node->value));
        break;
    }
    default:
    {
        throw compiler_exeption_t("%s:%zu:%zu: error: unsupported literal type %d", ctx->ast->source_id.c_str(), node->line, node->pos, node->line);
    }
    }

    return val;
        
}