#pragma once
#include "type.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace aloe
{
	struct fun_desc_t
	{
		fun_desc_t() :ir_func(nullptr) {}

		Function* ir_func;

		type_t type;
		
	};

	typedef shared_ptr<fun_desc_t>	
	fun_desc_ptr_t;

}