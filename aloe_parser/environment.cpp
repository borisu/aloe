#include "pch.h"
#include "environment.h"

using namespace aloe;

environment_t::environment_t(environment_ptr_t env) :prev(env) 
{

};

bool
environment_t::register_type(const string& name, node_ptr_t node)
{
	if (type_defs.count(name))
		return false;

	type_defs[name] = node;
	return true;
}


bool 
environment_t::register_var(const string& name, node_ptr_t node)
{
	if (var_defs.count(name))
		return false;

	var_defs[name] = node;
	return true;
}



node_ptr_t
environment_t::find_type_definition_by_name(const string& name)
{
    auto curr_env = shared_from_this();

    while (curr_env != nullptr)
    {
        if (curr_env->type_defs.count(name) > 0)
        {
            return curr_env->type_defs[name];
        }
        curr_env = curr_env->prev;
    }
    return nullptr;
}

node_ptr_t
environment_t::find_var_definition_by_name(const string& name)
{
    auto curr_env = shared_from_this();

    while (curr_env != nullptr)
    {
        if (curr_env->var_defs.count(name) > 0)
        {
            return curr_env->var_defs[name];
        }
        curr_env = curr_env->prev;
    }
    return nullptr;
}
