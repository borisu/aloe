#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cassert> 
#include <map>
#include "base/defs.h"
#include "lang/context.h"
#include "lang/ast/node.h"

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
	defptr(aloe_type_t);

	typedef proxy_t<aloe_type_t_ptr> type_proxy_t;
	defptr(type_proxy_t);
	
	struct var_t : 
		public context_t, 
		public node_ref_t
	{
		string name;

		type_proxy_t_ptr p_var_type;

		aloe_type_t_ptr var_type() const
		{
			return p_var_type->target;
		}

		void var_type(type_proxy_t_ptr type)
		{
			this->p_var_type = type;
		}
	};
	defptr(var_t);
	
	typedef vector<var_t_ptr> var_vec_t;
	defptr(var_vec_t);

	typedef map<string, var_t_ptr>	var_map_t;
	defptr(var_map_t);

	struct var_set_t : public context_t
	{
		var_vec_t v;
		var_map_t m;
	};
	defptr(var_set_t);
	
	struct gt_t
	{
		type_proxy_t_ptr p_gt_type;

		aloe_type_t_ptr gt_type() const
		{
			return p_gt_type->target;
		}

		void gt_type(type_proxy_t_ptr p_gt_type)
		{	
			this->p_gt_type = p_gt_type;
		}

	};
	defptr(gt_t);

	typedef vector<gt_t_ptr> gt_vec_t;
	defptr(gt_vec_t);

	typedef map<aloe_type_t, gt_t_ptr>	gt_map_t;
	defptr(gt_map_t);

	struct gt_set_t : public context_t
	{
		gt_vec_t v;
		gt_map_t m;
	};
	defptr(gt_set_t);

	struct layout_info_t 
	{
		layout_info_t() : id(-1), 
			gt_chain(newptr(gt_set_t)), 
			fields(newptr(var_set_t))
		{
		}
		string name;

		int id;

		gt_set_t_ptr gt_chain;

		var_set_t_ptr fields;

	};
	defptr(layout_info_t);

	struct array_info_t
	{
		array_info_t():size(-1)
		{

		}

		type_proxy_t_ptr p_elem_type;

		void elem_type(type_proxy_t_ptr p_elem_type)
		{
			this->p_elem_type = p_elem_type;
		}

		aloe_type_t_ptr elem_type() const
		{
			return p_elem_type->target;
		}

		int size; // -1 for unsized arrays
	};
	defptr(array_info_t);

	struct ptr_info_t
	{
		type_proxy_t_ptr p_ptr_type;

		void pointee_type(type_proxy_t_ptr p_ptr_type)
		{
			this->p_ptr_type = p_ptr_type;
		}

		aloe_type_t_ptr pointee_type() const
		{
			return p_ptr_type->target;
		}
	};
	defptr(ptr_info_t);

	struct fun_info_t 
	{
		type_proxy_t_ptr p_ret_type;

		aloe_type_t_ptr ret_type() const
		{
			return p_ret_type->target;
		}

		void ret_type(type_proxy_t_ptr p_ret_type)
		{
			this->p_ret_type = p_ret_type;
		}

		var_set_t_ptr params;

		string name;

	};
	defptr(fun_info_t);

	struct aloe_type_t : public context_t
	{
		aloe_type_t(aloe_type_e cat_id) : 
			type_id(cat_id), 
			is_incomplete(false),
			layout (newptr(layout_info_t)),
			arr (newptr(array_info_t)),
			ptr (newptr(ptr_info_t)),
			fun (newptr(fun_info_t))
		{
		
		}

		aloe_type_e type_id;

		layout_info_t_ptr layout;

		array_info_t_ptr arr;

		ptr_info_t_ptr ptr;

		fun_info_t_ptr fun;

		bool is_incomplete;
	
		virtual ~aloe_type_t() {};

		string to_str();

		string name();
	};
	

	bool operator==(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator!=(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator<(const aloe_type_t& t1, const aloe_type_t& t2);

	bool operator == (const var_t& v1, const var_t& v2);

	bool operator != (const var_t& v1, const var_t& v2);

	bool operator < (const var_t & v1, const var_t & v2);
	
}