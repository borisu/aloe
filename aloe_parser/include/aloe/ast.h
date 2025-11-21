#pragma once

#include <variant>
#include <string>
#include <map>

using namespace std;

namespace aloe
{
	enum unit_type_t
	{
		OBJECT
	};

	struct unit_t
	{
		unit_t(unit_type_t type);

		unit_type_t type;

		string fqn;
	};

	struct object_unit_t : public unit_t
	{
		object_unit_t();
	};

	enum node_type_t
	{
		SCOPE_NODE
	};

	struct node_t
	{
		node_t(node_type_t type, node_t* prev = nullptr);

		node_type_t type;

		node_t *prev;
	};

	typedef std::map<string, unit_t*> id_map_t;

	struct scope_node_t : public node_t
	{
		scope_node_t(node_t* prev = nullptr);

		id_map_t ids;
	};
	
	struct ast_t
	{
		ast_t();

		node_t *root;
	};

	scope_node_t* get_scope_node(node_t *node);

	void print_ast(ast_t *tree);
	
}
