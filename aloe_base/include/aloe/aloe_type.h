#pragma once
#include <memory>
#include <vector>
#include "aloe/defs.h"

using namespace std;

namespace aloe
{
	enum type_category_e
	{
		TT_UNKNOWN,
		TT_BUILTIN,
		TT_FUNCTION
	};

	struct aloe_type_t
	{
		aloe_type_t(type_category_e cat_id) :ref_count(0), type_cat_id(cat_id) {}
		size_t ref_count;
		type_category_e type_cat_id;

		virtual ~aloe_type_t() {};
	};

	typedef 
	shared_ptr<aloe_type_t> aloe_type_ptr_t;

	enum builtin_type_e
	{
		BIT_UNKNOWN,
		BIT_INT,
		BIT_VOID,
		BIT_DOUBLE,
		BIT_CHAR,
	};

	struct aloe_builtin_type_t : public aloe_type_t
	{
		aloe_builtin_type_t() :aloe_type_t(TT_BUILTIN), bit_type_id(BIT_UNKNOWN) {}

		aloe_builtin_type_t(builtin_type_e type_type, size_t ref_count = 0) :aloe_type_t(TT_BUILTIN), bit_type_id(type_type)
		{
			this->ref_count = ref_count;
			this->type_cat_id = TT_BUILTIN;
		}

		builtin_type_e bit_type_id;
	};

	typedef vector<aloe_type_ptr_t> aloe_type_vector_t;

	struct aloe_fun_type_t : public aloe_type_t
	{
		aloe_fun_type_t() :aloe_type_t(TT_FUNCTION) {}

		aloe_type_ptr_t ret_type;
		
		aloe_type_vector_t args_type_list;
	};

	
	bool operator==(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator!=(const aloe_type_t& t1, const aloe_type_t& t2);


	
}