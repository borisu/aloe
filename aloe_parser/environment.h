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
		
		virtual void register_type(identifier_node_ptr_t id, aloe_type_ptr_t node) = 0;
		
		virtual aloe_type_ptr_t find_type(identifier_node_ptr_t id, bool local_scope = false) = 0;

		virtual void register_object(identifier_node_ptr_t id, node_ptr_t node) = 0;

		virtual node_proxy_ptr_t find_object(identifier_node_ptr_t id, bool local_scope = false) = 0;

		virtual scope_e curr_scope() = 0;

		virtual fun_node_ptr_t curr_fun() = 0;

		virtual const string& source() = 0;
	
	};

	class base_modifier_t : public virtual environment_t
	{
	public:

		base_modifier_t(environment_ptr_t env = nullptr);

		virtual void register_type(identifier_node_ptr_t id, aloe_type_ptr_t node) override;

		virtual aloe_type_ptr_t find_type(identifier_node_ptr_t id, bool local_scope) override;

		virtual void register_object(identifier_node_ptr_t id, node_ptr_t node) override;

		virtual node_proxy_ptr_t find_object(identifier_node_ptr_t id, bool local_scope = false) override;

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

		virtual void register_type(identifier_node_ptr_t id, aloe_type_ptr_t node) override;

		virtual aloe_type_ptr_t find_type(identifier_node_ptr_t id, bool local_scope) override;

		virtual void register_object(identifier_node_ptr_t id, node_ptr_t node) override;

		virtual node_proxy_ptr_t find_object(identifier_node_ptr_t id, bool local_scope) override;
		
	protected:
	
		typedef map<string, aloe_type_ptr_t>
		type_map_t;

		type_map_t  type_map;

		typedef map<string, node_proxy_ptr_t>
		proxy_map_t;

		proxy_map_t proxy_map;
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


