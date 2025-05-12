#pragma once
#include <string>
#include "ast.h"

using namespace std;

namespace aloe
{
	class parser_t : public antlr4::BaseErrorListener
	{
	public:

		bool parse_from_file(const string &file_name, ast_t **ast_tree);

		virtual void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
			size_t line, size_t charPositionInLine, const std::string& msg,
			std::exception_ptr e) override;

	};

}


