#pragma once
#include <string>
#include <memory>
#include "llvm/IR/Function.h"
#include <aloe/ast.h>

using namespace std;
using namespace llvm;

namespace aloe
{
	struct value_type_t
	{
		value_type_t() : 
			ir_type(nullptr), 
			di_type(nullptr),
			ref_count(0),
			pointee_ir_type(nullptr),
			pointee_di_type(nullptr) {}

		Type* ir_type;

		DIType* di_type;

		size_t ref_count;

		Type* pointee_ir_type;

		DIType* pointee_di_type;
	};

	typedef shared_ptr<value_type_t>
	value_type_ptr_t;

}

