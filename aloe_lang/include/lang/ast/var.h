#pragma once
#include <string>
#include <map>
#include <vector>
#include "node.h"
#include "literal.h"
#include "expression.h"
#include "type.h"

using namespace std;

namespace aloe
{
	struct var_node_t : public node_t
	{
		var_node_t() :node_t(VAR_NODE) {};

		identifier_node_t_ptr	idt;

		type_proxy_t_ptr		p_var_type;

		void var_type(type_proxy_t_ptr type)
		{
			this->p_var_type = type;
		}

		aloe_type_t_ptr var_type() const
		{
			return p_var_type->target;
		}

		expr_node_t_ptr			initializer;
	};
	defptr(var_node_t);

}
