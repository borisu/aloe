#pragma once
#include "node.h"

using namespace std;

namespace aloe
{

	struct type_node_t : public node_t
	{
		type_node_t() :node_t(TYPE_NODE){}

		aloe_type_ptr_t atype;
	};

	struct type_node_t;
	typedef shared_ptr<type_node_t> type_node_ptr_t;

}