#pragma once
#pragma once
#include <string>
#include <map>
#include <vector>
#include "fwd.h"
#include "node.h"
#include "identifier.h"

using namespace std;

namespace aloe
{
	struct gt_node_t : public node_t
	{
		gt_node_t() :node_t(GT_NODE) {};

		type_proxy_t_ptr p_gt_type;

		type_node_t_ptr gt_type() const;

		identifier_node_t_ptr idt;

		void gt_type(type_proxy_t_ptr p_gt_type);
	};
	defptr(gt_node_t);

	typedef vector<gt_node_t_ptr> gt_vec_t;
	defptr(gt_vec_t);

	typedef map<type_node_t, gt_node_t_ptr>	gt_map_t;
	defptr(gt_map_t);

	struct gt_set_t
	{
		gt_vec_t v;
	};
	defptr(gt_set_t);

}
