#pragma once
#include "base/defs.h"

using namespace llvm;
namespace aloe
{

	struct value_t
	{	
		value_t() :
			ir_value(nullptr), 
			is_lvalue(false), 
			lval_ir_type(nullptr),
			di_type(nullptr) {}

		Value* ir_value;

		bool is_lvalue;

		type_node_t_ptr type;

		Type* lval_ir_type;

		DIType* di_type;
		
	};
	defptr(value_t);

}
