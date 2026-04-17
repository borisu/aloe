#include "pch.h"
#include "aloe/ast.h"
#include "aloe/defs.h"

using namespace aloe;

bool 
aloe::operator!=(const type_node_t& t1, const type_node_t& t2)
{
	return !(t1 == t2);
}

bool
aloe::operator!=(const fun_type_node_t& fun_t1, const fun_type_node_t& fun_t2)
{
	return !(fun_t1 == fun_t2);
}

bool 
aloe::operator==(const fun_type_node_t& fun_t1, const fun_type_node_t& fun_t2)
{
	if (fun_t1.var_list->vars_v.size() != fun_t2.var_list->vars_v.size())
	{
		return false;
	}

	if (!(*fun_t1.ret_type == *fun_t2.ret_type))
	{
		return false;
	}

	for (size_t i = 0; i < fun_t1.var_list->vars_v.size(); i++)
	{
		auto arg1 = fun_t1.var_list->vars_v[i];
		auto arg2 = fun_t2.var_list->vars_v[i];
		if (*arg1.second->type != *arg2.second->type)
		{
			return false;
		}
	}

	return true;
}

bool 
aloe::operator==(const type_node_t& t1, const type_node_t& t2)
{
	if (t1.ref_count != t2.ref_count)
	{
		return false;
	}

	if (t1.type_type_id != t2.type_type_id)
	{
		return false;
	}

	switch (t1.type_type_id)
	{
	case TT_BUILTIN:
	{
		return (t1.ast_def->target.get() == t2.ast_def->target.get());
		break;
	}
	case TT_FUNCTION:
	{
		auto fun_t1 = PCAST(fun_node_t, t1.ast_def->target);
		auto fun_t2 = PCAST(fun_node_t, t2.ast_def->target);

		return (fun_t1 == fun_t2);
		break;
	}
	default:

		return false;
	}

	return true;
}


