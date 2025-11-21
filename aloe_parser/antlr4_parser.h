#pragma once
#include "aloe/aloe-antlr4.h"
#include "aloe/parser.h"
#include <stack>

using namespace std;

namespace aloe
{
	class antl4_parser_t : public parser_t, public antlr4::BaseErrorListener, private aloeBaseListener
	{
	public:

		virtual bool parse_from_file(const string& file_name, ast_t** ast) override;

		virtual bool parse_from_stream(istream& is, ast_t** ast) override;

		virtual bool parse_from_string(const string& str, ast_t** ast) override;

		virtual void release_ast(ast_t* ast) override;

		virtual void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
			size_t line, size_t charPositionInLine, const std::string& msg,
			std::exception_ptr e) override;
	private:

		enum PARSE_STATE
		{
			GLOBAL_SCOPE,
			NAMESPACE_SCOPE,
			OBJECT_SCOPE
		};

		struct ctx_t
		{
			ctx_t(PARSE_STATE state, ctx_t* prev = nullptr);

			PARSE_STATE state;

			string fqn;

			unit_t* scope_unit;

			ctx_t* prev;

			union
			{
				object_unit_t *obj;
			}u;
		};
	
		ast_t* ast_;

		node_t* curr_node_;

		ctx_t*	curr_ctx_;

		virtual void enterObjectDeclaration(aloeParser::ObjectDeclarationContext* /*ctx*/) override;

		virtual void exitObjectDeclaration(aloeParser::ObjectDeclarationContext* /*ctx*/) override;

	};

}