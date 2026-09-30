#pragma once
#pragma once
#include <string>
#include <vector>
#include "lang/ast/node.h"
#include "lang/aloe_type.h"

using namespace std;

namespace aloe
{
	struct layout_node_t : public node_t
	{
		layout_node_t() :node_t(LAYOUT_NODE) {}

		aloe_type_ptr_t type;

	};

	typedef shared_ptr<layout_node_t>
	layout_node_ptr_t;

	


}

