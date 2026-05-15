#pragma once
#include <stack>
#include "aloe\type.h"
#include "value.h"
#include "type_cache.h"

using namespace llvm;

namespace aloe
{
	struct compiler_ctx_t {
		LLVMContext* llvm_ctx = nullptr;
		Module* llvm_module = nullptr;
		IRBuilder<>* llvm_ir = nullptr;
		DIBuilder* llvm_di = nullptr;
		DIFile* llvm_di_file = nullptr;
		DICompileUnit* llvm_cu = nullptr;
		ast_ptr_t		ast;
		std::stack<value_ptr_t> fun_desc_stack;
	};

	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(
			ast_ptr_t ast,
			ostream& out) override;

	protected:

		virtual void walk_prog(compiler_ctx_t* ctx, prog_node_ptr_t node);

		virtual value_type_ptr_t emit_type(compiler_ctx_t* ctx, type_node_ptr_t node);

		virtual value_type_ptr_t emit_fun_type(compiler_ctx_t* ctx, fun_type_node_ptr_t node);

		virtual void emit_fun(compiler_ctx_t* ctx, fun_node_ptr_t node);

		virtual void emit_exec_statement(compiler_ctx_t* ctx, node_ptr_t node);

		virtual void emit_return(compiler_ctx_t* ctx, return_node_ptr_t node);


		virtual void emit_var(compiler_ctx_t* ctx, var_node_ptr_t node);

		virtual value_ptr_t emit_default(compiler_ctx_t* ctx, value_type_ptr_t node);

		virtual value_ptr_t emit_expr_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node);

		virtual Value* emit_r_value(compiler_ctx_t* ctx, value_ptr_t expr);

		virtual Value* emit_cast(compiler_ctx_t* ctx, value_ptr_t val, value_type_ptr_t target_type, node_ptr_t node);



		//
		// EXPRESSIONS
		//
		virtual value_ptr_t emit_expression(compiler_ctx_t* ctx, expr_node_ptr_t node);

		virtual value_ptr_t emit_expr_fun_call(compiler_ctx_t* ctx, funcall_expr_node_ptr_t node);

		virtual value_ptr_t emit_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node);

		virtual value_ptr_t emit_literal(compiler_ctx_t* ctx, literal_node_ptr_t node);

		virtual value_ptr_t emit_expr_assign(compiler_ctx_t* ctx, assign_expr_node_ptr_t node);

		virtual value_ptr_t emit_arithmetic_binary(compiler_ctx_t* ctx, binary_expr_node_ptr_t node);

		virtual value_ptr_t emit_cmp_binary(compiler_ctx_t* ctx, binary_expr_node_ptr_t node);

		virtual value_ptr_t emit_assign_arithmetic_binary(compiler_ctx_t* ctx, binary_expr_node_ptr_t node);

		virtual value_ptr_t emit_comma(compiler_ctx_t* ctx, comma_expr_node_ptr_t node);

		virtual value_ptr_t emit_prefix(compiler_ctx_t* ctx, unary_expr_node_ptr_t node);

		virtual value_ptr_t emit_postfix(compiler_ctx_t* ctx, unary_expr_node_ptr_t node);

	
		//
		// HELPERS
		//
		virtual value_ptr_t emit_constant(compiler_ctx_t* ctx, variant<int,float, double, char> val, type_category_e type);

		virtual value_ptr_t emit_raw_assign(compiler_ctx_t* ctx, value_ptr_t lhs, value_ptr_t rhs, node_ptr_t node);

		virtual value_ptr_t emit_raw_binary_arithmetic(compiler_ctx_t* ctx, expression_op_e base_op,  value_ptr_t lhs, value_ptr_t rhs, node_ptr_t node);

		//
		// TYPE TESTERS
		// 
		virtual void check_ssa_type_equality(compiler_ctx_t* ctx, value_ptr_t v1, value_ptr_t v2, node_ptr_t node);

		virtual void check_lvalue(compiler_ctx_t* ctx, value_ptr_t v, node_ptr_t node);

		virtual llvm::DebugLoc InitDloc(compiler_ctx_t* ctx, node_ptr_t node);


	private:

		type_cache_t type_cache;

		map<node_ptr_t, value_ptr_t> id_ssa_cache;


	};

}