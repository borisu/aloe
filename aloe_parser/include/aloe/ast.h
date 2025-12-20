#pragma once

#include <variant>
#include <string>
#include <list>
#include <map>
#include <vector>

#include "aloe/types.h"

using namespace std;

namespace aloe
{
	enum node_type_e
	{
		NAMESPACE_NODE,
		OBJECT_NODE,
		FUNCTION_NODE
	};

	struct node_t;
	typedef shared_ptr<node_t> node_ptr_t;

	struct node_t
	{
		node_t(node_type_e type) : type(type) {}
		node_type_e type;
		node_ptr_t  prev;

		typedef map<string, node_ptr_t>  
		type_map_t;

		type_map_t  type_ids;

		virtual ~node_t() {}
	};

	struct ns_node_t : public node_t
	{
		ns_node_t() :node_t(NAMESPACE_NODE) {};
		
		vector<shared_ptr<ns_node_t>> children;
	};

	typedef shared_ptr<ns_node_t> ns_node_ptr_t;

	struct object_node_t : public node_t
	{
		object_node_t() :node_t(OBJECT_NODE) {}

		string           name;
		list<type_ptr_t>    layers;

	};

	typedef shared_ptr<object_node_t> object_node_ptr_t;

	struct fun_node_t : public node_t
	{
		fun_node_t() :node_t(FUNCTION_NODE) {}
		string           name;
	};

	typedef shared_ptr<fun_node_t> fun_node_ptr_t;

	struct ast_t
	{
		ast_t() {}

		node_ptr_t root;
	};

	typedef shared_ptr<ast_t> ast_t_ptr;
		
}
