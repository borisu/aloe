#pragma once
#include "node.h"
#include "identifier.h"

using namespace std;

namespace aloe
{
	enum type_type_e
	{
		TT_UNKNOWN,
		TT_BUILTIN,
		TT_OBJECT,
		TT_FUNCTION
	};

	struct type_node_t : public node_t
	{
		type_node_t(type_type_e type_type, size_t ref_count = 0) :node_t(TYPE_NODE),tt(type_type), ref_count(ref_count){};
		type_node_t(type_type_e type_type, node_ptr_t def, size_t ref_count = 0) :node_t(TYPE_NODE), tt(type_type), ref_count(ref_count), def(def){};

		size_t					ref_count;
		type_type_e				tt;
		identifier_node_ptr_t	tid;
		node_ptr_t				def;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

}

