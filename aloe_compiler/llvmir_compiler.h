#pragma once

namespace aloe
{
	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(const string& module_name, ast_ptr_t ast, object_type_e obj_type, string& out) override;
	};

}


