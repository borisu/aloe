#pragma once
#include "node.h"

using namespace std;

namespace aloe
{

	struct type_node_t : public node_t
	{
		type_node_t() :node_t(TYPE_NODE){}

		type_proxy_t_ptr p_type;

		void type(type_proxy_t_ptr type)
		{
			this->p_type = type;
		}

		aloe_type_t_ptr type() const
		{
			return p_type->target;
		}
	};
	defptr(type_node_t);

}