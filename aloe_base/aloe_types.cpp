#include "pch.h"
#include "aloe/aloe_type.h"
#include "aloe/defs.h"

using namespace aloe;

bool
aloe::operator!=(const aloe_type_t& t1, const aloe_type_t& t2)
{
	return !(t1 == t2);
}

bool
aloe::operator==(const aloe_type_t& t1, const aloe_type_t& t2)
{
	
	if (t1.type_id != t2.type_id)
	{
		return false;
	}

	switch (t1.type_id)
	{
	case ALOE_TYPE_CHAR:
	case ALOE_TYPE_VOID:
	case ALOE_TYPE_DOUBLE:
	case ALOE_TYPE_INT:
	{
		return true;
	}
	case ALOE_TYPE_ARRAY:
	{
		return 
			*t1.arr_element_type == *t2.arr_element_type && 
			t1.arr_size == t2.arr_size;
	}
	case ALOE_TYPE_PTR:
	{
		return *t1.ptr_pointee_type == *t2.ptr_pointee_type;
	}	
	case ALOE_TYPE_FUNCTION:
	{
		
		if (t1.fun_param_types.size() != t2.fun_param_types.size())
		{
			return false;
		}

		if (*t1.fun_ret_type != *t2.fun_ret_type)
		{
			return false;
		}

		for (size_t i = 0; i < t1.fun_param_types.size(); i++)
		{
			auto arg1 = t1.fun_param_types[i];
			auto arg2 = t2.fun_param_types[i];
			if (*arg1 != *arg2)
			{
				return false;
			}
		}

		return true;
	}
	case ALOE_TYPE_UNKNOWN:
	default:
	{
		return false;
	}
	}
}

bool aloe::operator < (const aloe_type_t& t1, const aloe_type_t& t2)
{
	if (t1.type_id != t2.type_id)
	{
		return t1.type_id < t2.type_id;
	}

	switch (t1.type_id)
	{
	case ALOE_TYPE_CHAR:
	case ALOE_TYPE_VOID:
	case ALOE_TYPE_DOUBLE:
	case ALOE_TYPE_INT:
	{
		return false;
	}
	case ALOE_TYPE_ARRAY:
	{
		if (t1.arr_size != t2.arr_size)
		{
			return t1.arr_size < t2.arr_size;
		}

		if (*t1.arr_element_type != *t2.arr_element_type)
		{
			return *t1.arr_element_type < *t2.arr_element_type;
		}

		return false;
	}
	case ALOE_TYPE_PTR:
	{
		return *t1.ptr_pointee_type < *t2.ptr_pointee_type;
	}
	case ALOE_TYPE_FUNCTION:
	{
		if (*t1.fun_ret_type != *t2.fun_ret_type)
		{
			return *t1.fun_ret_type < *t2.fun_ret_type;
		}

		if (t1.fun_param_types.size() != t2.fun_param_types.size())
		{
			return t1.fun_param_types.size() < t2.fun_param_types.size();
		}


		for (size_t i = 0; i < t1.fun_param_types.size(); i++)
		{
			auto arg1 = t1.fun_param_types[i];
			auto arg2 = t2.fun_param_types[i];
			if (*arg1 != *arg2)
			{
				return *arg1 < *arg2;
			}
		}

		return false;
	}
	case ALOE_TYPE_UNKNOWN:
	default:
	{
		return false;
	}
	}
}

std::string
aloe_type_t::to_str()
{
	switch (type_id)
	{
	case ALOE_TYPE_CHAR:
		return "char";
	case ALOE_TYPE_VOID:
		return "void";
	case ALOE_TYPE_DOUBLE:
		return "float";
	case ALOE_TYPE_INT:
		return "int";
	case ALOE_TYPE_FUNCTION:
	{
		std::string result = "fun(";
		for (size_t i = 0; i < fun_param_types.size(); i++)
		{
			if (i > 0)
			{
				result += ", ";
			}
			result += fun_param_types[i]->to_str();
		}
		result += ") -> ";
		result += fun_ret_type->to_str();
		return result;
	}
	case ALOE_TYPE_ARRAY:
	{
		return "array[" + (arr_size == -1 ? "" : std::to_string(arr_size)) + "] of " + arr_element_type->to_str();
	}
	default:
		return "unknown";
	}

	
}

