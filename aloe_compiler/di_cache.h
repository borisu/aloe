#pragma once
#include <map>
#include "lang\aloe_type.h"
#include "value.h"

using namespace llvm;
using namespace std;

namespace aloe
{
	class di_cache_t
	{
	public:

		di_cache_t(DIBuilder& dib);

		DIType* get_dit_type(aloe_type_ptr_t type);

	private:

		map<aloe_type_t, DIType*> di_cache;

		DIBuilder& di_builder;

	};

	typedef shared_ptr<di_cache_t> 
	di_cache_ptr_t;

}


