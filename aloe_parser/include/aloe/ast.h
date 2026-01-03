#pragma once

#include "node.h"
#include "type.h"
#include "scope.h"
#include "var.h"
#include "fun.h"
#include "object.h"
#include "prog.h"


using namespace std;

namespace aloe
{
	struct ast_t
	{
		ast_t() {}

		prog_node_ptr_t root;

		virtual ~ast_t() {};
	};

	typedef shared_ptr<ast_t> ast_ptr_t;
		
}

