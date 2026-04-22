#pragma once
#include <memory>
#include <vector>
#include <string>
#include "aloe/defs.h"

using namespace std;

namespace aloe
{
	enum type_category_e
	{
		TT_UNKNOWN,
		TT_VOID,
		TT_CHAR,
		TT_INT,
		TT_FLOAT,
		TT_FUNCTION,
	};

	struct aloe_type_t
	{
		aloe_type_t(type_category_e cat_id, size_t ref_count = 0) :ref_count(ref_count), type_cat_id(cat_id) 
		{
		}

		size_t ref_count;

		type_category_e type_cat_id;

		virtual ~aloe_type_t() {};

		string to_str();
	};

	typedef 
	shared_ptr<aloe_type_t> aloe_type_ptr_t;


	struct aloe_void_type_t : public aloe_type_t
	{
		aloe_void_type_t(size_t ref_count = 0) :aloe_type_t(TT_VOID, ref_count)
		{
		}
	};

	struct aloe_char_type_t : public aloe_type_t
	{
		aloe_char_type_t(size_t ref_count = 0) :aloe_type_t(TT_CHAR, ref_count)
		{
		}
	};

	struct aloe_int_type_t : public aloe_type_t
	{
		aloe_int_type_t(size_t ref_count = 0) :aloe_type_t(TT_INT, ref_count)
		{
		}
	};

	struct aloe_float_type_t : public aloe_type_t
	{
		aloe_float_type_t(size_t ref_count = 0) :aloe_type_t(TT_FLOAT)
		{
			
		}
	};

	typedef vector<aloe_type_ptr_t> aloe_type_vector_t;
	struct aloe_fun_type_t : public aloe_type_t
	{
		aloe_fun_type_t() :aloe_type_t(TT_FUNCTION) {}

		aloe_type_ptr_t ret_type;
		
		aloe_type_vector_t args_type_list;
	};

	typedef shared_ptr<aloe_fun_type_t> aloe_fun_type_ptr_t;
	
	bool operator==(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator!=(const aloe_type_t& t1, const aloe_type_t& t2);


	
}