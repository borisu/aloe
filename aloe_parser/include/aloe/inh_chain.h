#pragma once
#include <list>
#include "node.h"
#include "type.h"
#include "var.h"

using namespace std;

namespace aloe
{
	struct inh_chain_node_t : public node_t
	{
		inh_chain_node_t() :node_t(INH_CHAIN_NODE) {}

		list<type_node_ptr_t>    layers;
	};

	typedef shared_ptr<inh_chain_node_t> inh_chain_node_ptr_t;

}

