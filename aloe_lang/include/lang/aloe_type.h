#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cassert> 
#include <map>
#include "base/defs.h"
#include "lang/context.h"

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
		ALOE_TYPE_ARRAY,
		ALOE_TYPE_LAYOUT
	};

	inline bool is_arithmetic(aloe_type_e t)
	{
		switch (t)
		{
		case ALOE_TYPE_CHAR:
		case ALOE_TYPE_INT:
		case ALOE_TYPE_DOUBLE:
			return true;
		default:
			return false;
		}
	}

	struct aloe_type_t;

	typedef
	shared_ptr<aloe_type_t> aloe_type_ptr_t;

	typedef proxy_t<aloe_type_ptr_t>
	type_proxy_t;

	typedef
	shared_ptr<type_proxy_t> type_proxy_ptr_t;
	
	struct var_t : public context_t
	{
		string name;

		type_proxy_ptr_t type;
	};

	typedef
	shared_ptr<var_t> var_ptr_t;

	typedef vector<var_ptr_t>
	var_vec_t;

	typedef map<string, var_ptr_t>
	var_map_t;

	struct var_set_t : public context_t
	{
		var_vec_t v;
		var_map_t m;
	};

	typedef shared_ptr<var_set_t>
	var_set_ptr_t;

	struct gt_t;

	typedef shared_ptr<gt_t>
	gt_ptr_t;

	struct gt_t
	{
		type_proxy_ptr_t type;
	};

	typedef vector<gt_ptr_t>
	gt_vec_t;

	typedef map<aloe_type_t, gt_ptr_t>
	gt_map_t;

	struct gt_set_t : public context_t
	{
		gt_vec_t v;
		gt_map_t m;
	};

	typedef shared_ptr<gt_set_t>
	gt_set_ptr_t;

	struct layout_info_t
	{
		string name;

		int id;

		gt_set_ptr_t gt_chain;

		var_set_ptr_t fields;

		bool is_incomplete;
	};

	typedef shared_ptr<layout_info_t>	
	layout_info_ptr_t;

	struct array_info_t
	{
		array_info_t():size(-1)
		{

		}

		type_proxy_ptr_t elem_type;

		int size; // -1 for unsized arrays
	};

	typedef shared_ptr<array_info_t>
	array_info_ptr_t;

	struct ptr_info_t
	{
		type_proxy_ptr_t pointee_type;
	};

	typedef shared_ptr<ptr_info_t>
	ptr_info_ptr_t;

	struct fun_info_t
	{

		type_proxy_ptr_t ret_type;

		var_set_ptr_t params;

	};

	typedef shared_ptr<fun_info_t>
	fun_info_ptr_t;


	struct aloe_type_t : public context_t
	{
		aloe_type_t(aloe_type_e cat_id) : type_id(cat_id)
		{

		}

		aloe_type_e type_id;

		layout_info_ptr_t lay;

		array_info_ptr_t arr;

		ptr_info_ptr_t ptr;

		fun_info_ptr_t fun;
	
		virtual ~aloe_type_t() {};

		string to_str();
	};

	bool operator==(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator!=(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator<(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator == (const var_t& v1, const var_t& v2);

	bool operator != (const var_t& v1, const var_t& v2);

	bool operator < (const var_t & v1, const var_t & v2);
	
}