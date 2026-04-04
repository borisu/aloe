#pragma once
#include <string>
#include <memory>
#include "llvm/IR/Function.h"
#include <aloe/ast.h>

using namespace std;
using namespace llvm;

namespace aloe
{
	enum ir_type_e
	{
		IRT_UNKNOWN,
		IRT_OPAQUE,
		IRT_INTEGER,
		IRT_FUNCTION
	};

	struct ir_base_type_t
	{
		ir_base_type_t() : irtt(IRT_UNKNOWN), irt(nullptr), dit(nullptr) {}

		ir_base_type_t(ir_type_e irtt) : irtt(irtt), irt(nullptr), dit(nullptr) {}

		ir_type_e irtt;

		Type* irt;

		DIType* dit;

		node_ptr_t ast_def;
	};

	typedef shared_ptr<ir_base_type_t>
	ir_base_type_ptr_t;

	struct ir_type_t
	{
		ir_type_t() :  ref_count(0), irt(nullptr), dit(nullptr) {}

		size_t ref_count;

		Type* irt;

		DIType* dit;

		ir_base_type_ptr_t bt;

	};

	typedef shared_ptr<ir_type_t>
	ir_type_ptr_t;

}

