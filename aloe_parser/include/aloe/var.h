#pragma once
#include <string>
#include <map>
#include "node.h"
#include "type.h"

using namespace std;

namespace aloe
{
	struct var_node_t : public node_t
	{
		var_node_t() :node_t(VAR_NODE) {};
		string name;
		type_node_ptr_t type;
	};

	typedef shared_ptr<var_node_t> 
	var_node_ptr_t;

	struct var_node_list_t : public node_t
	{
		var_node_list_t() :node_t(VAR_LIST_NODE) {};
		map<string, var_node_ptr_t> vars;
	};

	typedef shared_ptr<var_node_list_t>
	var_list_node_ptr_t;

}
