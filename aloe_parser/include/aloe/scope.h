#pragma once
#include <map>

#include "node.h"
#include "type.h"

using namespace std;

namespace aloe
{
	struct scope_node_t : public node_t
	{
		scope_node_t(node_type_e type) :node_t(type) {};

		typedef map<string, node_ptr_t>
		def_map_t;

		def_map_t  type_defs;

		virtual ~scope_node_t() {};

	};

	typedef shared_ptr<scope_node_t> scope_node_ptr_t;
}
