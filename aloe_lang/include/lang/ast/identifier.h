#pragma once
#include <string>
#include <map>
#include "node.h"
#include "bridge.h"


using namespace std;

namespace aloe
{
	enum namespace_e
	{
		NS_UNKNOWN,
		NS_OBJ,
		NS_TYPE,
		NS_MODULE
	};

	struct identifier_node_t : public node_t
	{
		identifier_node_t() :node_t(IDENTFIER_NODE){};

		string name;
	};

	typedef shared_ptr<identifier_node_t>
	identifier_node_ptr_t;


}
#pragma once
