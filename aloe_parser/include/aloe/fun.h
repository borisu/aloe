#pragma once
#include <string>
#include "node.h"
#include "type.h"
#include "var.h"
#include "scope.h"

using namespace std;

namespace aloe
{

	struct fun_node_t : public scope_node_t
	{
		fun_node_t() :scope_node_t(FUNCTION_NODE) {}
		string        name;
		type_node_ptr_t ret_type;
		var_list_node_ptr_t params;
	};

	typedef shared_ptr<fun_node_t> fun_node_ptr_t;
}
