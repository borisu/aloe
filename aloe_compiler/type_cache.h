#pragma once
#include "aloe\type.h"
#include "ir_type.h"
#include "ir_value.h"
#include "ir_fun.h"

namespace aloe
{
	class type_cache_t
	{
	public:

		DIType* get_dit_type(size_t ref_count, Type* ir_type, DIType* dit);

		//ir_type_ptr_t get_ir_type(type_node_ptr_t node, ir_type_ptr_t ir_type);
	
	private:

		map<type_node_t, ir_type_ptr_t> type_cache;

		typedef std::pair<size_t, Type*> di_cache_key_t;
		map<di_cache_key_t, DIType*> di_cache;

	};


}


