#pragma once
#pragma once
#include <string>
#include <vector>
#include "lang/ast/node.h"
#include "lang/aloe_type.h"

using namespace std;

namespace aloe
{
	struct layout_node_t : public node_t
	{
		layout_node_t() :node_t(LAYOUT_NODE) {}

		type_proxy_t_ptr p_layout_type;

		aloe_type_t_ptr layout_type()
		{
			return p_layout_type->target;
		}

		void layout_type(type_proxy_t_ptr type)
		{
			this->p_layout_type = type;
		}

	};
	defptr(layout_node_t);

}

