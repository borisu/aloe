#pragma once
#include "node.h"

using namespace std;

namespace aloe
{
	struct marker_node_t : public node_t
	{
		marker_node_t() :node_t(MARKER_NODE) {}
	};

	typedef shared_ptr<marker_node_t> marker_node_ptr_t;

}
