#pragma once
#include <string>
#include <memory>
#include <aloe/ast.h>

using namespace std;
using namespace llvm;

namespace aloe
{
	enum type_e
	{
		TYPE_UNKNOWN,
		TYPE_INT,
		TYPE_VOID,
		TYPE_DOUBLE,
		TYPE_OPAQUE,
		TYPE_CHAR,
		TYPE_OBJECT,
		TYPE_FUNCTION
	};

	struct type_t
	{
		type_t() : irt(nullptr), dit(nullptr) {}

		Type* irt;

		DIType* dit;

		node_ptr_t node;
		
	};

	typedef shared_ptr<type_t>
	type_ptr_t;

}

