#include "pch.h"
#include "aloe/ast.h"
#include "aloe/defs.h"
#include "parse_exception.h"

using namespace aloe;


aloe_type_ptr_t 
aloe::convert_type(type_node_ptr_t type)
{
	switch (type->type_type_id)
	{
	case TT_VOID:
	{
		return aloe_type_ptr_t(new aloe_void_type_t(TT_VOID));
	}
	case TT_CHAR:
	{
		return aloe_type_ptr_t(new aloe_char_type_t(TT_CHAR));
	}
	case TT_INT:
	{
		return aloe_type_ptr_t(new aloe_int_type_t(TT_INT));
	}
	case TT_FLOAT:
	{
		return aloe_type_ptr_t(new aloe_float_type_t(TT_FLOAT));
	}
	case TT_FUNCTION:
	{
		auto fun_type = PCAST(fun_type_node_t, type);

		if (!fun_type)
		{
			throw parse_exeption_t("%s:%zu:%zu: (internal compiler error): invalid function type node", type->content.c_str(), type->line, type->pos);
		}

		aloe_fun_type_ptr_t fun_t = make_shared<aloe_fun_type_t>();

		fun_t->ret_type = convert_type(fun_type->ret_type);

		for (auto& var : fun_type->param_list->vars_v)
		{
			fun_t->args_type_list.push_back(convert_type(var.second->type));
		}


		return aloe_type_ptr_t(fun_t);
	}
	default:
	{
		throw parse_exeption_t("%s:%zu:%zu: (internal compiler error): unknown type", type->content.c_str(), type->line, type->pos);
	}
	}
}



