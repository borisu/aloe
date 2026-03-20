#include "pch.h"
#include "aloe\compiler.h"
#include "compiler_exception.h"
#include "ir_compiler.h"

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
	compiler_ctx.llvm_di        = &dib;
    compiler_ctx.llvm_di_file   = di_file;

    compiler_ctx.ast = ast;

    di_mapper_t mapper(dib, module, ctx);

    compiler_ctx.di_mapper = &mapper;
    

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

void 
llvmir_compiler_t::walk_func(compiler_ctx_t* ctx, fun_node_ptr_t fun)
{
  
    auto ir_fun_type = ctx->di_mapper->fun_to_ir(fun);

    Function* ir_func =
        Function::Create(ir_fun_type, 
            Function::ExternalLinkage, fun->id->name, ctx->llvm_module);
   
    if (fun->is_defined)
    {
        BasicBlock* ir_entry = BasicBlock::Create(*ctx->llvm_ctx, fun->id->name, ir_func);
        ctx->llvm_ir->SetInsertPoint(ir_entry);

        ctx->llvm_ir->CreateRet(ConstantInt::get(Type::getInt32Ty(*ctx->llvm_ctx), 0));
    }
   
    DISubprogram* sp = ctx->llvm_di->createFunction(
        ctx->llvm_di_file,        // Function scope.  
        fun->id->name,            // Function name.
        fun->id->name,            // Mangled function name.
        ctx->llvm_di_file,        // File where this variable is defined.
        fun->line,                // Line number.
        ctx->di_mapper->fun_to_di(fun), // fnType
        fun->line,                 // scope line
        DINode::FlagZero,
        DISubprogram::SPFlagDefinition
	);

    ir_func->setSubprogram(sp);
		
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