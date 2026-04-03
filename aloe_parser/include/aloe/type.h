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
		base_type_t() :node_t(BASE_TYPE_NODE), tt(TT_UNKNOWN){}

		base_type_t(type_type_e type_type, node_ptr_t def) :node_t(BASE_TYPE_NODE), tt(type_type), def(def) {}

		type_type_e				tt;

		identifier_node_ptr_t	tid;

		node_ptr_t				def;
	};

	typedef shared_ptr<base_type_t>
	base_type_ptr_t;

	struct type_node_t : public node_t
	{
		type_node_t(size_t ref_count = 0) :node_t(TYPE_NODE), ref_count(ref_count){};

		size_t					ref_count;

		base_type_ptr_t			bt;
	};

	typedef shared_ptr<type_node_t> 
	type_node_ptr_t;

}

