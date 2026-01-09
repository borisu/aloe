#pragma once
#include <list>
#include "node.h"
#include "type.h"
#include "var.h"

using namespace std;

namespace aloe
{
	struct object_node_t : public node_t
	{
		object_node_t() :node_t(OBJECT_NODE) {}

		string           name;

		list<type_node_ptr_t>    layers;

		var_list_node_ptr_t fields;
	};

	typedef shared_ptr<object_node_t> object_node_ptr_t;

}
	