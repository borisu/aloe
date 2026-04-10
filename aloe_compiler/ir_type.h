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
		IRT_CHAR,
		IRT_FUNCTION
	};

	struct ir_base_type_t
	{
		ir_base_type_t() : type_id(IRT_UNKNOWN), ir_type(nullptr), di_type(nullptr) {}

		ir_base_type_t(ir_type_e irtt) : type_id(irtt), ir_type(nullptr), di_type(nullptr) {}

		ir_type_e type_id;

		Type* ir_type;

		DIType* di_type;

		node_ptr_t ast_def;
	};

	typedef shared_ptr<ir_base_type_t>
	ir_base_type_ptr_t;

	struct ir_type_t
	{
		ir_type_t() :  
			ref_count(0), 
			ir_type(nullptr), 
			di_type(nullptr), 
			base_type(new ir_base_type_t){}

		size_t ref_count;

		Type* ir_type;

		DIType* di_type;

		ir_base_type_ptr_t base_type;

	};

	typedef shared_ptr<ir_type_t>
	ir_type_ptr_t;

}

