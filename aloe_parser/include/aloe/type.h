#pragma once
#include "node.h"

using namespace std;

namespace aloe
{
	enum type_e
	{
		UNKNOWN,
		INT,
		DOUBLE,
		CHAR,
		OBJECT,
		FUNCTION
	};

	struct type_node_t
	{
		type_node_t(type_e type_type) :type_type(type_type), ref_count(0){};
		type_node_t(type_e type_type, node_ptr_t definition) :type_type(type_type), ref_count(0), definition(definition){};

		size_t     ref_count;
		type_e	   type_type;
		string	   name;
		node_ptr_t definition;
	};

	typedef shared_ptr<type_node_t> type_node_ptr_t;
}

