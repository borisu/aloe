#pragma once
#include <list>
#include "node.h"
#include "scope.h"

using namespace std;

namespace aloe
{
	struct prog_node_t : public scope_node_t
	{
		prog_node_t() :scope_node_t(PROG_NODE) {}

		list<node_ptr_t> statements;
	};

	typedef shared_ptr<prog_node_t> prog_node_ptr_t;

}
