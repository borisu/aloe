#pragma once
#include <memory>
#include "lang/aloe_type.h"

using namespace std;

namespace aloe
{
	enum node_type_e
	{
		AST_ROOT_NODE,
		PROG_NODE,
		FUNCTION_NODE,
		EXECUTION_BLOCK_NODE,
		RETURN_NODE,
		TYPE_NODE,
		VAR_NODE,
		IDENTFIER_NODE,
		VAR_LIST_NODE,
		EXPRESSION_NODE,
		ARG_LIST_NODE,
		LITERAL_NODE,
		LAYOUT_NODE,
		LAYOUT_MEMBER_LIST_NODE,
		LAYOUT_MEMBER_NODE,
		LAYOUT_GT_CHAIN_NODE,
		LAYOUT_GT_MEMBER_NODE,
		MARKER_NODE
	};

	struct node_t;
	typedef shared_ptr<node_t> node_ptr_t;

	struct node_t
	{
		node_t(node_type_e node_type) : node_type_id(node_type), line(-1), pos(-1), ignore (false) {}

		node_type_e node_type_id;

		size_t line;

		size_t pos;
		
		bool ignore;

		virtual ~node_t() {}
	};

	struct node_proxy_t
	{
		node_proxy_t(node_ptr_t target) : target(target) {}
		node_ptr_t target;
	};

	typedef shared_ptr<node_proxy_t> 
	node_proxy_ptr_t;
}
