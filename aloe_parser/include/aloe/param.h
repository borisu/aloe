#pragma once
#include <string>
#include <variant>
#include "type.h"

using namespace std;

namespace aloe
{
	

	struct param_node_t : public node_t
	{
		param_node_t() :node_t(PARAM_NODE) {};
		string name;
		type_node_ptr_t type;
	};

	typedef shared_ptr<param_node_t> 
	param_node_ptr_t;
	

}
