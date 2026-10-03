#pragma once
#include <memory>
#include "base/defs.h"
#include "fwd.h"

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
		VAR_NODE,
		TYPE_NODE,
		IDENTFIER_NODE,
		EXPRESSION_NODE,
		ARG_LIST_NODE,
		LITERAL_NODE,
		LAYOUT_NODE,
		MARKER_NODE
	};

	struct node_t;
	defptr(node_t);

	struct node_t 
	{
		node_t(node_type_e node_type) : node_type_id(node_type),ignore (false), line(0), pos(0) {}

		node_type_e node_type_id;
		
		bool ignore;

		int line;

		int pos;

		virtual ~node_t() {}
	};

}
