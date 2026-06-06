#include "pch.h"
#include "lang/ast/ast.h"
#include "base/defs.h"
#include "lang/aloe_exception.h"

using namespace aloe;


bool aloe::operator < (const identifier_node_t& a, const identifier_node_t& b)
{
	if (a.name != b.name)
		return a.name < b.name;

	return a.idt_type_id < b.idt_type_id;
}


