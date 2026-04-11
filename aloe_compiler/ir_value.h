#pragma once
#include "ir_type.h"


using namespace llvm;
namespace aloe
{
	struct ir_value_t
	{
		ir_value_t() :ir_value(nullptr), type(new ir_type_t()){}

		Value* ir_value;

		ir_type_ptr_t type;

		Function* sc_function() const // safe cast to llvm::Function
		{
			if (isa<Function>(ir_value))
				(Function*)ir_value;
		}

		
		
	};

	typedef shared_ptr<ir_value_t> 
	ir_value_ptr_t;

}
