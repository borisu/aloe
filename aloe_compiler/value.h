#pragma once
#include "llvm/IR/Function.h"

using namespace llvm;
namespace aloe
{
	struct value_t
	{
		value_t() :ir_value(nullptr) {}

		Value* ir_value;

		type_ptr_t type;
		
	};

	typedef shared_ptr<value_t> 
	value_ptr_t;

}
