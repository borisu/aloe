#pragma once

#include "node.h"
#include "type.h"
#include "scope.h"
#include "var.h"
#include "fun.h"
#include "object.h"
#include "prog.h"
#include "fun_call.h"
#include "inh_chain.h"


using namespace std;

namespace aloe
{
	struct ast_t : public node_t
	{
		ast_t() : node_t(AST_ROOT_NODE) {}

		prog_node_ptr_t prog;

		virtual ~ast_t() {};
	};

	typedef shared_ptr<ast_t> ast_ptr_t;
		
}

