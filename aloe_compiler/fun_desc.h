#pragma once
#include "ir_type.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace aloe
{
	struct fun_desc_t
	{
		fun_desc_t() :ir_fun(nullptr) {}

		Function* ir_fun;

		ir_base_type_ptr_t  type;

		fun_node_ptr_t ast_def;
		
	};

	typedef shared_ptr<fun_desc_t>	
	fun_desc_ptr_t;

}