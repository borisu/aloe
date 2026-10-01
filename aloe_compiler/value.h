#pragma once

using namespace llvm;
namespace aloe
{

	struct value_t
	{	
		value_t() :
			ir_value(nullptr), 
			is_lvalue(false), 
			lval_type(nullptr),
			di_type(nullptr) {}

		Value* ir_value;

		bool is_lvalue;

		aloe_type_t_ptr aloe_type;

		Type* lval_type;

		DIType* di_type;
		
	};

	typedef shared_ptr<value_t>
	value_t_ptr;

}
