#pragma once
#include <string>
#include <vector>
#include "node.h"
#include "type.h"
#include "var.h"

using namespace std;

namespace aloe
{

	struct execution_block_node_t : public node_t
	{
		execution_block_node_t() :node_t(EXECUTION_BLOCK_NODE) {}
		vector<node_ptr_t> exec_statements;
	};

	typedef shared_ptr<execution_block_node_t> execution_block_node_ptr_t;

	struct fun_node_t : public node_t
	{
		fun_node_t() :node_t(FUNCTION_NODE),
			is_defined(false) {}

		identifier_node_ptr_t		id;

		type_node_ptr_t				ret_type;

		var_list_node_ptr_t			var_list;

		execution_block_node_ptr_t	exec_block;

		bool is_defined;
	};

	typedef shared_ptr<fun_node_t> fun_node_ptr_t;
}
