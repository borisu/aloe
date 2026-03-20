#pragma once
#include "aloe/ast.h"

namespace aloe
{
	struct type_comparator_t
	{
	
		bool operator () (const type_node_ptr_t& a, const type_node_ptr_t& b) const;

		
	};

}

