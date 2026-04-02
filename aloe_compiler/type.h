#pragma once
#include <string>
#include <memory>
#include <aloe/ast.h>

using namespace std;
using namespace llvm;

namespace aloe
{

	struct type_t
	{
		type_t() : irt(nullptr), dit(nullptr), ref_count(0){}

		size_t ref_count;

		Type* irt;

		DIType* dit;

		node_ptr_t def;
		
	};

	typedef shared_ptr<type_t>
	type_ptr_t;

}

