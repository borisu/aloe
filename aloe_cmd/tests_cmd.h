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
		void test_defauts();

		
		bool parse_string(const char* al);


		bool success;
		
	};

}
 


