#pragma once
#include "ast/ast.h"

using namespace std;

namespace aloe
{
	class parser_t
	{
	public:

		virtual bool parse_from_stream(istream& is, ast_t_ptr& ast, const string& source_id, node_type_e root_grammmar = PROG_NODE) = 0;
		
	};

	typedef shared_ptr<parser_t> parser_t_ptr;

}


