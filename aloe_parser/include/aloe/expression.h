#pragma once
#include <string>
#include <vector>
#include <variant>
#include "node.h"
#include "literal.h"
#include "identifier.h"


using namespace std;

namespace aloe
{
	struct expr_node_t;
	typedef shared_ptr<expr_node_t> expr_node_ptr_t;

	enum expression_op_e
	{
		expr_identifier,
		expr_literal,
		expr_bracketed,
		expr_sfxplusplus,
		expr_sfxminmin,
		expr_funcall,
		expr_index,
		expr_dot,
		expr_arrow,
		expr_preplusplus,
		expr_preminmin,
		expr_plus,
		expr_min,
		expr_not,
		expr_bwsnot,
		expr_cast,
		expr_deref,
		expr_addressof,
		expr_sizeofexpr,
		expr_sizeoftype,
		expr_mult,
		expr_div,
		expr_mod,
		expr_add,
		expr_sub,
		expr_shiftleft,
		expr_shiftright,
		expr_less,
		expr_lesseeq,
		expr_more,
		expr_moreeq,
		expr_logicaleq,
		expr_noteq,
		expr_and,
		expr_xor,
		expr_or,
		expr_logicaland,
		expr_logicalor,
		expr_ternary,
		expr_assign,
		expr_addassign,
		expr_subassign,
		expr_multassign,
		expr_divassign,
		expr_modassign,
		expr_shiftleftassign,
		expr_shiftrightassign,
		expr_andassign,
		expr_xorassign,
		expr_orassign,
		expr_comma
	} ;

	struct expr_node_t : public node_t
	{
		expr_node_t(expression_op_e op) :node_t(EXPRESSION_NODE),op_id(op), is_lvalue(true) {}

		expression_op_e op_id;

		type_node_ptr_t expr_type;

		bool is_lvalue;
	};

	struct arglist_node_t : public node_t
	{
		arglist_node_t() :node_t(ARG_LIST_NODE) {}

		vector<expr_node_ptr_t> args;
	};

	typedef shared_ptr<arglist_node_t> arglist_node_ptr_t;

	struct literal_expr_node_t : public expr_node_t
	{
		literal_expr_node_t() :expr_node_t(expr_literal) {}

		literal_node_ptr_t literal;
	};

	struct identifier_expr_node_t : public expr_node_t
	{
		identifier_expr_node_t() :expr_node_t(expr_identifier) {}

		identifier_node_ptr_t id;

		bridge_ptr_t ast_def;
	};
	
	struct unary_expr_node_t : public expr_node_t
	{
		unary_expr_node_t(expression_op_e op) :expr_node_t(op) {}

		expr_node_ptr_t operand;
	};

	typedef shared_ptr<unary_expr_node_t>
		unary_expr_node_ptr_t;

	struct funcall_expr_node_t : public expr_node_t
	{
		funcall_expr_node_t() :expr_node_t(expr_funcall) {}

		expr_node_ptr_t fun_expr;

		arglist_node_ptr_t arg_list;
	};

	struct cast_expr_node_t : public expr_node_t
	{
		cast_expr_node_t() :expr_node_t(expr_cast) {}

		type_node_ptr_t	type_node;

		expr_node_ptr_t operand;
	};

	struct sizeoftype_expr_node_t : public expr_node_t
	{
		sizeoftype_expr_node_t() :expr_node_t(expr_sizeoftype) {}

		type_node_ptr_t	type_node;
	};

	struct binary_expr_node_t : public expr_node_t
	{
		binary_expr_node_t(expression_op_e op) :expr_node_t(op) {}

		expr_node_ptr_t operand1;

		expr_node_ptr_t operand2;
	};

	typedef 
	shared_ptr<binary_expr_node_t> binary_expr_node_ptr_t;

	struct identifier_expression_node_t : public expr_node_t
	{
		identifier_expression_node_t() :expr_node_t(expr_identifier) {}

		expr_node_ptr_t operand;

		identifier_node_ptr_t id;
	};

