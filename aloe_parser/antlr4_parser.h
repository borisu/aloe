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
	protected:

		virtual void release_node(node_t* node);

		ast_t* ast_;

		stack<node_t*> s_;

		virtual void enterObjectDeclaration(aloeParser::ObjectDeclarationContext* /*ctx*/) override;
		virtual void exitObjectDeclaration(aloeParser::ObjectDeclarationContext* /*ctx*/) override;

		virtual void enterInheritanceChain(aloeParser::InheritanceChainContext* /*ctx*/) override;
		virtual void exitInheritanceChain(aloeParser::InheritanceChainContext* /*ctx*/) override;

		virtual void enterIdentifier(aloeParser::IdentifierContext* /*ctx*/) override;
		virtual void exitIdentifier(aloeParser::IdentifierContext* /*ctx*/) override;

		virtual void enterInheritedVirtualType(aloeParser::InheritedVirtualTypeContext* /*ctx*/) override;
		virtual void exitInheritedVirtualType(aloeParser::InheritedVirtualTypeContext* /*ctx*/) override;

	};

}