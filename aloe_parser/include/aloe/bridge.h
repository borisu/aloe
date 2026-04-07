#pragma once
#include "aloe/node.h"
namespace aloe
{
	struct bridge_t
	{
		bridge_t(node_ptr_t target):
			target(target) {}

		node_ptr_t target;
	};

	typedef shared_ptr<bridge_t> 
	bridge_ptr_t;
}
