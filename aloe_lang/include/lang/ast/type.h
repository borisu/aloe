#pragma once
#include "node.h"

using namespace std;

namespace aloe
{
	struct type_node_t;
	typedef shared_ptr<type_node_t> type_node_ptr_t;

	struct var_list_node_t;
	typedef shared_ptr<var_list_node_t>	var_list_node_ptr_t;
	
	struct type_node_t : public node_t
	{
		type_node_t() :node_t(TYPE_NODE) {}

		aloe_type_ptr_t type;

		var_list_node_ptr_t fun_params_node;

		type_node_ptr_t fun_ret_type_node;

		type_node_ptr_t ptr_pointee_type_node;

		type_node_ptr_t arr_element_type_node;

	};

	
}