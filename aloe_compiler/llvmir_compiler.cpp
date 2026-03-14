#include "pch.h"
#include "aloe\compiler.h"
#include "llvmir_compiler.h"

using namespace aloe;
using namespace llvm;
using namespace llvm::dwarf;

aloe::compiler_exeption_t::compiler_exeption_t(const char* format, ...)
{
    
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
}

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
        DW_LANG_ALOE,   
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
	compiler_ctx.ast            = ast;

	bool res = false;
    try
    {
        walk_prog(&compiler_ctx, ast->prog);
		res = true;
    }
    catch (std::exception& e)
    {
        loginl("Error during code generation: %s", e.what());
    }

    dib.finalize();

    // Print LLVM IR
    llvm::raw_os_ostream  llvmOs(out);
    module.print(llvmOs, nullptr);

    return res;
}

void 
llvmir_compiler_t::get_llvm_type(compiler_ctx_t* ctx, type_node_ptr_t node, Type*& type)
{
    switch (node->type_type)
    {
        case TYPE_INT:
        {
            type = Type::getInt32Ty(*ctx->llvm_ctx);
            break;
        }
		case TYPE_CHAR:       
        {
            if (node->ref_count > 0)
                type = PointerType::getInt8Ty(*ctx->llvm_ctx);
            else
				type = Type::getInt8Ty(*ctx->llvm_ctx);
            break;
        }
        default:
        {
            throw compiler_exeption_t("%d:%zu:%zu error: unsupported type  %d",ctx->ast->source_id.c_str(), node->line, node->pos, node->type_type);
        }
            
    }
}

void 
llvmir_compiler_t::get_fun_llvm_type(compiler_ctx_t* ctx, fun_node_ptr_t node, Type*& ret_type, std::vector<Type*>& param_types)
{

}

void 
llvmir_compiler_t::walk_func(compiler_ctx_t* ctx, fun_node_ptr_t fun)
{

    // create function with external linkage
    if (!fun->is_defined)
    {

    }
		
}


void 
llvmir_compiler_t::walk_prog(compiler_ctx_t* ctx, prog_node_ptr_t prog)
{
    for (auto& decl : prog->declarations)
    {
        switch (decl->type)
        {
        case FUNCTION_NODE:
            walk_func(ctx, static_pointer_cast<fun_node_t>(decl));
			break;
        }
    }

}