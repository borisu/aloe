#pragma once
#pragma once
#include <string>
#include <vector>
#include "lang/ast/node.h"
#include "lang/ast/fwd.h"

using namespace std;

namespace aloe
{

	struct type_proxy_node_t : public node_t
	{
		type_proxy_node_t() :node_t(TYPE_PROXY_NODE) {}

		type_proxy_node_t(type_node_t_ptr target) :node_t(TYPE_PROXY_NODE), target(target) {}

		type_node_t_ptr target;

		type_node_t_ptr type()
		{
			return target;
		}

		void type(type_node_t_ptr type)
		{
			this->target = type;
		}

	};

	defptr(type_proxy_node_t);

}

