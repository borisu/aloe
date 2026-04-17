#pragma once
#include "node.h"
#include "identifier.h"
#include "bridge.h"

using namespace std;

namespace aloe
{
	enum type_type_e
	{
		TT_UNKNOWN,
		TT_BUILTIN,
		TT_FUNCTION
	};

	struct type_node_t : public node_t
	{
		type_node_t(size_t ref_count = 0) :node_t(TYPE_NODE), ref_count(ref_count), type_type_id(TT_UNKNOWN) {};

		size_t					ref_count;

		type_type_e				type_type_id;

		bridge_ptr_t  		    ast_def;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

	// strict type equality check for type nodes, used for using in maps and sets
	bool operator==(const type_node_t& t1, const type_node_t& t2);

	bool operator!=(const type_node_t& t1, const type_node_t& t2);


	


}

