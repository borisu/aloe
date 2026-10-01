#include "pch.h"
#include "environment.h"
#include "lang/aloe_exception.h"


using namespace aloe;

base_modifier_t::base_modifier_t(environment_t_ptr env):prev(env)
{

}

type_proxy_t_ptr
base_modifier_t::register_type(identifier_node_t_ptr id, aloe_type_t_ptr node)
{
    return prev->register_type(id, node);
}

type_proxy_t_ptr
base_modifier_t::find_type(identifier_node_t_ptr id, bool local_scope)
{
	if (prev == nullptr)
		return nullptr;

    return prev->find_type(id, local_scope);
}

void 
base_modifier_t::register_object(identifier_node_t_ptr id, node_t_ptr node)
{
	prev->register_object(id, node);
}

node_proxy_t_ptr 
base_modifier_t::find_object(identifier_node_t_ptr id, bool local_scope) 
{
	if (prev == nullptr)
		return nullptr;

	return prev->find_object(id, local_scope);
}   

scope_e
base_modifier_t::curr_scope()
{
	if (prev == nullptr)
		return SCOPE_UNKNOWN;

    return prev->curr_scope();
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

environment_modifier_t::environment_modifier_t(environment_t_ptr env) :base_modifier_t(env)
{
	
};

type_proxy_t_ptr
environment_modifier_t::register_type(identifier_node_t_ptr id, aloe_type_t_ptr node)
{
	
	if (type_map.count(id->name) != 0)
    {
        auto prev = type_map[id->name];
        prev->target = node;
    }
    else
    {
		// the first pointer stored will serve as placeholder for the type information, 
        // so that when the type is redefined, the existing pointer will be updated with 
		// the new type information. This way, anyone holding a pointer to the type will 
        // see the updated information.
        type_map[id->name] = newptr(type_proxy_t,node); 
    }

    return type_map[id->name];
}

type_proxy_t_ptr
environment_modifier_t::find_type(identifier_node_t_ptr id, bool local_scope)
{
    if (type_map.count(id->name) > 0)
    {
        return type_map[id->name];
    }

    if (local_scope)
        return nullptr;

	if (prev == nullptr)
		return nullptr;
    
    return prev->find_type(id, false);

}

void
environment_modifier_t::register_object(identifier_node_t_ptr id, node_t_ptr node)
{
    if (obj_map.count(id->name) != 0)
    {
        auto &proxy = obj_map[id->name];
		proxy->target->ignore = true;
		proxy->target = node;
    }
    else
    {
        obj_map[id->name] = make_shared<node_proxy_t>(node);
    }
}

node_proxy_t_ptr
environment_modifier_t::find_object(identifier_node_t_ptr id, bool local_scope)
{
    if (obj_map.count(id->name) > 0)
    {
        return obj_map[id->name];
    }

    if (local_scope)
        return nullptr;

    if (prev == nullptr)
        return nullptr;

    return prev->find_object(id, false);
}


scope_modifier_t::scope_modifier_t(scope_e scope, environment_t_ptr env) : 
    base_modifier_t(env), 
    scope_id(scope)
{
	
}

scope_e
scope_modifier_t::curr_scope()
{
    return scope_id;
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