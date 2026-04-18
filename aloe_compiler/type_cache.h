#pragma once
#include <optional>
#include "aloe\type.h"
#include "value.h"



namespace aloe
{
	class type_cache_t
	{
	public:

		std::optional<DIType*> get_dit_type(size_t ref_count, Type* ir_type);

		DIType* get_dit_type(size_t ref_count, Type* ir_type, DIType* dit);

	private:

		map<type_node_t, value_type_ptr_t> type_cache;

		typedef std::pair<size_t, Type*> di_cache_key_t;
		map<di_cache_key_t, DIType*> di_cache;

	};


}


