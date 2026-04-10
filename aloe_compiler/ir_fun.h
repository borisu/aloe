#pragma once
#include "ir_type.h"
#include "ir_var.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace aloe
{
	struct ir_fun_t
	{
		ir_fun_t() :ir_value(new ir_value_t()) {}

		ir_value_ptr_t ir_value;

		fun_node_ptr_t ast_def;
		
	};

	typedef shared_ptr<ir_fun_t>	
	ir_fun_ptr_t;

}