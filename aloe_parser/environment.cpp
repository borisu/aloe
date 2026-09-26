#include "pch.h"
#include "environment.h"

using namespace aloe;

base_modifier_t::base_modifier_t(environment_ptr_t env):prev(env)
{

}

bridge_ptr_t
base_modifier_t::register_id(string id, namespace_e ns, node_ptr_t node)
{
    return prev->register_id(id, ns, node);
}

bridge_ptr_t
base_modifier_t::find_id(string id, namespace_e ns, bool local_scope)
{
	if (prev == nullptr)
		return nullptr;

    return prev->find_id(id, ns, local_scope);
}

scope_e
base_modifier_t::curr_scope()
{
	if (prev == nullptr)
		return SCOPE_UNKNOWN;

    return prev->curr_scope();
}

fun_node_ptr_t
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

environment_modifier_t::environment_modifier_t(environment_ptr_t env) :base_modifier_t(env)
{
	
};

bridge_ptr_t
environment_modifier_t::register_id(string id, namespace_e ns, node_ptr_t node)
{
	auto key = std::make_pair(id, ns);
	if (bridge_map.count(key) == 0)
    {
        bridge_map[key] = bridge_ptr_t(new bridge_t(node));
    }
    else
    {
		bridge_map[key]->target->ignore = true; // mark previous definition as ignored, so that it won't be compiled
		bridge_map[key]->target = node;
    }

	return bridge_map[key];
}

bridge_ptr_t
environment_modifier_t::find_id(string id, namespace_e ns, bool local_scope)
{
    auto key = std::make_pair(id, ns);
    if (bridge_map.count(key) > 0)
    {
        return bridge_map[key];
    }

    if (local_scope)
        return nullptr;

	if (prev == nullptr)
		return nullptr;
    
    return prev->find_id(id, ns, false);

}

scope_modifier_t::scope_modifier_t(scope_e scope, environment_ptr_t env) : 
    base_modifier_t(env), 
    scope_id(scope)
{
	
}

scope_e
scope_modifier_t::curr_scope()
{
    return scope_id;
}

fun_modifier_t::fun_modifier_t(fun_node_ptr_t fun, environment_ptr_t env) :
    base_modifier_t(env),
    fun(fun)
{
}


fun_node_ptr_t 
fun_modifier_t::curr_fun()
{
	return fun;
}



source_modifier_t::source_modifier_t(string source) :
    source_name(source)
{};

const string& 
source_modifier_t::source() 
{
    return source_name;
}