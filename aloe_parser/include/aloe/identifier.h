#pragma once
#include <string>
#include <map>
#include "node.h"
#include "type.h"

using namespace std;

namespace aloe
{
	enum identifier_type_e
	{
		ID_UNKNOWN,
		ID_NONTYPE,
		ID_TYPE,
		ID_MODULE
	};

	struct identifier_node_t : public node_t
	{
		identifier_node_t() :node_t(IDENTFIER_NODE),idt_type_id(ID_UNKNOWN) {};

		string name;

		identifier_type_e idt_type_id;
	};

	typedef shared_ptr<identifier_node_t>
	identifier_node_ptr_t;

	
	bool operator < (const identifier_node_t& a, const identifier_node_t& b);

}
#pragma once
