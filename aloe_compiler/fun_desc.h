#pragma once
#include "type.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace aloe
{
	struct fun_desc_t : public type_t
	{
		fun_desc_t() :ir_func(nullptr) {}

		Function* ir_func;
		
	};

	typedef shared_ptr<fun_desc_t>	fun_desc_ptr_t;

}