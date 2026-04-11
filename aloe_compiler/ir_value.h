#pragma once
#include "ir_type.h"


using namespace llvm;
namespace aloe
{
	struct ir_value_t
	{
		ir_value_t() :ir_value(nullptr), type(new ir_type_t()){}

		Value* ir_value;

		// this stores original type, for variable the actual type of ir_value may be different 
		// (e.g. alloca instruction has pointer type, but the variable it represents may be int), 
		// this is used for type checking and debug info generation

		ir_type_ptr_t type; 
		
	};

	typedef shared_ptr<ir_value_t> 
	ir_value_ptr_t;

}
