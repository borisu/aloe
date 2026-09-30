#include "pch.h"
#include "base/defs.h"
#include "lang/aloe_type.h"

using namespace aloe;

bool
aloe::operator == (const aloe_type_t& t1, const aloe_type_t& t2)  
{
	return  !(t1 < t2) && !(t2 < t1);
}

bool
aloe::operator != (const aloe_type_t& t1, const aloe_type_t& t2)
{
	return  !(t1 == t2);
}

bool
aloe::operator == (const var_t& v1, const var_t& v2)
{
	return v1.name == v2.name && *v1.type->target == *v2.type->target;
}

bool 
aloe::operator != (const var_t& v1, const var_t& v2)
{
	return !(v1 == v2);
}

bool 
aloe::operator < (const var_t& v1, const var_t& v2)
{
	if (v1.name != v2.name)
	{
		return v1.name < v2.name;
	}
	return *v1.type->target < *v2.type->target;
}

bool aloe::operator < (const aloe_type_t& t1, const aloe_type_t& t2)
{
	if (t1.type_id != t2.type_id)
	{
		return t1.type_id < t2.type_id;
	}

	switch (t1.type_id)
	{	
	case ALOE_TYPE_ARRAY:
	{
		if (t1.arr->size != t2.arr->size)
		{
			return t1.arr->size < t2.arr->size;
		}

		if (*t1.arr->elem_type->target != *t2.arr->elem_type->target)
		{
			return *t1.arr->elem_type->target < *t2.arr->elem_type->target;
		}

		break;

	}
	case ALOE_TYPE_PTR:
	{
		if (*t1.ptr->pointee_type->target != *t2.ptr->pointee_type->target)
		{
			return *t1.ptr->pointee_type->target < *t2.ptr->pointee_type->target;
		}
		break;
	}
	case ALOE_TYPE_FUNCTION:
	{
		if (*t1.fun->ret_type->target != *t2.fun->ret_type->target)
		{
			return *t1.fun->ret_type->target < *t2.fun->ret_type->target;
		}

		if (t1.fun->params->v.size() != t2.fun->params->v.size())
		{
			return t1.fun->params->v.size() < t2.fun->params->v.size();
		}

		for (size_t i = 0; i < t1.fun->params->v.size(); i++)
		{
			auto param1 = t1.fun->params->v[i];
			auto param2 = t2.fun->params->v[i];

			if (*param1 != *param2)
			{
				return *param1 < *param2;
			}
		}
		break;

	}
	case ALOE_TYPE_LAYOUT:
	{
		if (t1.lay->id != t2.lay->id)
		{
			return t1.lay->id < t2.lay->id;
		}
		
		break;

	}
	case ALOE_TYPE_CHAR:
	case ALOE_TYPE_VOID:
	case ALOE_TYPE_DOUBLE:
	case ALOE_TYPE_INT:
	default:
	{
		break;
	}
	};

	return false;
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
		for (size_t i = 0; i < fun->params->v.size(); i++)
		{
			if (i > 0)
			{
				result += ", ";
			}
			result += fun->params->v[i]->name + ": " + fun->params->v[i]->type->target->to_str();
		}
		result += ") -> ";
		result += fun->ret_type->target->to_str();
		return result;
	}
	case ALOE_TYPE_PTR:
	{
		return  "^" + ptr->pointee_type->target->to_str();
	}
	case ALOE_TYPE_ARRAY:
	{
		return  arr->elem_type->target->to_str() + "[" + (arr->size == -1 ? "" : std::to_string(arr->size	)) + "]";

	}
	case ALOE_TYPE_LAYOUT:
	{
		string s = "layout " + lay->name + "{";
		for (auto& m : lay->fields->v)
		{
			s += m->name + ":" + m->type->target->to_str() + "; ";
		}
		s += "}";
		return s;
	}
	default:
		return "unknown";
	}

	
}

