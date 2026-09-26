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
		SCOPE_UNKNOWN,
		SCOPE_GLOBAL,
		SCOPE_FUNCTION,
		SCOPE_FUN_ARGS,
		SCOPE_EXEC_BLOCK,
	};

	class environment_t 
	{
	public:
		
		virtual bridge_ptr_t register_id(string id, namespace_e ns, node_ptr_t node) = 0;

		virtual bridge_ptr_t find_id(string id, namespace_e ns, bool local_scope = false) = 0;

		virtual scope_e curr_scope() = 0;

		virtual fun_node_ptr_t curr_fun() = 0;

		virtual const string& source() = 0;
	
	};

	class base_modifier_t : public virtual environment_t
	{
	public:

		base_modifier_t(environment_ptr_t env = nullptr);

		bridge_ptr_t register_id(string id, namespace_e ns, node_ptr_t node) override;

		bridge_ptr_t find_id(string id, namespace_e ns, bool local_scope) override;

		scope_e curr_scope() override;

		fun_node_ptr_t curr_fun() override;

		const string& source() override;

	protected:

		environment_ptr_t prev;
	};


	class environment_modifier_t : public virtual base_modifier_t
	{
	public:

		environment_modifier_t(environment_ptr_t env = nullptr);

		bridge_ptr_t register_id(string id, namespace_e ns, node_ptr_t node) override;

		bridge_ptr_t find_id(string id, namespace_e ns, bool local_scope) override;
		
	protected:

		typedef std::pair<string, namespace_e> id_key_t;

		typedef map<id_key_t, bridge_ptr_t>

		bridge_map_t;

		bridge_map_t  bridge_map;
	};

	class scope_modifier_t : public virtual base_modifier_t
	{
	public:
		scope_modifier_t(scope_e scope, environment_ptr_t env = nullptr);

		scope_e curr_scope() override;
		
		scope_e scope_id;
	};

	class fun_modifier_t : public virtual base_modifier_t
	{
	public:
		fun_modifier_t(fun_node_ptr_t fun = nullptr, environment_ptr_t env = nullptr);
		
		// function scope accessors
		fun_node_ptr_t curr_fun() override;

		fun_node_ptr_t fun;
	};
	 
	class source_modifier_t : public virtual base_modifier_t
	{
	public:
		source_modifier_t(string source);

		const string& source() override;

		string source_name;
	};

}


