#pragma once
#include "lang/parser.h"
#include "antlr4/aloe-antlr4.h"
#include "environment.h"

using namespace std;

namespace aloe
{
	
	class antl4_parser_t : public parser_t, public antlr4::BaseErrorListener, private aloeBaseListener
	{
	public:

		antl4_parser_t();

		virtual bool parse_from_stream(istream& is, ast_ptr_t& ast, const string &source_id, node_type_e root_grammar = PROG_NODE) override;

		virtual void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
			size_t line, size_t charPositionInLine, const std::string& msg,
			std::exception_ptr e) override;

	protected:

		virtual prog_node_ptr_t walk_prog(environment_ptr_t env,  aloeParser::ProgContext* ctx);

		virtual type_node_ptr_t walk_type(environment_ptr_t env,  aloeParser::TypeContext* ctx);

		virtual type_node_ptr_t walk_fun_type(environment_ptr_t env, aloeParser::FunTypeContext* ctx);

		virtual var_set_ptr_t walk_var_list(environment_ptr_t env, aloeParser::VarListContext* ctx);

		virtual var_node_ptr_t walk_var(environment_ptr_t env, aloeParser::VarDeclarationContext* ctx);

		virtual fun_node_ptr_t walk_fun_declaration(environment_ptr_t env,  aloeParser::FunDeclarationContext* ctx);

		virtual void walk_expectation(environment_ptr_t env, aloeParser::ExpectationContext* ctx);

		virtual void walk_fun_expectation(environment_ptr_t env, aloeParser::ExpectFunContext* ctx);

		virtual void walk_layout_expectation(environment_ptr_t env, aloeParser::ExpectLayoutContext* ctx);

		virtual type_node_ptr_t walk_layout_declaration(environment_ptr_t env, aloeParser::LayoutDeclarationContext* ctx);

		virtual gt_set_ptr_t walk_gt_chain_node(environment_ptr_t env, aloeParser::GtChainContext* ctx);

		virtual gt_ptr_t walk_gt_chain_member(environment_ptr_t env, aloeParser::GtMemberContext* ctx);

		virtual var_set_ptr_t walk_layout_member_list(environment_ptr_t env, aloeParser::LayoutMemberListContext* ctx);

		virtual var_ptr_t walk_layout_member(environment_ptr_t env, aloeParser::LayoutMemberContext* ctx);

		virtual expr_node_ptr_t walk_expression(environment_ptr_t env, aloeParser::ExpressionContext* ctx);

		virtual return_node_ptr_t  walk_return(environment_ptr_t env, aloeParser::ReturnStatementContext* ctx);

		virtual literal_node_ptr_t  walk_literal(environment_ptr_t env, aloeParser::LiteralContext* ctx);

		virtual identifier_node_ptr_t  walk_identifier(environment_ptr_t env, aloeParser::IdentifierContext* ctx);

		virtual arglist_node_ptr_t walk_arg_list(environment_ptr_t env, aloeParser::ArgumentExpressionListContext* ctx);

        virtual void check_expr_type_equality(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr1, expr_node_ptr_t expr2, const char* op_str);

		virtual void check_type_equality(environment_ptr_t env, antlr4::ParserRuleContext* ctx, aloe_type_ptr_t type1, aloe_type_ptr_t  type2);

		virtual void check_binary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, binary_expr_node_ptr_t  expr_node, const char* op_str);

		virtual void check_unary_arithmetic(environment_ptr_t env, aloeParser::ExpressionContext* ctx, unary_expr_node_ptr_t  expr_node, const char* op_str);

		virtual void check_is_lvalue(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str);

		virtual void check_assignment(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t lhs, expr_node_ptr_t rhs);

		virtual void check_is_pointer(environment_ptr_t env, aloeParser::ExpressionContext* ctx, expr_node_ptr_t expr_node, const char* op_str);
		
		bool syntax_error_occurred;

	};

}