#include "pch.h"
#include "type_utils.h"
#include "aloe\ast.h"

using namespace aloe;

bool 
type_comparator_t::operator () (const type_node_ptr_t& a, const type_node_ptr_t& b) const
{
	if (a->type_type != b->type_type)
	{
		return a->type_type < b->type_type;
	}
	else
	{
		return a->type_definition < b->type_definition;
	}
}
