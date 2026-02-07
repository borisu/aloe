#pragma once

namespace aloe
{
	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(ast_ptr_t ast) override;
	};

}


