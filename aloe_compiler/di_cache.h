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

		DIType* get_dit_type(aloe_type_t_ptr atype);

	private:

		map<aloe_type_t, DIType*> di_cache;

		DIBuilder& di_builder;

	};
	defptr(di_cache_t);
}


