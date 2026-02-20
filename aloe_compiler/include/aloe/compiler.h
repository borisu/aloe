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
			const string &module_name, 
			ast_ptr_t ast, 
			object_type_e obj_type, 
			string &out) = 0;

	};

	typedef shared_ptr<compiler_t> compiler_ptr_t;

	compiler_ptr_t create_compiler();

	
}
