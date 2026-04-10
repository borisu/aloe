#pragma once
#include "llvm/IR/Function.h"
#include "aloe/ast.h"
#include "ir_type.h"
#include "ir_value.h"

using namespace llvm;

namespace aloe
{
	struct ir_var_t
	{
		ir_var_t() :ast_def(nullptr) {}

		ir_value_t value;

		var_node_ptr_t ast_def;

	};

	typedef shared_ptr<ir_var_t>
		ir_var_ptr_t;
}
