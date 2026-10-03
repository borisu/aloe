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

		virtual bool parse_from_stream(istream& is, ast_t_ptr& ast, const string &source_id, node_type_e root_grammar = PROG_NODE) override;

		virtual void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
			size_t line, size_t charPositionInLine, const std::string& msg,
			std::exception_ptr e) override;

	protected:

		virtual prog_node_t_ptr walk_prog(environment_t_ptr env,  aloeParser::ProgContext* ctx);

		virtual type_proxy_t_ptr walk_type(environment_t_ptr env,  aloeParser::TypeContext* ctx);

		virtual type_proxy_t_ptr walk_fun_type(environment_t_ptr env, aloeParser::FunTypeContext* ctx);

		virtual var_set_t_ptr walk_var_list(environment_t_ptr env, aloeParser::VarListContext* ctx);

		virtual var_node_t_ptr walk_var(environment_t_ptr env, aloeParser::VarDeclarationContext* ctx);

		virtual fun_node_t_ptr walk_fun_declaration(environment_t_ptr env,  aloeParser::FunDeclarationContext* ctx);

		virtual void walk_expectation(environment_t_ptr env, aloeParser::ExpectationContext* ctx);

		virtual void walk_fun_expectation(environment_t_ptr env, aloeParser::ExpectFunContext* ctx);

		virtual void walk_layout_expectation(environment_t_ptr env, aloeParser::ExpectLayoutContext* ctx);

		virtual layout_node_t_ptr walk_layout_declaration(environment_t_ptr env, aloeParser::LayoutDeclarationContext* ctx);

		virtual gt_set_t_ptr walk_gt_chain_node(environment_t_ptr env, aloeParser::GtChainContext* ctx);

		virtual gt_t_ptr walk_gt_chain_member(environment_t_ptr env, aloeParser::GtMemberContext* ctx);

		virtual var_set_t_ptr walk_layout_member_list(environment_t_ptr env, aloeParser::LayoutFieldsListContext* ctx);

		virtual var_t_ptr walk_layout_member(environment_t_ptr env, aloeParser::LayoutFieldContext* ctx);

		virtual expr_node_t_ptr walk_expression(environment_t_ptr env, aloeParser::ExpressionContext* ctx);

		virtual return_node_t_ptr  walk_return(environment_t_ptr env, aloeParser::ReturnStatementContext* ctx);

		virtual literal_node_t_ptr  walk_literal(environment_t_ptr env, aloeParser::LiteralContext* ctx);

		virtual identifier_node_t_ptr  walk_identifier(environment_t_ptr env, aloeParser::IdentifierContext* ctx);

		virtual arglist_node_t_ptr walk_arg_list(environment_t_ptr env, aloeParser::ArgumentExpressionListContext* ctx);

        virtual void check_expr_type_equality(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr1, expr_node_t_ptr expr2, const char* op_str);

		virtual void check_type_equality(environment_t_ptr env, antlr4::ParserRuleContext* ctx, aloe_type_t_ptr type1, aloe_type_t_ptr  type2);

		virtual void check_binary_arithmetic(environment_t_ptr env, aloeParser::ExpressionContext* ctx, binary_expr_node_t_ptr  expr_node, const char* op_str);

		virtual void check_unary_arithmetic(environment_t_ptr env, aloeParser::ExpressionContext* ctx, unary_expr_node_t_ptr  expr_node, const char* op_str);

		virtual void check_is_lvalue(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr_node, const char* op_str);

		virtual void check_assignment(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr lhs, expr_node_t_ptr rhs);

		virtual void check_is_pointer(environment_t_ptr env, aloeParser::ExpressionContext* ctx, expr_node_t_ptr expr_node, const char* op_str);
		
		bool syntax_error_occurred;

	};

}