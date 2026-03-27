#pragma once
using namespace std;
using namespace llvm;

#include "aloe\compiler.h"
#include "type.h"


namespace aloe
{
    class type_mapper_t {

    public:

        type_mapper_t(DIBuilder& dib, Module& m, LLVMContext& ctx);

        /*** IR ***/

        Type* node_ir_type(node_ptr_t node);

        Type* literal_ir_type(literal_node_ptr_t literal);
        
        FunctionType* fun_ir_type(fun_node_ptr_t fun);

        /*** DI ***/

        DIType* node_di_type(node_ptr_t node);

        DISubroutineType* fun_di_type(fun_node_ptr_t node);

    private:

        LLVMContext& ctx;

        DIBuilder& dib;

        Module& module;
        
    };

	typedef shared_ptr<type_mapper_t> di_mapper_ptr_t;

}



