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
		ID_VAR,
		ID_TYPE,
		ID_MODULE,
	};

	struct identifier_node_t : public node_t
	{
		identifier_node_t() :node_t(IDENTFIER_NODE),id_type(ID_UNKNOWN) {};
		string name;
		identifier_type_e id_type;
	};

	typedef shared_ptr<identifier_node_t>
		identifier_node_ptr_t;

}
#pragma once
