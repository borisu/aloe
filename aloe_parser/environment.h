#pragma once
#include <string>
#include "lang/ast/ast.h"

using namespace std;

namespace aloe
{
	class environment_t;
	typedef shared_ptr<environment_t> environment_t_ptr;

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
		
		virtual type_proxy_t_ptr register_type(identifier_node_t_ptr id, aloe_type_t_ptr node) = 0;
		
		virtual type_proxy_t_ptr find_type(identifier_node_t_ptr id, bool local_scope = false) = 0;

		virtual void register_object(identifier_node_t_ptr id, node_t_ptr node) = 0;

		virtual node_proxy_t_ptr find_object(identifier_node_t_ptr id, bool local_scope = false) = 0;

		virtual scope_e curr_scope() = 0;

		virtual fun_node_t_ptr curr_fun() = 0;

		virtual const string& source() = 0;
	
	};

	class base_modifier_t : public virtual environment_t
	{
	public:

		base_modifier_t(environment_t_ptr env = nullptr);

		virtual type_proxy_t_ptr register_type(identifier_node_t_ptr id, aloe_type_t_ptr node) override;

		virtual type_proxy_t_ptr find_type(identifier_node_t_ptr id, bool local_scope) override;

		virtual void register_object(identifier_node_t_ptr id, node_t_ptr node) override;

		virtual node_proxy_t_ptr find_object(identifier_node_t_ptr id, bool local_scope = false) override;

		scope_e curr_scope() override;

		fun_node_t_ptr curr_fun() override;

		const string& source() override;

	protected:

		environment_t_ptr prev;
	};


	class environment_modifier_t : public virtual base_modifier_t
	{
	public:

		environment_modifier_t(environment_t_ptr env = nullptr);

		virtual type_proxy_t_ptr register_type(identifier_node_t_ptr id, aloe_type_t_ptr node) override;

		virtual type_proxy_t_ptr find_type(identifier_node_t_ptr id, bool local_scope) override;

		virtual void register_object(identifier_node_t_ptr id, node_t_ptr node) override;

		virtual node_proxy_t_ptr find_object(identifier_node_t_ptr id, bool local_scope) override;
		
	protected:
	
		typedef map<string, type_proxy_t_ptr>
		type_map_t;

		type_map_t  type_map;

		typedef map<string, node_proxy_t_ptr>
		proxy_map_t;

		proxy_map_t obj_map;
	};

	class scope_modifier_t : public virtual base_modifier_t
	{
	public:
		scope_modifier_t(scope_e scope, environment_t_ptr env = nullptr);

		scope_e curr_scope() override;
		
		scope_e scope_id;
	};

	class fun_modifier_t : public virtual base_modifier_t
	{
	public:
		fun_modifier_t(fun_node_t_ptr fun = nullptr, environment_t_ptr env = nullptr);
		
		// function scope accessors
		fun_node_t_ptr curr_fun() override;

		fun_node_t_ptr fun;
	};
	 
	class source_modifier_t : public virtual base_modifier_t
	{
	public:
		source_modifier_t(string source);

		const string& source() override;

		string source_name;
	};

}


