#pragma once
#include "node.h"
#include "identifier.h"

using namespace std;

namespace aloe
{
	enum type_type_e
	{
		TT_UNKNOWN,
		TT_BUILTIN,
		TT_OBJECT,
		TT_FUNCTION
	};

	struct base_type_t : public node_t
	{
		base_type_t() :node_t(BASE_TYPE_NODE), type_type_id(TT_UNKNOWN){}

		base_type_t(type_type_e type_type, node_ptr_t ast_def) :node_t(BASE_TYPE_NODE), type_type_id(type_type), ast_def(ast_def) {}

		type_type_e				type_type_id;

		identifier_node_ptr_t	tid;

		node_ptr_t				ast_def;
	};

	typedef shared_ptr<base_type_t>
	base_type_ptr_t;

	struct type_node_t : public node_t
	{
		type_node_t(size_t ref_count = 0) :node_t(TYPE_NODE), ref_count(ref_count), base_type (new base_type_t()) {};

		size_t					ref_count;

		base_type_ptr_t			base_type;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

}

