#pragma once
#include <string>
#include <vector>
#include <variant>
#include "node.h"


using namespace std;

namespace aloe
{

	struct expr_node_t;
	typedef shared_ptr<expr_node_t> expr_node_ptr_t;

	enum operator_e
	{
		OP_UNKONWN,
		OP_LITERAL,
		OP_FUN_CALL,
		OP_IDENTIFIER
	} ;

	struct expr_node_t : public node_t
	{
		expr_node_t() :node_t(EXPRESSION_NODE),op(OP_UNKONWN) {}

		vector<node_ptr_t> operands;

		operator_e op;

	};

	
}

