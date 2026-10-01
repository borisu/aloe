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

		identifier_node_t_ptr	id;

		type_proxy_t_ptr		type;

		expr_node_t_ptr			initializer;
	};

	typedef shared_ptr<var_node_t> 
	var_node_t_ptr;

}
