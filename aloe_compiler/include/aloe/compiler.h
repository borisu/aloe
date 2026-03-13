#pragma once
#include <string>
#include "aloe\ast.h"


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
			ast_ptr_t ast,
			string& out) = 0;

	
	};

	typedef shared_ptr<compiler_t> compiler_ptr_t;

	compiler_ptr_t create_compiler();

	
}
