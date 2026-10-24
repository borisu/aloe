#pragma once
#include <string>
#include "lang/ast/ast.h"

using namespace std;

namespace aloe
{
	class environment_t;
	defptr(environment_t);

	enum context_e
	{
		CTX_UNKNOWN,
		CTX_GLOBAL,
		CTX_FUNCTION,
		CTX_FUN_ARGS,
		CTX_EXEC_BLOCK,
	};

	class environment_t 
	{
	public:
		
		virtual type_proxy_t_ptr register_type(identifier_node_t_ptr id, type_node_t_ptr node) = 0;
		
		virtual type_proxy_t_ptr find_type(identifier_node_t_ptr id, bool local_scope = false) = 0;

		virtual void register_object(identifier_node_t_ptr id, node_t_ptr node) = 0;

		virtual node_proxy_t_ptr find_object(identifier_node_t_ptr id, bool local_scope = false) = 0;

		virtual context_e curr_ctx() = 0;

		virtual const string& scope_name() = 0;

		virtual fun_node_t_ptr curr_fun() = 0;

		virtual const string& source() = 0;
	
	};

	class base_modifier_t : public virtual environment_t
	{
	public:

		base_modifier_t(environment_t_ptr env = nullptr);

		virtual type_proxy_t_ptr register_type(identifier_node_t_ptr id, type_node_t_ptr node) override;

		virtual type_proxy_t_ptr find_type(identifier_node_t_ptr id, bool local_scope) override;

		virtual void register_object(identifier_node_t_ptr id, node_t_ptr node) override;

		virtual node_proxy_t_ptr find_object(identifier_node_t_ptr id, bool local_scope = false) override;

		virtual context_e curr_ctx() override;

		virtual const string& scope_name() override;

		virtual fun_node_t_ptr curr_fun() override;

		virtual const string& source() override;

	protected:

		environment_t_ptr prev;
	};


	class environment_modifier_t : public virtual base_modifier_t
	{
	public:

		environment_modifier_t(environment_t_ptr env = nullptr, string name = "");

		virtual type_proxy_t_ptr register_type(identifier_node_t_ptr id, type_node_t_ptr node) override;

		virtual type_proxy_t_ptr find_type(identifier_node_t_ptr id, bool local_scope) override;

		virtual void register_object(identifier_node_t_ptr id, node_t_ptr node) override;

		virtual node_proxy_t_ptr find_object(identifier_node_t_ptr id, bool local_scope) override;

		virtual const string& scope_name() override;
		
	protected:
	
		typedef map<string, type_proxy_t_ptr>
		type_map_t;

		type_map_t  type_map;

		typedef map<string, node_proxy_t_ptr>
		proxy_map_t;

		proxy_map_t obj_map;

		string scope_name_;
	};

	class context_modifier_t : public virtual base_modifier_t
	{
	public:
		context_modifier_t(context_e scope, environment_t_ptr env = nullptr);

		context_e curr_ctx() override;
				
		context_e ctx_id;
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


