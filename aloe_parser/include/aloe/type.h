#pragma once
#include "node.h"
#include "identifier.h"
#include "bridge.h"
#include "aloe/aloe_type.h"

using namespace std;

namespace aloe
{
	struct type_node_t : public node_t
	{
		type_node_t(type_category_e type_type_id, size_t ref_count = 0) :node_t(TYPE_NODE), ref_count(ref_count), type_type_id(type_type_id) {};

		size_t					ref_count;

		type_category_e			type_type_id;

	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

	aloe_type_ptr_t convert_type(type_node_ptr_t type);
	
}


