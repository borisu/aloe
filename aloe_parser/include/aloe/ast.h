#pragma once
#include "node.h"
#include "type.h"
#include "var.h"
#include "fun.h"
#include "prog.h"
#include "expression.h"
#include "literal.h"
#include "identifier.h"
#include "built_in.h"


using namespace std;

namespace aloe
{
	struct ast_t : public node_t
	{
		ast_t() : node_t(AST_ROOT_NODE) {}

		prog_node_ptr_t prog;

		string source_id;

		virtual ~ast_t() {};
	};

	typedef shared_ptr<ast_t> ast_ptr_t;
		
}

