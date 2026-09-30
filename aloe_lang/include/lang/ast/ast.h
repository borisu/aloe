#pragma once
#include "node.h"
#include "var.h"
#include "fun.h"
#include "prog.h"
#include "expression.h"
#include "literal.h"
#include "identifier.h"
#include "type.h"
#include "layout.h"


using namespace std;

namespace aloe
{
	struct ast_t : public node_t
	{
		ast_t() : node_t(AST_ROOT_NODE) {}

		node_ptr_t root;

		string source_id;

		virtual ~ast_t() {};
	};

	typedef shared_ptr<ast_t> ast_ptr_t;

		
}

