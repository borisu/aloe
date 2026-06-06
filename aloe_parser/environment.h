#pragma once
#include <string>
#include "lang/ast/ast.h"

using namespace std;

namespace aloe
{
	class environment_t;
	typedef shared_ptr<environment_t> environment_ptr_t;

	enum scope_e
	{
		CTX_UNKNOWN,
		CTX_GLOBAL,
		CTX_FUNCTION,
		CTX_FUN_ARGS,
		CTX_EXEC_BLOCK,
	};

	class environment_t : public std::enable_shared_from_this<environment_t>
	{
	public:

		environment_t(scope_e scope, environment_ptr_t env = nullptr);

		environment_t(scope_e scope, fun_node_ptr_t fun, environment_ptr_t env = nullptr);

		void register_id(identifier_node_ptr_t id, node_ptr_t node);

		bridge_ptr_t find_id(identifier_node_ptr_t id, bool local_scope=false);

		const scope_e& curr_scope();

		fun_node_ptr_t curr_fun();

		string source_id;

	private:

		struct id_ptr_map_cmp
		{
			bool operator()(const identifier_node_ptr_t& a, const identifier_node_ptr_t& b) const
			{
				return (*a < *b);
			}
		};


		typedef map<identifier_node_ptr_t, bridge_ptr_t, id_ptr_map_cmp>
		bridge_map_t;

		bridge_map_t  bridge_map;

		environment_ptr_t prev;

		scope_e scope_id;

		fun_node_ptr_t fun;

	};

}


