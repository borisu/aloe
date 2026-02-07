#pragma once
#include "ast.h"

using namespace std;

namespace aloe
{
	class parser_t
	{
	public:

		virtual bool parse_from_file(const string &file_name, ast_ptr_t &ast) = 0;

		virtual bool parse_from_stream(istream& is, ast_ptr_t& ast) = 0;

		virtual bool parse_from_string(const string& str, ast_ptr_t& ast) = 0;
		
	};

	typedef shared_ptr<parser_t> parser_ptr_t;

	parser_ptr_t create_parser();

}


