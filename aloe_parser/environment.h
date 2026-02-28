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

		bool register_id(identifier_node_ptr_t	id, node_ptr_t node);

		node_ptr_t find_id(identifier_node_ptr_t id);

	private:

		struct id_ptr_cmp
		{
			bool operator()(const identifier_node_ptr_t& a, const identifier_node_ptr_t& b) const
			{
				if (a->name != b->name)
					return a->name < b->name;
				return a->id_type < b->id_type;
			}
		};

		typedef map<identifier_node_ptr_t, node_ptr_t, id_ptr_cmp>
		def_map_t;

		def_map_t  id_map;

		environment_ptr_t prev;

	};

}


