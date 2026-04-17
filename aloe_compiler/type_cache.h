#pragma once
#include "aloe\type.h"
#include "ir_type.h"
#include "ir_value.h"


namespace aloe
{
	class type_cache_t
	{
	public:

		DIType* get_dit_type(size_t ref_count, Type* ir_type, DIType* dit);

	
	private:

		map<type_node_t, ir_type_ptr_t> type_cache;

		typedef std::pair<size_t, Type*> di_cache_key_t;
		map<di_cache_key_t, DIType*> di_cache;

	};


}


