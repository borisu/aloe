#pragma once
#include <string>
#include "aloe\ast.h"


using namespace std;

namespace aloe
{
	class compiler_t
	{
	public:

		virtual bool compile(ast_ptr_t ast) = 0;

	};

	typedef shared_ptr<compiler_t> compiler_ptr_t;

	compiler_ptr_t create_compiler();

	
}
