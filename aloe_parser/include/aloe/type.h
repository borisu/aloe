#pragma once
#include "node.h"
#include "identifier.h"

using namespace std;

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

	struct type_node_t : public node_t
	{
		type_node_t(type_e type_type) :node_t(TYPE_NODE),type_type(type_type), ref_count(0){};
		type_node_t(type_e type_type, node_ptr_t definition) :node_t(TYPE_NODE), type_type(type_type), ref_count(0), definition(definition){};

		size_t     ref_count;
		type_e	   type_type;
		identifier_node_ptr_t id;
		node_ptr_t definition;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

}

