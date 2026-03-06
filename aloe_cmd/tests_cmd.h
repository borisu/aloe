#pragma once
namespace aloe
{
	class tests_cmd_t
	{
	public:
		void run_tests();
	private:
		
		void parse_file_tests();
		void compile_file_tests();

		void test_var_declarations1();
		void test_object_declarations1(); 
		void test_fun_declarations1();
		void test_expressions1();
		void test_fun_expect1();


		
		bool parse_string(const char* al);
		bool parse_file(const char* al);
		
	};

}
 


