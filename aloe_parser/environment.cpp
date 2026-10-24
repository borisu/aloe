#include "pch.h"
#include "lang/ast/fwd.h"
#include "lang/ast/node.h"
#include "environment.h"
#include "lang/aloe_exception.h"


using namespace aloe;

#define SCOPE_SEPARATOR "::"

base_modifier_t::base_modifier_t(environment_t_ptr env):prev(env)
{

}

type_proxy_t_ptr
base_modifier_t::register_type(identifier_node_t_ptr idt, type_node_t_ptr type)
{
    return prev->register_type(idt, type);
}

type_proxy_t_ptr
base_modifier_t::find_type(identifier_node_t_ptr idt, bool local_scope)
{
	if (prev == nullptr)
		return nullptr;

    return prev->find_type(idt, local_scope);
}

void 
base_modifier_t::register_object(identifier_node_t_ptr idt, node_t_ptr type)
{
	prev->register_object(idt, type);
}

node_proxy_t_ptr 
base_modifier_t::find_object(identifier_node_t_ptr idt, bool local_scope) 
{
	if (prev == nullptr)
		return nullptr;

	return prev->find_object(idt, local_scope);
}   

context_e
base_modifier_t::curr_ctx()
{
	if (prev == nullptr)
		return CTX_UNKNOWN;

    return prev->curr_ctx();
}

const string&
base_modifier_t::scope_name()
{
	static string global = "";
    if (prev == nullptr)
        return global;

    return prev->scope_name();
}

fun_node_t_ptr
base_modifier_t::curr_fun()
{
	if (prev == nullptr)
		return nullptr;

    return prev->curr_fun();
}

const string&
base_modifier_t::source()
{
	static string empty_string = "";
	if (prev == nullptr)
		return empty_string;

    return prev->source();
}

environment_modifier_t::environment_modifier_t(environment_t_ptr env, string name):
base_modifier_t(env),
scope_name_(name)
{
    if (prev)
    {
        scope_name_ = (prev->scope_name() == SCOPE_SEPARATOR) ? prev->scope_name() + name : prev->scope_name() + "::" + name;
    }
    else
    {
        scope_name_ = name;
    }
};

type_proxy_t_ptr
environment_modifier_t::register_type(identifier_node_t_ptr idt, type_node_t_ptr type)
{
	
	if (type_map.count(idt->name) != 0)
    {
        auto prev = type_map[idt->name];
        prev->target = type;
    }
    else
    {
		// the first pointer stored will serve as placeholder for the type information, 
        // so that when the type is redefined, the existing pointer will be updated with 
		// the new type information. This way, anyone holding a pointer to the type will 
        // see the updated information.
        type_map[idt->name] = newptr(type_proxy_t,type); 
    }

    return type_map[idt->name];
}

type_proxy_t_ptr
environment_modifier_t::find_type(identifier_node_t_ptr idt, bool local_scope)
{
    if (type_map.count(idt->name) > 0)
    {
        return type_map[idt->name];
    }

    if (local_scope)
        return nullptr;

	if (prev == nullptr)
		return nullptr;
    
    return prev->find_type(idt, false);

}

void
environment_modifier_t::register_object(identifier_node_t_ptr idt, node_t_ptr node)
{
    if (obj_map.count(idt->name) != 0)
    {
        auto &proxy = obj_map[idt->name];
		proxy->target->ignore = true;
		proxy->target = node;
    }
    else
    {
        obj_map[idt->name] = make_shared<node_proxy_t>(node);
    }
}

node_proxy_t_ptr
environment_modifier_t::find_object(identifier_node_t_ptr idt, bool local_scope)
{
    if (obj_map.count(idt->name) > 0)
    {
        return obj_map[idt->name];
    }

    if (local_scope)
        return nullptr;

    if (prev == nullptr)
        return nullptr;

    return prev->find_object(idt, false);
}

const string&
environment_modifier_t::scope_name()
{
    return scope_name_;
}

context_modifier_t::context_modifier_t(context_e scope, environment_t_ptr env) : 
    base_modifier_t(env), 
    ctx_id(scope)
{
  	
}

context_e
context_modifier_t::curr_ctx()
{
    return ctx_id;
}



fun_modifier_t::fun_modifier_t(fun_node_t_ptr fun, environment_t_ptr env) :
    base_modifier_t(env),
    fun(fun)
{

}

fun_node_t_ptr 
fun_modifier_t::curr_fun()
{
	return fun;
}

source_modifier_t::source_modifier_t(string source) :
    source_name(source)
{

};

const string& 
source_modifier_t::source() 
{
    return source_name;
}