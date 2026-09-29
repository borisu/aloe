#pragma once
#include <string>
#include <map>
#include "node.h"


using namespace std;

namespace aloe
{

	struct identifier_node_t : public node_t
	{
		identifier_node_t() :node_t(IDENTFIER_NODE){};

		string name;
	};

	typedef shared_ptr<identifier_node_t>
	identifier_node_ptr_t;

}
#pragma once
