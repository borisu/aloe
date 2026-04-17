#pragma once
#include "ir_type.h"


using namespace llvm;
namespace aloe
{
	struct ir_value_t
	{
		ir_value_t() :ir_value(nullptr), ssa_type(nullptr){}

		Value* ir_value;

		Type* ssa_type;
		
	};

	typedef shared_ptr<ir_value_t>
	ir_value_ptr_t;


	struct ir_var_t : public ir_value_t
	{
		ir_var_t(var_node_ptr_t ast_def) :ast_def(ast_def) {}

		var_node_ptr_t ast_def;
	};

	typedef shared_ptr<ir_var_t>
	ir_var_ptr_t;

	struct ir_fun_t :public ir_value_t
	{
		ir_fun_t(fun_node_ptr_t ast_def) :ast_def(ast_def) {}

		fun_node_ptr_t ast_def;

	};

	typedef shared_ptr<ir_fun_t>
	ir_fun_ptr_t;


}
