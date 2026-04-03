#pragma once
#include "node.h"

using namespace std;

namespace aloe
{
	enum builtin_type_e
	{
		BIT_UNKNOWN,
		BIT_INT,
		BIT_VOID,
		BIT_DOUBLE,
		BIT_OPAQUE,
		BIT_CHAR,
	};

	struct builtin_node_t : public node_t
	{
		builtin_node_t(builtin_type_e type_type, size_t ref_count = 0) :node_t(BUILTIN_TYPE_NODE), bit_type(type_type){};
		builtin_type_e 	bit_type;
	};

	typedef shared_ptr<builtin_node_t> 
	builtin_node_ptr_t;
}