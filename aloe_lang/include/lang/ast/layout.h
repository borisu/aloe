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

	struct layout_member_list_node_t : public node_t
	{
		layout_member_list_node_t() :node_t(LAYOUT_MEMBER_LIST_NODE) {}

		map<string, layout_member_ptr_t> members_m;

		vector<layout_member_ptr_t> members_v;
	};

	typedef shared_ptr<layout_member_list_node_t> 
	layout_member_list_ptr_t;

	struct layout_node_t;

	typedef shared_ptr<layout_node_t>
	layout_node_ptr_t;

	struct gt_chain_member_t :public node_t
	{
		gt_chain_member_t() :node_t(LAYOUT_GT_MEMBER_NODE), is_virtual(false) {}

		identifier_node_ptr_t id;

		layout_node_ptr_t layout;

		bool is_virtual;
	};

	typedef shared_ptr<gt_chain_member_t>
	gt_chain_member_ptr_t;

	struct gt_chain_node_t : public node_t
	{
		gt_chain_node_t() :node_t(LAYOUT_GT_CHAIN_NODE){}

		list<gt_chain_member_ptr_t> members;
	};

	typedef shared_ptr<gt_chain_node_t> 
	gt_chain_node_ptr_t;

	struct layout_node_t : public node_t
	{
		layout_node_t() :node_t(LAYOUT_NODE){};

		identifier_node_ptr_t id;

		gt_chain_node_ptr_t gt_chain;

		layout_member_list_ptr_t member_list;

		aloe_type_ptr_t type;
		
	};

}
