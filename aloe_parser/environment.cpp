#include "pch.h"
#include "environment.h"

using namespace aloe;

environment_t::environment_t(environment_ptr_t env) :prev(env) 
{
	this->source_id = env ? env->source_id : "";
};

void
environment_t::register_id(identifier_node_ptr_t id, node_ptr_t node)
{
	if (bridge_map.count(id) == 0)
    {
        bridge_map[id] = bridge_ptr_t(new bridge_t(node));
    }
    else
    {
		bridge_map[id]->target->ignore = true; // mark previous definition as ignored, so that it won't be compiled
		bridge_map[id]->target = node;
    }
}


bridge_ptr_t
environment_t::find_id(identifier_node_ptr_t id)
{
    auto curr_env = shared_from_this();

    while (curr_env != nullptr)
    {
        if (curr_env->bridge_map.count(id) > 0)
        {
            return curr_env->bridge_map[id];
        }
        curr_env = curr_env->prev;
    }
    return nullptr;

}

