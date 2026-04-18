#pragma once
#include "value_type.h"


using namespace llvm;
namespace aloe
{
	struct value_t
	{	
		value_t() :ir_value(nullptr), ssa_type(new value_type_t()), is_lvalue(false), di_type(nullptr) {}

		Value* ir_value;

		DIType* di_type;

		bool is_lvalue;

		value_type_ptr_t ssa_type;
		
	};

	typedef shared_ptr<value_t>
	value_ptr_t;

}
