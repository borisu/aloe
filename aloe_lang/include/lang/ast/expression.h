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

		type_proxy_t_ptr p_expr_type;

		void expr_type(type_proxy_t_ptr type)
		{
			this->p_expr_type = type;
		}

		aloe_type_t_ptr expr_type() const
		{
			return p_expr_type->target;
		}

		bool is_lvalue;
	};
	defptr(expr_node_t);

	struct arglist_node_t : public node_t
	{
		arglist_node_t() :node_t(ARG_LIST_NODE) {}

		vector<expr_node_t_ptr> args;
	};
	defptr(arglist_node_t);

	struct literal_expr_node_t : public expr_node_t
	{
		literal_expr_node_t() :expr_node_t(expr_literal) {}

		literal_node_t_ptr literal;
	};
	defptr(literal_expr_node_t);

	struct identifier_expr_node_t : public expr_node_t
	{
		identifier_expr_node_t() :expr_node_t(expr_identifier) {}

		identifier_node_t_ptr idt;

		node_proxy_t_ptr p_ref;

		node_t_ptr ref()
		{
			return p_ref->target;
		}

		void ref(node_proxy_t_ptr ref)
		{
			this->p_ref = ref;
		}
	};
	defptr(identifier_expr_node_t);
	
	struct unary_expr_node_t : public expr_node_t
	{
		unary_expr_node_t(expression_op_e op) :expr_node_t(op) {}

		expr_node_t_ptr operand;
	};
	defptr(unary_expr_node_t);

	struct funcall_expr_node_t : public expr_node_t
	{
		funcall_expr_node_t() :expr_node_t(expr_funcall) {}

		expr_node_t_ptr fun_expr;

		arglist_node_t_ptr arg_list;
	};
	defptr(funcall_expr_node_t);

	struct cast_expr_node_t : public expr_node_t
	{
		cast_expr_node_t() :expr_node_t(expr_cast) {}

		expr_node_t_ptr operand;

	};
	defptr(cast_expr_node_t);

	struct sizeoftype_expr_node_t : public expr_node_t
	{
		sizeoftype_expr_node_t() :expr_node_t(expr_sizeoftype) {}

	};
	defptr(sizeoftype_expr_node_t);

	struct binary_expr_node_t : public expr_node_t
	{
		binary_expr_node_t(expression_op_e op) :expr_node_t(op) {}

		expr_node_t_ptr operand1;

		expr_node_t_ptr operand2;
	};
	defptr(binary_expr_node_t);

	struct ternary_expr_node_t : public expr_node_t
	{
		ternary_expr_node_t() :expr_node_t(expr_ternary) {}

		expr_node_t_ptr condition;

		expr_node_t_ptr true_expr;

		expr_node_t_ptr false_expr;
	};
	defptr(ternary_expr_node_t);

	struct comma_expr_node_t : public expr_node_t
	{
		comma_expr_node_t() :expr_node_t(expr_comma) {}

		arglist_node_t_ptr arg_list;
	};
	defptr(comma_expr_node_t);

	struct dot_expr_node_t : public expr_node_t
	{
		dot_expr_node_t(): expr_node_t(expr_dot) {}

		expr_node_t_ptr operand;

		identifier_node_t_ptr idt;
	};
	defptr(dot_expr_node_t);

	struct arrow_expr_node_t : public expr_node_t
	{
		arrow_expr_node_t() : expr_node_t(expr_arrow) {}

		expr_node_t_ptr operand;

		identifier_node_t_ptr idt;
	};
	defptr(arrow_expr_node_t);

#define DEFINE_UNARY_EXPR_NODE_TYPE(E,N) struct N:public unary_expr_node_t { N() : unary_expr_node_t(E) {} }; defptr(N)

	DEFINE_UNARY_EXPR_NODE_TYPE(expr_sfxplusplus, sfxplusplus_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_sfxminmin, sfxminmin_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_preplusplus, preplusplus_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_preminmin, preminmin_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_plus, plus_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_min, min_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_not, not_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_bwsnot, bwsnot_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_sizeofexpr, sizeofexpr_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_deref, deref_expr_node_t);
	DEFINE_UNARY_EXPR_NODE_TYPE(expr_addressof, addressof_expr_node_t);

#define DEFINE_BINARY_EXPR_NODE_TYPE(E, N) struct N : public binary_expr_node_t { N() : binary_expr_node_t(E) {} }; defptr(N)

	DEFINE_BINARY_EXPR_NODE_TYPE(expr_index, index_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_mult, mult_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_div, div_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_mod, mod_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_add, add_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_sub, sub_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_shiftleft, shiftleft_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_shiftright, shiftright_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_less, less_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_lesseeq, lesseeq_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_more, more_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_moreeq, moreeq_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_logicaleq, logicaleq_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_noteq, noteq_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_and, and_expr_node_t	);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_xor, xor_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_or, or_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_logicaland, logicaland_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_logicalor, logicalor_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_assign, assign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_addassign, addassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_subassign, subassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_multassign, multassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_divassign, divassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_modassign, modassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_shiftleftassign, shiftleftassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_shiftrightassign, shiftrightassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_andassign, andassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_xorassign, xorassign_expr_node_t);
	DEFINE_BINARY_EXPR_NODE_TYPE(expr_orassign, orassign_expr_node_t);
}

