#pragma once
#include <stack>
#include "aloe/aloe-antlr4.h"
#include "aloe/parser.h"

using namespace std;

namespace aloe
{
	struct parse_exeption_t : public std::exception	{ };

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

		virtual scope_node_ptr_t find_my_scope_node(node_ptr_t first);

		virtual node_ptr_t find_type_definition_by_name(node_ptr_t first, const string& name);

		virtual prog_node_ptr_t walk_prog(ast_ptr_t ast, aloeParser::ProgContext* ctx);

		//virtual func_call_ptr_t walk_fun_call(node_ptr_t parent, aloeParser::ExecutionStatementContext* ctx);
		
		virtual object_node_ptr_t walk_object_declaration(node_ptr_t parent, aloeParser::ObjectDeclarationContext* ctx);

		virtual inh_chain_node_ptr_t walk_chain_declaration(node_ptr_t parent, aloeParser::InheritanceChainContext* ctx);

		virtual type_node_ptr_t walk_type(node_ptr_t parent, aloeParser::TypeContext* ctx, int ref_count = 0);

		virtual var_list_node_ptr_t walk_var_list(node_ptr_t parent, aloeParser::VarListContext* ctx);

		virtual var_node_ptr_t walk_var(node_ptr_t parent, aloeParser::VarDeclarationContext* ctx);

		virtual fun_node_ptr_t walk_function_decalaration(node_ptr_t parent, aloeParser::FunDeclarationContext* ctx);
		

	};

}