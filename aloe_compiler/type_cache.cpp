#include "pch.h"
#include "type_cache.h"
using namespace aloe;




DIType* 
type_cache_t::get_dit_type(size_t ref_count, Type* ir_type, DIType* dit)
{
	auto key = di_cache_key_t(ref_count, ir_type);
	auto ir = di_cache.find(di_cache_key_t(ref_count, ir_type));
	if (ir != di_cache.end())
	{
		return ir->second;
	}

	di_cache[key] = dit;
	return dit;
}

std::optional<DIType*>
type_cache_t::get_dit_type(size_t ref_count, Type* ir_type)
{
	auto key = di_cache_key_t(ref_count, ir_type);
	auto ir = di_cache.find(di_cache_key_t(ref_count, ir_type));
	if (ir != di_cache.end())
	{
		return ir->second;
	}

	return std::nullopt;
}


