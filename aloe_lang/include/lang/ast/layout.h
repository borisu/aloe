#pragma once
#include <string>
#include <map>
#include "node.h"
#include "type.h"

using namespace std;

namespace aloe
{
	struct layout_member_node_t : public node_t
	{
		layout_member_node_t() :node_t(LAYOUT_MEMBER_NODE) {}
		string name;
		type_node_ptr_t type_node;
		aloe_type_ptr_t type;
	};
	
	typedef shared_ptr<layout_member_node_t> 
	layout_member_ptr_t;


	struct layout_node_t : public node_t
	{
		layout_node_t() :node_t(LAYOUT_NODE){};

		aloe_type_ptr_t type;
	};

	typedef shared_ptr<layout_node_t>
	layout_node_ptr_t;


}
