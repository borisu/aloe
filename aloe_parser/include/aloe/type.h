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
		type_node_t(type_category_e type_cat_id = TT_UNKNOWN, size_t ref_count = 0) :node_t(TYPE_NODE), ref_count(ref_count), type_cat_id(type_cat_id) {};

		size_t					ref_count;

		type_category_e			type_cat_id;

		aloe_type_ptr_t			aloe_type;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

	struct builtin_type_node_t : public type_node_t
	{
		builtin_type_node_t(builtin_type_e type_type, size_t ref_count = 0) :type_node_t(TT_BUILTIN, ref_count), bit_type(type_type) {};
		builtin_type_e 	bit_type;
	};

	typedef shared_ptr<builtin_type_node_t>
	builtin_type_node_ptr_t;
}


