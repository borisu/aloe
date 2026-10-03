#pragma once
#include <memory>
#include "base/defs.h"
#include "lang/context.h"

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

	struct node_t : public context_t, public noncopyable_t
	{
		node_t(node_type_e node_type) : node_type_id(node_type),ignore (false) {}

		node_type_e node_type_id;
		
		bool ignore;

		virtual ~node_t() {}
	};

	typedef proxy_t<node_t_ptr>	node_proxy_t;
	defptr(node_proxy_t);

	struct node_ref_t
	{
		node_t_ptr ref;
	};
	defptr(node_ref_t);

}
