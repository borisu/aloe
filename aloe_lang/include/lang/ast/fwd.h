#pragma once
#include <memory>
#include "base/defs.h"

using namespace std;

namespace aloe
{
	struct node_t;
	defptr(node_t);

	struct type_node_t;
	defptr(type_node_t);

	typedef proxy_t<node_t_ptr> node_proxy_t;
	defptr(node_proxy_t);

	struct expr_node_t;
	defptr(expr_node_t);

	typedef proxy_t<type_node_t_ptr> type_proxy_t;
	defptr(type_proxy_t);

	
}
