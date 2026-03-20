#pragma once
using namespace std;
using namespace llvm;

#include "aloe\compiler.h"
#include "type_utils.h"


namespace aloe
{
    class di_mapper_t {

    public:
        di_mapper_t(DIBuilder& dib, Module& m, LLVMContext& ctx);

        Type* to_ir(type_node_ptr_t node);

        DIType* to_di(type_node_ptr_t node);

        FunctionType* fun_to_ir(fun_node_ptr_t node);

		DISubroutineType* fun_to_di(fun_node_ptr_t node);

    private:

        LLVMContext& ctx;

        DIBuilder& dib;

        Module& module;

        std::map<type_node_ptr_t, DIType*, type_comparator_t> cache;
    };

	typedef shared_ptr<di_mapper_t> di_mapper_ptr_t;

}



