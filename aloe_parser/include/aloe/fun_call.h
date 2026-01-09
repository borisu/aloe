#pragma once

#include "node.h"
#include "type.h"
#include "scope.h"
#include "var.h"
#include "fun.h"
#include "object.h"
#include "prog.h"
#include "param.h"

using namespace std;

namespace aloe
{
	struct func_call_t
	{
		func_call_t() {}

		fun_node_ptr_t fun_def;

		list<param_node_ptr_t> params;

		virtual ~func_call_t() {};
	};

	typedef shared_ptr<func_call_t> func_call_ptr_t;

}


