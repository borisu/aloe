#pragma once
#include "ir_type.h"

using namespace llvm;
namespace aloe
{
	struct ir_value_t
	{
		ir_value_t() :ir_value(nullptr) {}

		Value* ir_value;

		ir_type_ptr_t type;
		
	};

	typedef shared_ptr<ir_value_t> 
	ir_value_ptr_t;

}
