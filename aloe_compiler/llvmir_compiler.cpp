#include "pch.h"
#include "aloe\compiler.h"
#include "llvmir_compiler.h"


using namespace aloe;
using namespace llvm;

compiler_ptr_t
aloe::create_compiler()
{
    return compiler_ptr_t(new llvmir_compiler_t());
}
    
bool 
llvmir_compiler_t::compile(
    const string& file_name,
    const string& module_name, 
    ast_ptr_t ast, 
    object_type_e obj_type, 
    string& out)
{
    LLVMContext ctx;
    Module module(module_name, ctx);
    DIBuilder dib(module);

    module.addModuleFlag(Module::Warning, "CodeView",1);
    module.addModuleFlag(Module::Warning, "Dwarf Version", 4);
    module.addModuleFlag(Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);

    std::filesystem::path pth(file_name);

    DIFile* file = dib.createFile(pth.filename().string(), pth.parent_path().string());

    DICompileUnit* cu = dib.createCompileUnit(
        dwarf::DW_LANG_C,   // or DW_LANG_C_plus_plus
        file,
        "aloe-frontend",    // producer
        false,              // isOptimized
        "",                 // flags
        0                   // runtime version
    );

    auto* intTy = dib.createBasicType(
        "int",
        32,
        dwarf::DW_ATE_signed
    );

    auto* fnType = dib.createSubroutineType(
        dib.getOrCreateTypeArray({ intTy })
    );

 
    // int xmain()
    FunctionType* FT = FunctionType::get(Type::getInt32Ty(ctx), false);
    Function* fn =
        Function::Create(FT, Function::ExternalLinkage, "xmain", module);

    DISubprogram* sp = dib.createFunction(
        file,               // scope
        "xmain",            // source name
        "xmain",            // linkage name
        file,
        1,                  // line number
        fnType,
        1,                  // scope line
        DINode::FlagZero,
        DISubprogram::SPFlagDefinition
    );

    fn->setSubprogram(sp);

    // Create entry block
    BasicBlock* bb = BasicBlock::Create(ctx, "entry", fn);
    IRBuilder<> ir(ctx);
    ir.SetInsertPoint(bb);

    ir.SetCurrentDebugLocation(
        DILocation::get(ctx, 1, 1, sp)
    );

    // return 0;
    ir.CreateRet(ConstantInt::get(Type::getInt32Ty(ctx), 0));

    dib.finalize();

   
    // Print LLVM IR
    llvm::raw_string_ostream llvmOs(out);
    module.print(llvmOs, nullptr);

    return true;
}
