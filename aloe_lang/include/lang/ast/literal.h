#pragma once
#include <string>
#include <vector>
#include <variant>
#include "node.h"
#include "var.h"
#include "lang/aloe_type.h"


using namespace std;

namespace aloe
{

	struct literal_node_t;
	typedef shared_ptr<literal_node_t> literal_node_t_ptr;

	enum literal_type_e
	{
		LIT_UNKNOWN,
		LIT_STRING,
		LIT_INT,
		LIT_CHAR,
		LIT_POINTER_VOID
	};

	struct literal_node_t : public node_t
	{
		literal_node_t() :node_t(LITERAL_NODE), 
			lit_type_id(LIT_UNKNOWN) {}

		literal_type_e lit_type_id;

		type_proxy_t_ptr type;

		variant<string, int, char> value;

	};


}

#pragma once
