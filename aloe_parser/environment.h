#pragma once
#include <string>
#include "aloe/ast.h"

using namespace std;

namespace aloe
{
	class environment_t;
	typedef shared_ptr<environment_t> environment_ptr_t;

	class environment_t : public std::enable_shared_from_this<environment_t>
	{
	public:

		environment_t(environment_ptr_t env = nullptr);

		bool register_type(const string& name, node_ptr_t node);

		bool register_var(const string& name, node_ptr_t node);

		node_ptr_t find_type_definition_by_name(const string& name);

		node_ptr_t find_var_definition_by_name(const string& name);

	private:

		typedef map<string, node_ptr_t>
			def_map_t;

		def_map_t  type_defs;

		def_map_t  var_defs;

		environment_ptr_t prev;

	};

}


