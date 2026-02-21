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
		void parse_string_tests();

		
		bool parse_string(const char* al);
		bool parse_file(const char* al);
		
	};

}
 


