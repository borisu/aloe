#pragma once
namespace aloe
{
	class tests_cmd_t
	{
	public:
		bool run_tests();
	private:
		
		void test_var_declarations1();
		void test_fun_declarations1();
		void test_expressions1();
		void test_fun_expect1();
		void test_defaults();
		void test_return_type_mismatch();
		void test_funcall_type_mismatch();
		void test_var_scope();
		
		bool parse_string(const char* al);


		bool success;
		
	};

}
 


