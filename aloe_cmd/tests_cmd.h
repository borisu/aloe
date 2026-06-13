#pragma once
namespace aloe
{
	class tests_cmd_t
	{
	public:
		bool run_tests();

		void set_validate(bool validate);

		void set_dump_ir(bool dump_ir);

		void set_compile(bool compile);

	private:
		
		void test_var_declarations1();
		void test_fun_declarations1();
		void test_expressions1();
		void test_fun_expect1();
		void test_defaults();
		void test_return_type_mismatch();
		void test_funcall_type_mismatch();
		void test_var_scope();
		void test_recursion();
		
		
		void run_test(const char* test_name, const char* al, bool expected);
		
		bool success;

		bool validate = true;

		bool dump_ir = false;

		bool compile = true;

		
	};

}
 


