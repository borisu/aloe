#pragma once
#include <map>
#include "lang/ast/type.h"
#include "value.h"

using namespace llvm;
using namespace std;

namespace aloe
{
	class di_cache_t
	{
	public:

		di_cache_t(DIBuilder& dib);

		DIType* get_dit_type(type_node_t_ptr atype);

	private:

		map<type_node_t, DIType*> di_cache;

		DIBuilder& di_builder;

	};
	defptr(di_cache_t);
}


