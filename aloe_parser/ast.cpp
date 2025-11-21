#include "pch.h"
#include "aloe/ast.h"
#include "aloe/logger.h"
#include "tinyxml2.h"

using namespace aloe;

unit_t::unit_t(unit_type_t type) :type(type)
{

}

object_unit_t::object_unit_t():unit_t(OBJECT)
{

}

node_t::node_t(node_type_t type , node_t* prev):
	type(type), 
	prev(nullptr)
{

}

scope_node_t::scope_node_t(node_t* prev): node_t(SCOPE_NODE, prev)
{

}

ast_t::ast_t():root(nullptr)
{
	
}

scope_node_t* 
aloe::get_scope_node(node_t* node)
{
	auto curr_node = node;
	while (node && node->type != SCOPE_NODE)
	{
		node = node->prev;
	}

	return (scope_node_t*)curr_node;
}

void aloe::print_ast(ast_t* tree)
{

}


