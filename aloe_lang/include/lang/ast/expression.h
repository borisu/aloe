#pragma once
#include <string>
#include <vector>
#include <variant>
#include "node.h"
#include "literal.h"
#include "identifier.h"
#include "type.h"

using namespace std;

namespace aloe
{
	struct expr_node_t;
	typedef shared_ptr<expr_node_t> expr_node_t_ptr;

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
		expr_node_t(expression_op_e op) :node_t(EXPRESSION_NODE),op_id(op), is_lvalue(false) {}

		expression_op_e op_id;

		type_proxy_t_ptr type;

		bool is_lvalue;
	};

	struct arglist_node_t : public node_t
	{
		arglist_node_t() :node_t(ARG_LIST_NODE) {}

		vector<expr_node_t_ptr> args;
	};

	typedef shared_ptr<arglist_node_t> arglist_node_t_ptr;

	struct literal_expr_node_t : public expr_node_t
	{
		literal_expr_node_t() :expr_node_t(expr_literal) {}

		literal_node_t_ptr literal;
	};

	struct identifier_expr_node_t : public expr_node_t
	{
		identifier_expr_node_t() :expr_node_t(expr_identifier) {}

		identifier_node_t_ptr id;

		node_proxy_t_ptr ref;

	};
	
	struct unary_expr_node_t : public expr_node_t
	{
		unary_expr_node_t(expression_op_e op) :expr_node_t(op) {}

		expr_node_t_ptr operand;
	};

	typedef shared_ptr<unary_expr_node_t>
		unary_expr_node_t_ptr;

	struct funcall_expr_node_t : public expr_node_t
	{
		funcall_expr_node_t() :expr_node_t(expr_funcall) {}

		expr_node_t_ptr fun_expr;

		arglist_node_t_ptr arg_list;
	};

	struct cast_expr_node_t : public expr_node_t
	{
		cast_expr_node_t() :expr_node_t(expr_cast) {}

		expr_node_t_ptr operand;

		type_proxy_t_ptr type;
	};

	struct sizeoftype_expr_node_t : public expr_node_t
	{
		sizeoftype_expr_node_t() :expr_node_t(expr_sizeoftype) {}

		type_proxy_t_ptr type;

	};

	struct binary_expr_node_t : public expr_node_t
	{
		binary_expr_node_t(expression_op_e op) :expr_node_t(op) {}

		expr_node_t_ptr operand1;

		expr_node_t_ptr operand2;
	};

	typedef 
	shared_ptr<binary_expr_node_t> binary_expr_node_t_ptr;

	struct ternary_expr_node_t : public expr_node_t
	{
		ternary_expr_node_t() :expr_node_t(expr_ternary) {}

		expr_node_t_ptr condition;

		expr_node_t_ptr true_expr;

		expr_node_t_ptr false_expr;
	};

	struct comma_expr_node_t : public expr_node_t
	{
		comma_expr_node_t() :expr_node_t(expr_comma) {}

		arglist_node_t_ptr arg_list;
	};

	struct dot_expr_node_t : public expr_node_t
	{
		dot_expr_node_t(): expr_node_t(expr_dot) {}

		expr_node_t_ptr operand;

		identifier_node_t_ptr id;
	};

	struct arrow_expr_node_t : public expr_node_t
	{
		arrow_expr_node_t() : expr_node_t(expr_arrow) {}

		expr_node_t_ptr operand;

		identifier_node_t_ptr id;
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

#define DEFINE_EXPR_NODE_t_ptrYPE(NAME) typedef shared_ptr<NAME##_expr_node_t> NAME##_expr_node_t_ptr
	DEFINE_EXPR_NODE_t_ptrYPE(sfxplusplus);
	DEFINE_EXPR_NODE_t_ptrYPE(sfxminmin);
	DEFINE_EXPR_NODE_t_ptrYPE(preplusplus);
	DEFINE_EXPR_NODE_t_ptrYPE(preminmin);
	DEFINE_EXPR_NODE_t_ptrYPE(plus);
	DEFINE_EXPR_NODE_t_ptrYPE(min);
	DEFINE_EXPR_NODE_t_ptrYPE(not);
	DEFINE_EXPR_NODE_t_ptrYPE(bwsnot);
	DEFINE_EXPR_NODE_t_ptrYPE(sizeofexpr);
	DEFINE_EXPR_NODE_t_ptrYPE(funcall);
	DEFINE_EXPR_NODE_t_ptrYPE(index);
	DEFINE_EXPR_NODE_t_ptrYPE(cast);
	DEFINE_EXPR_NODE_t_ptrYPE(deref);
	DEFINE_EXPR_NODE_t_ptrYPE(addressof);
	DEFINE_EXPR_NODE_t_ptrYPE(sizeoftype);
	DEFINE_EXPR_NODE_t_ptrYPE(mult);
	DEFINE_EXPR_NODE_t_ptrYPE(div);
	DEFINE_EXPR_NODE_t_ptrYPE(mod);
	DEFINE_EXPR_NODE_t_ptrYPE(add);
	DEFINE_EXPR_NODE_t_ptrYPE(sub);
	DEFINE_EXPR_NODE_t_ptrYPE(shiftleft);
	DEFINE_EXPR_NODE_t_ptrYPE(shiftright);
	DEFINE_EXPR_NODE_t_ptrYPE(less);
	DEFINE_EXPR_NODE_t_ptrYPE(lesseeq);
	DEFINE_EXPR_NODE_t_ptrYPE(more);
	DEFINE_EXPR_NODE_t_ptrYPE(moreeq);
	DEFINE_EXPR_NODE_t_ptrYPE(logicaleq);
	DEFINE_EXPR_NODE_t_ptrYPE(noteq);
	DEFINE_EXPR_NODE_t_ptrYPE(and);
	DEFINE_EXPR_NODE_t_ptrYPE(xor);
	DEFINE_EXPR_NODE_t_ptrYPE(or);
	DEFINE_EXPR_NODE_t_ptrYPE(logicaland);
	DEFINE_EXPR_NODE_t_ptrYPE(logicalor);
	DEFINE_EXPR_NODE_t_ptrYPE(dot);
	DEFINE_EXPR_NODE_t_ptrYPE(arrow);
	DEFINE_EXPR_NODE_t_ptrYPE(identifier);
	DEFINE_EXPR_NODE_t_ptrYPE(literal);
	DEFINE_EXPR_NODE_t_ptrYPE(ternary);
	DEFINE_EXPR_NODE_t_ptrYPE(assign);
	DEFINE_EXPR_NODE_t_ptrYPE(addassign);	
	DEFINE_EXPR_NODE_t_ptrYPE(subassign);
	DEFINE_EXPR_NODE_t_ptrYPE(multassign);
	DEFINE_EXPR_NODE_t_ptrYPE(divassign);
	DEFINE_EXPR_NODE_t_ptrYPE(modassign);
	DEFINE_EXPR_NODE_t_ptrYPE(shiftleftassign);
	DEFINE_EXPR_NODE_t_ptrYPE(shiftrightassign);
	DEFINE_EXPR_NODE_t_ptrYPE(andassign);
	DEFINE_EXPR_NODE_t_ptrYPE(xorassign);
	DEFINE_EXPR_NODE_t_ptrYPE(orassign);
	DEFINE_EXPR_NODE_t_ptrYPE(comma);

#define NEW_EXPR_NODE(VAR, NAME) NAME##_expr_node_t_ptr VAR(new NAME##_expr_node_t())

}

