#include "pch.h"
#include "environment.h"

using namespace aloe;

environment_t::environment_t(environment_ptr_t env) :prev(env) 
{

};

bool
environment_t::register_id(identifier_node_ptr_t id, node_ptr_t node)
{
	if (id_map.count(id))
		return false;

    id_map[id] = node;
	return true;
}



node_ptr_t
environment_t::find_id(identifier_node_ptr_t id)
{
    auto curr_env = shared_from_this();

    while (curr_env != nullptr)
    {

        if (curr_env->id_map.count(id) > 0)
        {
            return curr_env->id_map[id];
        }
        curr_env = curr_env->prev;
    }
    return nullptr;
}

