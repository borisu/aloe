#pragma once
#include <string>
#include <vector>
#include "node.h"
#include "type.h"
#include "var.h"
#include "scope.h"

using namespace std;

namespace aloe
{

	
	struct expr_node_t;
	typedef shared_ptr<expr_node_t> expr_node_ptr_t;

	enum operator_e
	{
		OP_UNKONWN,
		
	} ;

	struct expr_node_t : public scope_node_t
	{
		expr_node_t() :scope_node_t(EXPRESSION_NODE),op(OP_UNKONWN) {}

		vector<expr_node_ptr_t> operands;
	
		string value;

		operator_e op;

	};

	
}

