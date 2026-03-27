#pragma once
#include <string>
#include <memory>

using namespace std;
using namespace llvm;

namespace aloe
{
	enum type_e
	{
		TYPE_UNKNOWN,
		TYPE_INT,
		TYPE_VOID,
		TYPE_DOUBLE,
		TYPE_OPAQUE,
		TYPE_CHAR,
		TYPE_OBJECT,
		TYPE_FUNCTION
	};

	struct type_desc_t
	{
		type_desc_t() : ir_type(nullptr), di_type(nullptr) {}

		Type* ir_type;

		DIType* di_type;
		
	};

	typedef shared_ptr<type_desc_t>
	type_desc_ptr_t;

	struct type_t 
	{
		type_t(type_e type) :type_type(type), ref_count(0) {};
	
		size_t			ref_count;
		type_e			type_type;
		type_desc_ptr_t type_desc;
		
	};


}

