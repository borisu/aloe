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
			ast_t_ptr ast,
			ostream& out) override;

		virtual void set_validate(bool validate) override;

		virtual void set_no_debug(bool no_debug) override;

	protected:

		virtual void walk_prog(compiler_ctx_t_ptr ctx, prog_node_t_ptr node);

		virtual Type* emit_ir_type(compiler_ctx_t_ptr ctx, type_node_t_ptr node);

		virtual Type* emit_ir_type(compiler_ctx_t_ptr ctx, aloe_type_t_ptr atype);

		virtual value_t_ptr emit_fun(compiler_ctx_t_ptr ctx, fun_node_t_ptr node);

		virtual void emit_fun_definition(compiler_ctx_t_ptr ctx, Function *fun, fun_node_t_ptr node);

		virtual void emit_return(compiler_ctx_t_ptr ctx, return_node_t_ptr node);

		virtual void emit_var(compiler_ctx_t_ptr ctx, var_node_t_ptr node);

		virtual value_t_ptr emit_default(compiler_ctx_t_ptr ctx, aloe_type_t_ptr atype);

		virtual value_t_ptr emit_expr_identifier(compiler_ctx_t_ptr ctx, identifier_expr_node_t_ptr node);

		virtual Value* emit_rvalue(compiler_ctx_t_ptr ctx, value_t_ptr expr);

	
		//
		// EXPRESSIONS
		//
		virtual value_t_ptr emit_expr_value(compiler_ctx_t_ptr ctx, expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_fun_call(compiler_ctx_t_ptr ctx, funcall_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_literal(compiler_ctx_t_ptr ctx, literal_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_comma(compiler_ctx_t_ptr ctx, comma_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_assign(compiler_ctx_t_ptr ctx, assign_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_prefix(compiler_ctx_t_ptr ctx, unary_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_postfix(compiler_ctx_t_ptr ctx, unary_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_addressof(compiler_ctx_t_ptr ctx, addressof_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_deref(compiler_ctx_t_ptr ctx, deref_expr_node_t_ptr node);

		virtual value_t_ptr emit_expr_index(compiler_ctx_t_ptr ctx, index_expr_node_t_ptr node);


		virtual value_t_ptr emit_literal(compiler_ctx_t_ptr ctx, literal_node_t_ptr node);

		virtual value_t_ptr emit_arithmetic_binary(compiler_ctx_t_ptr ctx, binary_expr_node_t_ptr node);

		virtual value_t_ptr emit_cmp_binary(compiler_ctx_t_ptr ctx, binary_expr_node_t_ptr node);

		virtual value_t_ptr emit_assign_arithmetic_binary(compiler_ctx_t_ptr ctx, binary_expr_node_t_ptr node);

		//
		// HELPERS
		//
		virtual value_t_ptr emit_constant(compiler_ctx_t_ptr ctx, variant<int,float, double, char> val, aloe_type_t_ptr atype, node_t_ptr node);

		virtual value_t_ptr emit_raw_assign(compiler_ctx_t_ptr ctx, value_t_ptr lhs, value_t_ptr rhs, node_t_ptr node);

		virtual value_t_ptr emit_raw_binary_arithmetic(compiler_ctx_t_ptr ctx, expression_op_e base_op,  value_t_ptr lhs, value_t_ptr rhs, node_t_ptr node);

		//
		// TYPE TESTERS --> THROW EXCEPTIONS 
		// 
		virtual void check_ir_type_equal(compiler_ctx_t_ptr ctx, Value* v1, Value *v2, node_t_ptr node);

		virtual void check_assign_val_type_equality(compiler_ctx_t_ptr ctx, value_t_ptr v1, value_t_ptr v2, node_t_ptr node);

		virtual void check_lvalue(compiler_ctx_t_ptr ctx, value_t_ptr v, node_t_ptr node);

		virtual llvm::DebugLoc init_dloc(compiler_ctx_t_ptr ctx, node_t_ptr node);

		virtual DIScope* get_scope(compiler_ctx_t_ptr ctx);


	private:

		di_cache_t_ptr di_cache;

		map<node_t_ptr, value_t_ptr> obj_cache;

		bool validate;

		bool no_debug;

		node_t_ptr curr_node;

	};

}