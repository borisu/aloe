#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cassert> 
#include "aloe/defs.h"

using namespace std;

namespace aloe
{
	enum aloe_type_e
	{
		ALOE_TYPE_UNKNOWN,
		ALOE_TYPE_VOID,
		ALOE_TYPE_CHAR,
		ALOE_TYPE_INT,
		ALOE_TYPE_DOUBLE,
		ALOE_TYPE_PTR,
		ALOE_TYPE_FUNCTION,
		ALOE_TYPE_ARRAY
	};

	inline bool is_arithmetic(aloe_type_e cat)
	{
		switch (cat)
		{
		case ALOE_TYPE_CHAR:
		case ALOE_TYPE_INT:
		case ALOE_TYPE_DOUBLE:
		case ALOE_TYPE_PTR:
		case ALOE_TYPE_ARRAY:
			return true;
		default:
			return false;
		}
	}

	struct aloe_type_t;

	typedef
	shared_ptr<aloe_type_t> aloe_type_ptr_t;

	typedef vector<aloe_type_ptr_t>
	aloe_type_vector_t;
	
	struct aloe_type_t
	{
		aloe_type_t(aloe_type_e cat_id) : type_id(cat_id), arr_size(-1)
		{
		}

		aloe_type_e type_id;

		aloe_type_ptr_t ptr_pointee_type;

		aloe_type_ptr_t arr_element_type;

		int arr_size; // -1 for unsized arrays

		aloe_type_ptr_t fun_ret_type;

		aloe_type_vector_t fun_param_types;

		virtual ~aloe_type_t() {};

		string to_str();
	};

	
	bool operator==(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator!=(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator < (const aloe_type_t & t1, const aloe_type_t & t2);


	
}