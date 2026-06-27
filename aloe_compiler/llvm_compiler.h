#pragma once
#include <stack>
#include "lang\aloe_type.h"
#include "lang\ast\ast.h"
#include "lang\compiler.h"
#include "value.h"
#include "di_cache.h"
#include "compiler_ctx.h"

using namespace llvm;

namespace aloe
{
	class llvmir_compiler_t : public compiler_t
	{
	public:

		llvmir_compiler_t();

		virtual bool compile(
			ast_ptr_t ast,
			ostream& out) override;

		virtual void set_validate(bool validate) override;

	protected:

		virtual void walk_prog(compiler_ctx_ptr_t ctx, prog_node_ptr_t node);

		virtual Type* emit_type(compiler_ctx_ptr_t ctx, type_node_ptr_t node);

		virtual Type* emit_type(compiler_ctx_ptr_t ctx, aloe_type_ptr_t type);

		virtual value_ptr_t emit_fun(compiler_ctx_ptr_t ctx, fun_node_ptr_t node);

		virtual void emit_fun_definition(compiler_ctx_ptr_t ctx, Function *fun, fun_node_ptr_t node);

		virtual void emit_exec_statement(compiler_ctx_ptr_t ctx, node_ptr_t node);

		virtual void emit_return(compiler_ctx_ptr_t ctx, return_node_ptr_t node);

		virtual void emit_var(compiler_ctx_ptr_t ctx, var_node_ptr_t node);

		virtual value_ptr_t emit_default(compiler_ctx_ptr_t ctx, aloe_type_ptr_t type);

		virtual value_ptr_t emit_expr_identifier(compiler_ctx_ptr_t ctx, identifier_expr_node_ptr_t node);

		virtual Value* emit_r_value(compiler_ctx_ptr_t ctx, value_ptr_t expr);

	
		//
		// EXPRESSIONS
		//
		virtual value_ptr_t emit_expression(compiler_ctx_ptr_t ctx, expr_node_ptr_t node);

		virtual value_ptr_t emit_expr_fun_call(compiler_ctx_ptr_t ctx, funcall_expr_node_ptr_t node);

		virtual value_ptr_t emit_expr_literal(compiler_ctx_ptr_t ctx, literal_expr_node_ptr_t node);

		virtual value_ptr_t emit_literal(compiler_ctx_ptr_t ctx, literal_node_ptr_t node);

		virtual value_ptr_t emit_expr_assign(compiler_ctx_ptr_t ctx, assign_expr_node_ptr_t node);

		virtual value_ptr_t emit_arithmetic_binary(compiler_ctx_ptr_t ctx, binary_expr_node_ptr_t node);

		virtual value_ptr_t emit_cmp_binary(compiler_ctx_ptr_t ctx, binary_expr_node_ptr_t node);

		virtual value_ptr_t emit_assign_arithmetic_binary(compiler_ctx_ptr_t ctx, binary_expr_node_ptr_t node);

		virtual value_ptr_t emit_comma(compiler_ctx_ptr_t ctx, comma_expr_node_ptr_t node);

		virtual value_ptr_t emit_prefix(compiler_ctx_ptr_t ctx, unary_expr_node_ptr_t node);

		virtual value_ptr_t emit_postfix(compiler_ctx_ptr_t ctx, unary_expr_node_ptr_t node);

	
		//
		// HELPERS
		//
		virtual value_ptr_t emit_constant(compiler_ctx_ptr_t ctx, variant<int,float, double, char> val, aloe_type_ptr_t type);

		virtual value_ptr_t emit_raw_assign(compiler_ctx_ptr_t ctx, value_ptr_t lhs, value_ptr_t rhs, node_ptr_t node);

		virtual value_ptr_t emit_raw_binary_arithmetic(compiler_ctx_ptr_t ctx, expression_op_e base_op,  value_ptr_t lhs, value_ptr_t rhs, node_ptr_t node);

		//
		// TYPE TESTERS
		// 
		virtual void check_val_type_equality(compiler_ctx_ptr_t ctx, value_ptr_t v1, value_ptr_t v2, node_ptr_t node);

		virtual void check_assign_val_type_equality(compiler_ctx_ptr_t ctx, value_ptr_t v1, value_ptr_t v2, node_ptr_t node);

		virtual void check_lvalue(compiler_ctx_ptr_t ctx, value_ptr_t v, node_ptr_t node);

		virtual llvm::DebugLoc init_dloc(compiler_ctx_ptr_t ctx, node_ptr_t node);

		virtual DIScope* get_scope(compiler_ctx_ptr_t ctx);


	private:

		di_cache_ptr_t di_cache;

		map<node_ptr_t, value_ptr_t> id_cache;

		bool validate;

	};

}