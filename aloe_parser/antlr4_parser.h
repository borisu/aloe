#pragma once
#include <stack>
#include "aloe/aloe-antlr4.h"
#include "aloe/parser.h"
#include "environment.h"

using namespace std;

namespace aloe
{
	struct parse_exeption_t : public std::exception	{ 
	
		parse_exeption_t(const char* format, ...);

		char buffer[ALOE_MAX_LOG_LEN];

		virtual const char* what() const noexcept override {
			return buffer;
		}
	};

	class antl4_parser_t : public parser_t, public antlr4::BaseErrorListener, private aloeBaseListener
	{
	public:

		antl4_parser_t();

		virtual bool parse_from_stream(istream& is, ast_ptr_t& ast, const string &source_id) override;

		virtual void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
			size_t line, size_t charPositionInLine, const std::string& msg,
			std::exception_ptr e) override;

	protected:

		virtual prog_node_ptr_t walk_prog(environment_ptr_t env,  aloeParser::ProgContext* ctx);

		virtual type_node_ptr_t walk_type(environment_ptr_t env,  aloeParser::TypeContext* ctx, int ref_count = 0);

		virtual base_type_ptr_t walk_base_type(environment_ptr_t env, aloeParser::BaseTypeContext* ctx);

		virtual var_list_node_ptr_t walk_var_list(environment_ptr_t env,  aloeParser::VarListContext* ctx);

		virtual var_node_ptr_t walk_var(environment_ptr_t env, aloeParser::VarDeclarationContext* ctx);

		virtual fun_node_ptr_t walk_func_declaration(environment_ptr_t env,  aloeParser::FunDeclarationContext* ctx);

		virtual expr_node_ptr_t walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx);

		virtual literal_node_ptr_t  walk_literal(environment_ptr_t env, aloeParser::LiteralContext* ctx);

		virtual identifier_node_ptr_t  walk_identifier(environment_ptr_t env, aloeParser::IdentifierContext* ctx, bool declaration, identifier_type_e expected_type);

		virtual arglist_node_ptr_t walk_arg_list(environment_ptr_t env, aloeParser::ArgumentExpressionListContext* ctx);

		virtual execution_block_node_ptr_t walk_execution_block(environment_ptr_t env, aloeParser::ExecutionBlockContext* ctx);

		virtual builtin_node_ptr_t walk_built_in_type(environment_ptr_t env, aloeParser::BuiltinTypeContext* ctx);

		virtual fun_type_ptr_t walk_fun_type(environment_ptr_t env, aloeParser::FunTypeContext* ctx);

		builtin_node_ptr_t INT_NODE;

		builtin_node_ptr_t CHAR_NODE;

		builtin_node_ptr_t DOUBLE_NODE;

		builtin_node_ptr_t OPAQUE_NODE;

		builtin_node_ptr_t VOID_NODE;

		bool syntax_error_occurred;

	};

}