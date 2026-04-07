#pragma once
#include "ir_type.h"

using namespace llvm;
namespace aloe
{
	struct ir_value_t
	{
		ir_value_t() :ir_value(nullptr), ir_type(new ir_type_t()),is_lvalue(false){}

		Value* ir_value;

		ir_type_ptr_t ir_type;

		bool is_lvalue;
		
	};

	typedef shared_ptr<ir_value_t> 
	ir_value_ptr_t;

}
