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

		virtual bool parse_from_file(const string& file_name) override;

		virtual bool parse_from_string(const string& str) override;

		virtual bool parse_from_stream(istream& is) override;

		virtual void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
			size_t line, size_t charPositionInLine, const std::string& msg,
			std::exception_ptr e) override;

	protected:

		virtual node_ptr_t find_type_definition(node_ptr_t parent, const string& name);

		virtual bool walk_global_statement(ns_node_ptr_t parent, aloeParser::StatementContext *ctx);

		virtual object_node_ptr_t walk_object_declaration(node_ptr_t parent, aloeParser::ObjectDeclarationContext* ctx);

		virtual bool walk_chain_declaration(object_node_ptr_t obj, aloeParser::InheritanceChainContext* ctx);

		virtual type_ptr_t walk_type(node_ptr_t parent, aloeParser::TypeContext* ctx, int ref_count = 0);

		virtual fun_node_ptr_t walk_function_decalaration(ns_node_ptr_t parent, aloeParser::FunDeclarationContext* ctx);
		

	};

}