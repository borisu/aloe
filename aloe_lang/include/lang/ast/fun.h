#pragma once
#include <string>
#include <vector>
#include "node.h"
#include "var.h"
#include "expression.h"
#include "marker.h"

using namespace std;

namespace aloe
{
	struct exec_block_node_t : public node_t
	{
		exec_block_node_t() : node_t(EXECUTION_BLOCK_NODE) {}
		vector<node_ptr_t> exec_statements;
	};

	typedef shared_ptr<exec_block_node_t> 
	exec_block_node_ptr_t;

	
	struct fun_node_t : public node_t
	{
		fun_node_t() :node_t(FUNCTION_NODE),
			is_defined(false) {}

		identifier_node_ptr_t	id;

		type_node_ptr_t 		type_node;

		aloe_type_ptr_t			type;
		
		exec_block_node_ptr_t	exec_block;

		marker_node_ptr_t		end_of_fun;
	
		bool is_defined;
	};

	typedef shared_ptr<fun_node_t> fun_node_ptr_t;

	struct return_node_t : public node_t
	{
		return_node_t() :node_t(RETURN_NODE){}

		expr_node_ptr_t return_expr;
	};

	typedef shared_ptr<return_node_t> 
	return_node_ptr_t;

	
}
