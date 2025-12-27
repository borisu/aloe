#pragma once
#include <string>
#include "node.h"

using namespace std;

namespace aloe
{
	struct fun_node_t : public node_t
	{
		fun_node_t() :node_t(FUNCTION_NODE) {}
		string        name;
	};

	typedef shared_ptr<fun_node_t> fun_node_ptr_t;
}
