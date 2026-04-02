#pragma once
using namespace std;
using namespace llvm;

#include "aloe\compiler.h"
#include "type.h"


namespace aloe
{
    class type_cache_t {

    public:

        type_cache_t(DIBuilder& dib, Module& m, LLVMContext& ctx);

		type_ptr_t get_type(node_ptr_t node);

    private:

        /*** IR ***/

        Type* ir_get_type(node_ptr_t node);

        Type* ir_get_type_lit(literal_node_ptr_t literal);

        FunctionType* ir_get_type_fun(fun_node_ptr_t fun);

        /*** DI ***/

        DIType* di_get_type(node_ptr_t node);

        DISubroutineType* di_get_type_fun(fun_node_ptr_t node);

        LLVMContext& ctx;

        DIBuilder& dib;

        Module& module;

		map<node_ptr_t, DIType*> di_type_cache;
        
    };

	typedef shared_ptr<type_cache_t> 
    type_cache_ptr_t;

}