	struct ternary_expr_node_t : public expr_node_t
	{
		ternary_expr_node_t() :expr_node_t(expr_ternary) {}

		expr_node_ptr_t condition;

		expr_node_ptr_t true_expr;

		expr_node_ptr_t false_expr;
	};

	struct comma_expr_node_t : public expr_node_t
	{
		comma_expr_node_t() :expr_node_t(expr_comma) {}

		arglist_node_ptr_t arg_list;
	};

#define DEFINE_UNARY_EXPR_NODE_TYPE(NAME) struct NAME##_expr_node_t : public unary_expr_node_t { NAME##_expr_node_t() : unary_expr_node_t(expr_##NAME) {} }

	DEFINE_UNARY_EXPR_NODE_TYPE(sfxplusplus);
	DEFINE_UNARY_EXPR_NODE_TYPE(sfxminmin);
	DEFINE_UNARY_EXPR_NODE_TYPE(preplusplus);
	DEFINE_UNARY_EXPR_NODE_TYPE(preminmin);
	DEFINE_UNARY_EXPR_NODE_TYPE(plus);
	DEFINE_UNARY_EXPR_NODE_TYPE(min);
	DEFINE_UNARY_EXPR_NODE_TYPE(not);
	DEFINE_UNARY_EXPR_NODE_TYPE(bwsnot);
	DEFINE_UNARY_EXPR_NODE_TYPE(sizeofexpr);
	DEFINE_UNARY_EXPR_NODE_TYPE(deref);
	DEFINE_UNARY_EXPR_NODE_TYPE(addressof);
	
#define DEFINE_IDENTIFIER_EXPR_NODE_TYPE(NAME) struct NAME##_expr_node_t : public identifier_expression_node_t { NAME##_expr_node_t () : identifier_expression_node_t() { op_id = expr_##NAME; } }
	DEFINE_IDENTIFIER_EXPR_NODE_TYPE(dot);
	DEFINE_IDENTIFIER_EXPR_NODE_TYPE(arrow);

#define DEFINE_BINARY_EXPR_NODE_TYPE(NAME) struct NAME##_expr_node_t : public binary_expr_node_t { NAME##_expr_node_t() : binary_expr_node_t(expr_##NAME) {} }
	DEFINE_BINARY_EXPR_NODE_TYPE(index);
	DEFINE_BINARY_EXPR_NODE_TYPE(mult);
	DEFINE_BINARY_EXPR_NODE_TYPE(div);
	DEFINE_BINARY_EXPR_NODE_TYPE(mod);
	DEFINE_BINARY_EXPR_NODE_TYPE(add);
	DEFINE_BINARY_EXPR_NODE_TYPE(sub);
	DEFINE_BINARY_EXPR_NODE_TYPE(shiftleft);
	DEFINE_BINARY_EXPR_NODE_TYPE(shiftright);
	DEFINE_BINARY_EXPR_NODE_TYPE(less);
	DEFINE_BINARY_EXPR_NODE_TYPE(lesseeq);
	DEFINE_BINARY_EXPR_NODE_TYPE(more);
	DEFINE_BINARY_EXPR_NODE_TYPE(moreeq);
	DEFINE_BINARY_EXPR_NODE_TYPE(logicaleq);
	DEFINE_BINARY_EXPR_NODE_TYPE(noteq);
	DEFINE_BINARY_EXPR_NODE_TYPE(and);
	DEFINE_BINARY_EXPR_NODE_TYPE(xor);
	DEFINE_BINARY_EXPR_NODE_TYPE(or);
	DEFINE_BINARY_EXPR_NODE_TYPE(logicaland);
	DEFINE_BINARY_EXPR_NODE_TYPE(logicalor);
	DEFINE_BINARY_EXPR_NODE_TYPE(assign);
	DEFINE_BINARY_EXPR_NODE_TYPE(addassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(subassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(multassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(divassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(modassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(shiftleftassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(shiftrightassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(andassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(xorassign);
	DEFINE_BINARY_EXPR_NODE_TYPE(orassign);

#define DEFINE_EXPR_NODE_PTR_TYPE(NAME) typedef shared_ptr<NAME##_expr_node_t> NAME##_expr_node_ptr_t
	DEFINE_EXPR_NODE_PTR_TYPE(sfxplusplus);
	DEFINE_EXPR_NODE_PTR_TYPE(sfxminmin);
	DEFINE_EXPR_NODE_PTR_TYPE(preplusplus);
	DEFINE_EXPR_NODE_PTR_TYPE(preminmin);
	DEFINE_EXPR_NODE_PTR_TYPE(plus);
	DEFINE_EXPR_NODE_PTR_TYPE(min);
	DEFINE_EXPR_NODE_PTR_TYPE(not);
	DEFINE_EXPR_NODE_PTR_TYPE(bwsnot);
	DEFINE_EXPR_NODE_PTR_TYPE(sizeofexpr);
	DEFINE_EXPR_NODE_PTR_TYPE(funcall);
	DEFINE_EXPR_NODE_PTR_TYPE(index);
	DEFINE_EXPR_NODE_PTR_TYPE(cast);
	DEFINE_EXPR_NODE_PTR_TYPE(deref);
	DEFINE_EXPR_NODE_PTR_TYPE(addressof);
	DEFINE_EXPR_NODE_PTR_TYPE(sizeoftype);
	DEFINE_EXPR_NODE_PTR_TYPE(mult);
	DEFINE_EXPR_NODE_PTR_TYPE(div);
	DEFINE_EXPR_NODE_PTR_TYPE(mod);
	DEFINE_EXPR_NODE_PTR_TYPE(add);
	DEFINE_EXPR_NODE_PTR_TYPE(sub);
	DEFINE_EXPR_NODE_PTR_TYPE(shiftleft);
	DEFINE_EXPR_NODE_PTR_TYPE(shiftright);
	DEFINE_EXPR_NODE_PTR_TYPE(less);
	DEFINE_EXPR_NODE_PTR_TYPE(lesseeq);
	DEFINE_EXPR_NODE_PTR_TYPE(more);
	DEFINE_EXPR_NODE_PTR_TYPE(moreeq);
	DEFINE_EXPR_NODE_PTR_TYPE(logicaleq);
	DEFINE_EXPR_NODE_PTR_TYPE(noteq);
	DEFINE_EXPR_NODE_PTR_TYPE(and);
	DEFINE_EXPR_NODE_PTR_TYPE(xor);
	DEFINE_EXPR_NODE_PTR_TYPE(or);
	DEFINE_EXPR_NODE_PTR_TYPE(logicaland);
	DEFINE_EXPR_NODE_PTR_TYPE(logicalor);
	DEFINE_EXPR_NODE_PTR_TYPE(dot);
	DEFINE_EXPR_NODE_PTR_TYPE(arrow);
	DEFINE_EXPR_NODE_PTR_TYPE(identifier);
	DEFINE_EXPR_NODE_PTR_TYPE(literal);
	DEFINE_EXPR_NODE_PTR_TYPE(ternary);
	DEFINE_EXPR_NODE_PTR_TYPE(assign);
	DEFINE_EXPR_NODE_PTR_TYPE(addassign);	
	DEFINE_EXPR_NODE_PTR_TYPE(subassign);
	DEFINE_EXPR_NODE_PTR_TYPE(multassign);
	DEFINE_EXPR_NODE_PTR_TYPE(divassign);
	DEFINE_EXPR_NODE_PTR_TYPE(modassign);
	DEFINE_EXPR_NODE_PTR_TYPE(shiftleftassign);
	DEFINE_EXPR_NODE_PTR_TYPE(shiftrightassign);
	DEFINE_EXPR_NODE_PTR_TYPE(andassign);
	DEFINE_EXPR_NODE_PTR_TYPE(xorassign);
	DEFINE_EXPR_NODE_PTR_TYPE(orassign);
	DEFINE_EXPR_NODE_PTR_TYPE(comma);

#define NEW_EXPR_NODE(VAR, NAME) NAME##_expr_node_ptr_t VAR(new NAME##_expr_node_t())

}

