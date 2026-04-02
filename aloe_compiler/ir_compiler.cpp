#include "pch.h"
#include "aloe\compiler.h"
#include "aloe\defs.h"
#include "compiler_exception.h"
#include "ir_compiler.h"
#include "fun_desc.h"

using namespace aloe;
using namespace llvm;
using namespace llvm::dwarf;


#define DW_LANG_ALOE (DW_LANG_lo_user+1)

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

type_ptr_t
llvmir_compiler_t::walk_type(compiler_ctx_t* ctx, type_node_ptr_t node)
{
	type_ptr_t type(new type_t());

    switch (node->syn_type)
    {
    case SYN_INT:
    {
        type->irt = Type::getInt32Ty(*ctx->llvm_ctx);
        break;
	}
    case SYN_CHAR:
    {
        type->irt = Type::getInt8Ty(*ctx->llvm_ctx);
        break;
    }
    case SYN_VOID:
    {
        type->irt = Type::getVoidTy(*ctx->llvm_ctx);
        break;
    }
    case SYN_DOUBLE:
    {
        type->irt = Type::getDoubleTy(*ctx->llvm_ctx);
        break;
    }
    case SYN_OPAQUE:
    {
        type->irt = PointerType::getUnqual(*ctx->llvm_ctx);
        break;
    }
    case SYN_FUNCTION:
    case SYN_OBJECT:
    case SYN_UNKNOWN:
    default:
    {
        throw;
    }
    }

    return type;

}

void 
llvmir_compiler_t::walk_func(compiler_ctx_t* ctx, fun_node_ptr_t fun)
{
    // return type
	type_ptr_t ret_irt = walk_type(ctx, fun->ret_type);

    // params
    std::vector<Type*> args_irt;
    for (auto p : fun->var_list->vars_m)
    {
        type_ptr_t arg_irt = walk_type(ctx, p.second->type);
        args_irt.push_back(arg_irt->irt);
    };

    auto fun_irt =  FunctionType::get(ret_irt->irt, args_irt, false);

    Function* fun_ir =
        Function::Create(fun_irt,
            Function::ExternalLinkage, fun->id->name, ctx->llvm_module);
   
    if (!fun->is_defined)
        return;
    
    BasicBlock* ir_entry = BasicBlock::Create(*ctx->llvm_ctx, fun->id->name, fun_ir);

    ctx->llvm_ir->SetInsertPoint(ir_entry);

    //ctx->llvm_ir->CreateRet(ConstantInt::get(Type::getInt32Ty(*ctx->llvm_ctx), 0));
   
    /*DISubprogram* sp = ctx->llvm_di->createFunction(
        ctx->llvm_di_file,        // Function scope.  
        fun->id->name,            // Function name.
        fun->id->name,            // Mangled function name.
        ctx->llvm_di_file,        // File where this variable is defined.
        fun->line,                // Line number.
        ctx->type_cache->fun_di_type(fun), // fnType
        fun->line,                 // scope line
        DINode::FlagZero,
        DISubprogram::SPFlagDefinition
	);*/

    
    for (auto& statement : fun->exec_block->exec_statements)
    {
        walk_exec_statement(ctx, statement);
    }

    //fun_ir->setSubprogram(sp);

    bool is_broken = llvm::verifyFunction(*fun_ir);

    if (is_broken) {
        throw compiler_exeption_t("%s:%zu:%zu: error: generated IR for function '%s' is broken", 
            ctx->ast->source_id.c_str(), 
            fun->line, 
            fun->pos, 
			fun->id->name.c_str());
    }
		
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

value_ptr_t 
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

value_ptr_t
llvmir_compiler_t::walk_expression(compiler_ctx_t* ctx, expr_node_ptr_t node)
{
	value_ptr_t val(new value_t());
    switch (node->op)
    {
    case expr_literal:
    {
        val = walk_expr_literal(ctx, PCAST(literal_expr_node_t,node));
        break;
    }
    case expr_funcall:
    {
        val = walk_fun_call (ctx, PCAST(funcall_expr_node_t,node));
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



value_ptr_t
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

value_ptr_t
llvmir_compiler_t::walk_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node)
{
	return walk_literal(ctx, node->literal);
}

value_ptr_t
llvmir_compiler_t::walk_literal(compiler_ctx_t* ctx, literal_node_ptr_t node)
{
	value_ptr_t val(new value_t());

    //val->type = ctx->type_cache->get_type(node);


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