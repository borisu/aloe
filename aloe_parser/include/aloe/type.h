#pragma once
#include "node.h"
#include "identifier.h"

using namespace std;

namespace aloe
{
	enum syntax_type_e
	{
		SYN_UNKNOWN,
		SYN_INT,
		SYN_VOID,
		SYN_DOUBLE,
		SYN_OPAQUE,
		SYN_CHAR,
		SYN_OBJECT,
		SYN_FUNCTION
	};

	struct type_node_t : public node_t
	{
		type_node_t(syntax_type_e type_type) :node_t(TYPE_NODE),syn_type(type_type), ref_count(0){};
		type_node_t(syntax_type_e type_type, node_ptr_t definition) :node_t(TYPE_NODE), syn_type(type_type), ref_count(0), def_node(definition){};

		size_t					ref_count;
		syntax_type_e			syn_type;
		identifier_node_ptr_t	id;
		node_ptr_t				def_node;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

}

