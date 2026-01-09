#pragma once
#include "ast.h"

using namespace std;

namespace aloe
{
	class parser_t
	{
	public:

		virtual bool parse_from_file(const string &file_name) = 0;

		virtual bool parse_from_stream(istream& is) = 0;

		virtual bool parse_from_string(const string& str) = 0;
		
	};

	parser_t *create_parser();

	void release_parser(parser_t*);

}


