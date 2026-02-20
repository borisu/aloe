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
llvmir_compiler_t::compile(const string& module_name, ast_ptr_t ast, object_type_e obj_type, string& out)
{
    LLVMContext context;
    Module module(module_name, context);

    IRBuilder<> builder(context);

    // int main()
    FunctionType* mainType = FunctionType::get(builder.getInt32Ty(), false);
    Function* mainFunc =
        Function::Create(mainType, Function::ExternalLinkage, "main", module);

    // Create entry block
    BasicBlock* entry = BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // return 0;
    builder.CreateRet(builder.getInt32(0));

    // Print LLVM IR
    llvm::raw_string_ostream llvmOs(out);
    module.print(llvmOs, nullptr);

    return true;
}
