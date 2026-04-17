#pragma once
#include "ir_type.h"


using namespace llvm;
namespace aloe
{
	struct ir_value_t
	{
		ir_value_t() :ir_value(nullptr), ssa_type(new ir_type_t()){}

		Value* ir_value;

		ir_type_ptr_t ssa_type;
		
	};

	typedef shared_ptr<ir_value_t>
	ir_value_ptr_t;

}
