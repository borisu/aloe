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

		identifier_node_ptr_t	id;

		type_proxy_ptr_t		type;

		expr_node_ptr_t			initializer;
	};

	typedef shared_ptr<var_node_t> 
	var_node_ptr_t;

}
