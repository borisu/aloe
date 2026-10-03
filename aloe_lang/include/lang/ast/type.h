#pragma once
#include <vector>
#include <map>
#include "node.h"
#include "var.h"

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

	typedef vector<var_node_t_ptr> var_vec_t;
	defptr(var_vec_t);

	typedef map<string, var_node_t_ptr>	var_map_t;
	defptr(var_map_t);

	struct var_set_t 
	{
		var_vec_t v;
		var_map_t m;
	};
	defptr(var_set_t);

	struct gt_t
	{
		type_proxy_t_ptr p_gt_type;

		type_node_t_ptr gt_type() const;
		
		void gt_type(type_proxy_t_ptr p_gt_type);

	};
	defptr(gt_t);

	typedef vector<gt_t_ptr> gt_vec_t;
	defptr(gt_vec_t);

	typedef map<type_node_t, gt_t_ptr>	gt_map_t;
	defptr(gt_map_t);

	struct gt_set_t 
	{
		gt_vec_t v;
	};
	defptr(gt_set_t);

	struct layout_info_t
	{
		layout_info_t();

		string name;

		int id;

		gt_set_t_ptr gt_chain;

		var_set_t_ptr fields;

	};
	defptr(layout_info_t);

	struct array_info_t
	{
		array_info_t();

		type_proxy_t_ptr p_elem_type;

		void elem_type(type_proxy_t_ptr p_elem_type);

		type_node_t_ptr elem_type() const;

		int size; // -1 for unsized arrays
	};
	defptr(array_info_t);

	struct ptr_info_t
	{
		type_proxy_t_ptr p_ptr_type;

		void pointee_type(type_proxy_t_ptr p_ptr_type);

		type_node_t_ptr pointee_type() const;
	};
	defptr(ptr_info_t);

	struct fun_info_t
	{
		type_proxy_t_ptr p_ret_type;

		type_node_t_ptr ret_type() const;

		void ret_type(type_proxy_t_ptr p_ret_type);

		var_set_t_ptr params;

		string name;

	};
	defptr(fun_info_t);

	struct type_node_t : public node_t
	{
		type_node_t(aloe_type_e type_id);

		aloe_type_e type_id;

		layout_info_t_ptr layout;

		array_info_t_ptr arr;

		ptr_info_t_ptr ptr;

		fun_info_t_ptr fun;

		bool is_incomplete;

		virtual ~type_node_t() {};

		string to_str();

		string name();
		
	};
	defptr(type_node_t);

	bool operator==(const type_node_t& t1, const type_node_t& t2);

	bool operator!=(const type_node_t& t1, const type_node_t& t2);

	bool operator<(const type_node_t& t1, const type_node_t& t2);

	/*bool operator == (const var_node_t& v1, const var_node_t& v2);

	bool operator != (const var_node_t& v1, const var_node_t& v2);

	bool operator < (const var_node_t& v1, const var_node_t& v2);*/
}