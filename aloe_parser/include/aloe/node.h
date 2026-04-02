#pragma once
#include <memory>

using namespace std;

namespace aloe
{
	enum node_type_e
	{
		AST_ROOT_NODE,
		PROG_NODE,
		BUILTIN_NODE,
		/* declaration nodes */
		OBJECT_NODE,
		INH_CHAIN_NODE,
		FUNCTION_NODE,
		EXECUTION_BLOCK_NODE,
		TYPE_NODE,
		VAR_NODE,
		IDENTFIER_NODE,
		VAR_LIST_NODE,
		ARG_LIST_NODE,    
		/* execution nodes */
		EXPRESSION_NODE,
		LITERAL_NODE
	};

	struct node_t;
	typedef shared_ptr<node_t> node_ptr_t;

	struct node_t
	{
		node_t(node_type_e type) : type(type), line(-1), pos(-1) {}

		node_type_e type;

		int line;
		int pos;

		virtual ~node_t() {}
	};
}
