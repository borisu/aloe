#pragma once
#include <string>
#include "lang/ast/ast.h"

using namespace std;

namespace aloe
{
	enum object_type_e
	{
		LLVM_IR,
		OBJECT_FILE
	};
	
	class compiler_t
	{	
	public:

		virtual bool compile(
			ast_t_ptr ast,
			ostream& out) = 0;

		virtual void set_validate(bool validate) = 0;

		virtual void set_no_debug(bool no_debug) = 0;
	
	};
	defptr(compiler_t);
	
}
