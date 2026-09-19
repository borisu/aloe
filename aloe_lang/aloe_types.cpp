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
		if (t1.arr_size != t2.arr_size)
		{
			return t1.arr_size < t2.arr_size;
		}

		if (*t1.arr_type != *t2.arr_type)
		{
			return *t1.arr_type < *t2.arr_type;
		}

		break;

	}
	case ALOE_TYPE_PTR:
	{
		if (*t1.ptr_type != *t2.ptr_type)
		{
			return *t1.ptr_type < *t2.ptr_type;
		}
		break;
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
		break;

	}
	case ALOE_TYPE_LAYOUT:
	{
		if (t1.lot_name != t2.lot_name)
		{
			return t1.lot_name < t2.lot_name;
		};

		if ((t1.lot_gt != nullptr) != (t2.lot_gt != nullptr))
		{
			return (t1.lot_gt != nullptr) < (t2.lot_gt != nullptr);
		}

		if (t1.lot_gt && (*t1.lot_gt != *t2.lot_gt))
		{
			return *t1.lot_gt < *t2.lot_gt;
		}

		if (t1.lot_members.size() != t2.lot_members.size())
		{
			return t1.lot_members.size() < t2.lot_members.size();
		};

		for (size_t i = 0; i < t1.lot_members.size(); i++)
		{
			auto m1 = t1.lot_members[i];
			auto m2 = t2.lot_members[i];

			if (*m1->type != *m2->type)
			{
				return *m1->type < *m2->type;
			}
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

bool aloe::operator == (const gt_member_t& m1, const gt_member_t& m2)
{
	return  !(m1 < m2) && !(m2 < m1);
}

bool aloe::operator != (const gt_member_t& m1, const gt_member_t& m2)
{
	return  !(m1 == m2);
}

bool aloe::operator < (const gt_member_t & m1, const gt_member_t & m2)
{
	if (m1.type != m2.type)
	{
		return m1.type < m2.type;
	}

	if (m1.is_virtual != m2.is_virtual)
	{
		return m1.is_virtual < m2.is_virtual;
	}

	if ((m1.next != nullptr) != (m2.next != nullptr))
	{
		return (m1.next != nullptr) < (m2.next != nullptr);
	}

	if (m1.next)
	{
		auto next_m1 = m1.next;
		auto next_m2 = m2.next;

		if (*next_m1 != *next_m2)
		{
			return *next_m1 < *next_m2;
		}
	}

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
	case ALOE_TYPE_PTR:
	{
		return  "^" + ptr_type->to_str();
	}
	case ALOE_TYPE_ARRAY:
	{
		return  arr_type->to_str() + "[" + (arr_size == -1 ? "" : std::to_string(arr_size)) + "]";

	}
	case ALOE_TYPE_LAYOUT:
	{
		string s = "layout " + lot_name + "{";
		for (auto& m : lot_members)
		{
			s += m->name + ":" + m->type->to_str() + "; ";
		}
		s += "}";
		return s;
	}
	default:
		return "unknown";
	}

	
}

