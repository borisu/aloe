#pragma once
#include <memory>

using namespace std;

namespace aloe
{
	enum node_type_e
	{
		AST_ROOT_NODE,
		PROG_NODE,
		BUILTIN_TYPE_NODE,
		FUNCTION_NODE,
		EXECUTION_BLOCK_NODE,
		RETURN_NODE,
		BASE_TYPE_NODE,
		TYPE_NODE,
		FUN_TYPE_NODE,
		VAR_NODE,
		IDENTFIER_NODE,
		VAR_LIST_NODE,
		EXPRESSION_NODE,
		ARG_LIST_NODE,
		LITERAL_NODE
	};

	struct node_t;
	typedef shared_ptr<node_t> node_ptr_t;

	struct node_t
	{
		node_t(node_type_e ir_type) : node_type_id(ir_type), line(-1), pos(-1), ignore (false) {}

		node_type_e node_type_id;

		int line;

		int pos;

		bool ignore;

		virtual ~node_t() {}
	};
}
