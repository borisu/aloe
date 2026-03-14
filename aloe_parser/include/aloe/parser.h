#pragma once
#include "ast.h"

using namespace std;

namespace aloe
{
	class parser_t
	{
	public:

		virtual bool parse_from_stream(istream& is, ast_ptr_t& ast, const string& source_id) = 0;
		
	};

	typedef shared_ptr<parser_t> parser_ptr_t;

	parser_ptr_t create_parser();

}


