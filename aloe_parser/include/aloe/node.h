#pragma once
#include <memory>

using namespace std;

namespace aloe
{
	enum node_type_e
	{
		AST_ROOT_NODE,
		PROG_NODE,
		/* declaration nodes */
		OBJECT_NODE,
		INH_CHAIN_NODE,
		FUNCTION_NODE,
		TYPE_NODE,
		VAR_NODE,
		VAR_LIST_NODE,
		/* execution nodes */
		FUN_CALL_NODE,
		PARAM_NODE,
	};

	struct node_t;
	typedef shared_ptr<node_t> node_ptr_t;

	struct node_t
	{
		node_t(node_type_e type) : type(type) {}
		node_type_e type;

		virtual ~node_t() {}
	};
}
