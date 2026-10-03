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
	struct fun_node_t : public node_t
	{
		fun_node_t() :node_t(FUNCTION_NODE),
			is_defined(false) {}

		identifier_node_t_ptr	idt;

		vector<node_t_ptr>		statements;

		marker_node_t_ptr		end_of_fun;

		type_proxy_t_ptr	    p_fun_type;

		type_node_t_ptr fun_type() const
		{
			return p_fun_type->target;
		}

		void fun_type(type_proxy_t_ptr fun_type)
		{
			this->p_fun_type = fun_type;
		}
		

		bool is_defined;
	};
	defptr(fun_node_t);

	
	struct return_node_t : public node_t
	{
		return_node_t() :node_t(RETURN_NODE){}

		expr_node_t_ptr return_expr;
	};
	defptr(return_node_t);
	
}
