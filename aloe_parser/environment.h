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

		void register_id(identifier_node_ptr_t id, node_ptr_t node);

		bridge_ptr_t find_id(identifier_node_ptr_t id, bool local_scope=false);

		void push_fun(fun_node_ptr_t fun);

		fun_node_ptr_t top_fun();

		string source_id;

	private:

		struct id_ptr_map_cmp
		{
			bool operator()(const identifier_node_ptr_t& a, const identifier_node_ptr_t& b) const
			{
				if (a->name != b->name)
					return a->name < b->name;

				return a->idt_type_id < b->idt_type_id;
			}
		};


		typedef map<identifier_node_ptr_t, bridge_ptr_t, id_ptr_map_cmp>
		bridge_map_t;

		bridge_map_t  bridge_map;

		stack<fun_node_ptr_t> fun_stack;

		environment_ptr_t prev;

	};

}


