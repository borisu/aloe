#pragma once
#include "node.h"

using namespace std;

namespace aloe
{

	struct type_node_t : public node_t
	{
		type_node_t() :node_t(TYPE_NODE){}

		type_proxy_t_ptr type;
	};

	typedef shared_ptr<type_node_t> 
	type_node_t_ptr;

}