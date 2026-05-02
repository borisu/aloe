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
	if (t1.ref_count != t2.ref_count)
	{
		return false;
	}

	if (t1.type_cat_id != t2.type_cat_id)
	{
		return false;
	}

	switch (t1.type_cat_id)
	{
	case TT_CHAR:
	case TT_VOID:
	case TT_FLOAT:
	case TT_INT:
	{
		return (t1.type_cat_id == t2.type_cat_id);
		break;
	}
	case TT_FUNCTION:
	{
		try {

			auto fun_t1 = dynamic_cast<const aloe_fun_type_t&>(t1);
			auto fun_t2 = dynamic_cast<const aloe_fun_type_t&>(t2);


			if (fun_t1.args_type_list.size() != fun_t2.args_type_list.size())
			{
				return false;
			}

			if (*fun_t1.ret_type != *fun_t2.ret_type)
			{
				return false;
			}

			for (size_t i = 0; i < fun_t1.args_type_list.size(); i++)
			{
				auto arg1 = fun_t1.args_type_list[i];
				auto arg2 = fun_t2.args_type_list[i];
				if (*arg1 != *arg2)
				{
					return false;
				}
			}

		}
		catch (const std::bad_cast&)
		{
			return false;
		}

		return true;

		break;
	}
	default:

		return false;
	}

	return true;
}




std::string
aloe_type_t::to_str()
{
	switch (type_cat_id)
	{
	case TT_CHAR:
		return "char";
	case TT_VOID:
		return "void";
	case TT_FLOAT:
		return "float";
	case TT_INT:
		return "int";
	case TT_FUNCTION:
	{
		auto fun_t = dynamic_cast<const aloe_fun_type_t&>(*this);
		std::string result = "fun(";
		for (size_t i = 0; i < fun_t.args_type_list.size(); i++)
		{
			if (i > 0)
			{
				result += ", ";
			}
			result += fun_t.args_type_list[i]->to_str();
		}
		result += ") -> ";
		result += fun_t.ret_type->to_str();
		return result;
	}
	default:
		return "unknown";
	}

	
}