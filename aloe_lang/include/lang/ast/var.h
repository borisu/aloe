#pragma once
#include <string>
#include <map>
#include <vector>
#include "fwd.h"
#include "node.h"
#include "identifier.h"

using namespace std;

namespace aloe
{
	struct var_node_t : public node_t
	{
		var_node_t() :node_t(VAR_NODE) {};

		identifier_node_t_ptr	idt;

		type_proxy_t_ptr		p_var_type;

		void var_type(type_proxy_t_ptr type);

		type_node_t_ptr var_type() const;

		string name() const;

		expr_node_t_ptr	initializer;
	};
	defptr(var_node_t);

	typedef vector<var_node_t_ptr> var_vec_t;
	defptr(var_vec_t);

	typedef map<string, var_node_t_ptr>	var_map_t;
	defptr(var_map_t);

	struct var_set_t
	{
		var_vec_t v;
		var_map_t m;
	};
	defptr(var_set_t);

}
