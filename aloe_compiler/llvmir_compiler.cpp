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
llvmir_compiler_t::compile(ast_ptr_t ast)
{
    LLVMContext context;
    Module module("simple_module", context);

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
    module.print(outs(), nullptr);

    return true;
}
