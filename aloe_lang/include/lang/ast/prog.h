#pragma once
#include <list>
#include "node.h"

using namespace std;

namespace aloe
{
	struct prog_node_t : public node_t
	{
		prog_node_t() :node_t(PROG_NODE) {}

		identifier_node_t_ptr module_name;

		vector<node_t_ptr> statements;

	};
	defptr(prog_node_t);

}
