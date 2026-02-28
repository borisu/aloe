#pragma once
#include <string>
#include <vector>
#include <variant>
#include "node.h"
#include "type.h"
#include "var.h"
#include "scope.h"

using namespace std;

namespace aloe
{

	struct literal_node_t;
	typedef shared_ptr<literal_node_t> literal_node_ptr_t;

	struct literal_node_t : public node_t
	{
		literal_node_t() :node_t(LITERAL_NODE) {}

		variant<string, int, char> value;

	};


}

#pragma once
