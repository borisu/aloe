#pragma once

using namespace llvm;
namespace aloe
{
	enum value_type_e
	{
		VALUE_DIRECT,
		VALUE_PTR
	};

	struct value_t
	{	
		value_t(value_type_e type = VALUE_DIRECT) :
			ir_value(nullptr), 
			is_lvalue(false), 
			lval_type(nullptr),
			di_type(nullptr) {}

		value_type_e type_id;

		Value* ir_value;

		bool is_lvalue;

		aloe_type_ptr_t aloe_type;

		Type* lval_type;
		Type* ptr_type;

		DIType* di_type;
		
	};

	typedef shared_ptr<value_t>
	value_ptr_t;

}
