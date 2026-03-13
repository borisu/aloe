#pragma once
#include <list>
#include "node.h"

using namespace std;

namespace aloe
{
	struct prog_node_t : public node_t
	{
		prog_node_t() :node_t(PROG_NODE) {}

		identifier_node_ptr_t module_name;

		vector<node_ptr_t> declarations;

	};

	typedef shared_ptr<prog_node_t> prog_node_ptr_t;

}
