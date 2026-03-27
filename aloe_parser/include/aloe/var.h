#pragma once
#include <string>
#include <map>
#include <vector>
#include "node.h"
#include "type.h"

using namespace std;

namespace aloe
{
	struct var_node_t : public node_t
	{
		var_node_t() :node_t(VAR_NODE) {};
		identifier_node_ptr_t	id;
		type_node_ptr_t			type;
	};

	typedef shared_ptr<var_node_t> 
	var_node_ptr_t;

	typedef map<identifier_node_ptr_t, var_node_ptr_t> 
	var_map_t;

	typedef pair<identifier_node_ptr_t, var_node_ptr_t> 
	var_id_t;

	typedef vector<var_id_t> 
	var_vec_t;

	struct var_node_list_t : public node_t
	{
		var_node_list_t() :node_t(VAR_LIST_NODE) {};

		var_map_t vars_m;

		var_vec_t vars_v;
	};

	typedef shared_ptr<var_node_list_t>
	var_list_node_ptr_t;

}
