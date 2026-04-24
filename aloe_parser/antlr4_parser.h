#pragma once
#include <stack>
#include "aloe/aloe-antlr4.h"
#include "aloe/parser.h"
#include "environment.h"

using namespace std;

namespace aloe
{
	
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

		virtual type_node_ptr_t walk_base_type(environment_ptr_t env, aloeParser::BaseTypeContext* ctx);

		virtual var_list_node_ptr_t walk_var_list(environment_ptr_t env,  aloeParser::VarListContext* ctx);

		virtual var_node_ptr_t walk_var(environment_ptr_t env, aloeParser::VarDeclarationContext* ctx);

		virtual fun_node_ptr_t walk_fun_declaration(environment_ptr_t env,  aloeParser::FunDeclarationContext* ctx);

		virtual expr_node_ptr_t walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx);

		virtual return_node_ptr_t  walk_return(environment_ptr_t env, aloeParser::ReturnStatementContext* ctx);

		virtual literal_node_ptr_t  walk_literal(environment_ptr_t env, aloeParser::LiteralContext* ctx);

		virtual identifier_node_ptr_t  walk_identifier(environment_ptr_t env, aloeParser::IdentifierContext* ctx, identifier_type_e expected_type, bool must_exist);

		virtual arglist_node_ptr_t walk_arg_list(environment_ptr_t env, aloeParser::ArgumentExpressionListContext* ctx);

		virtual exec_block_node_ptr_t walk_execution_block(environment_ptr_t env, aloeParser::ExecutionBlockContext* ctx);

		virtual type_node_ptr_t walk_built_in_type(environment_ptr_t env, aloeParser::BuiltinTypeContext* ctx);

		virtual fun_type_node_ptr_t walk_fun_type(environment_ptr_t env, aloeParser::FunTypeContext* ctx);

        virtual void check_type_equality(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr1, expr_node_ptr_t expr2, const char* op_str);

		virtual void check_binary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, binary_expr_node_ptr_t  expr_node, const char* op_str);

		virtual void check_unary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, unary_expr_node_ptr_t  expr_node, const char* op_str);

		virtual void check_lvalue(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str);

		virtual void check_assignment(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t lhs, expr_node_ptr_t rhs);
		
		bool syntax_error_occurred;

	};

}