#pragma once
#include <stack>
#include "aloe\type.h"
#include "value.h"
#include "type_cache.h"

using namespace llvm;

namespace aloe
{
	struct compiler_ctx_t {
		LLVMContext*	llvm_ctx	 = nullptr;
		Module*			llvm_module	 = nullptr;
		IRBuilder<>*	llvm_ir		 = nullptr;
		DIBuilder*		llvm_di		 = nullptr;
		DIFile*			llvm_di_file = nullptr;
		ast_ptr_t		ast;
		std::stack<value_ptr_t> fun_desc_stack;
	};

	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(
			ast_ptr_t ast,
			ostream& out) override;

	protected:

		virtual void walk_prog(compiler_ctx_t *ctx, prog_node_ptr_t node);

		virtual value_type_ptr_t emit_type(compiler_ctx_t* ctx, type_node_ptr_t node);

		virtual value_type_ptr_t emit_fun_type(compiler_ctx_t* ctx, fun_type_node_ptr_t node);

		virtual void emit_fun(compiler_ctx_t* ctx, fun_node_ptr_t node);

		virtual void emit_exec_statement(compiler_ctx_t* ctx, node_ptr_t node);

		virtual void emit_return(compiler_ctx_t* ctx, return_node_ptr_t node);

		virtual value_ptr_t emit_expression(compiler_ctx_t* ctx, expr_node_ptr_t node);

		virtual value_ptr_t emit_expr_fun_call(compiler_ctx_t* ctx, funcall_expr_node_ptr_t node);

		virtual value_ptr_t emit_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node);

		virtual value_ptr_t emit_literal(compiler_ctx_t* ctx, literal_node_ptr_t node);

		virtual void emit_var(compiler_ctx_t* ctx, var_node_ptr_t node);

		virtual value_ptr_t emit_default(compiler_ctx_t* ctx, value_type_ptr_t node);

		virtual value_ptr_t emit_expr_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node);

		virtual Value* emit_r_value(compiler_ctx_t* ctx, value_ptr_t expr);

		virtual Value* emit_cast(compiler_ctx_t* ctx, value_ptr_t val, value_type_ptr_t target_type, node_ptr_t node);


	private:

		type_cache_t type_cache;

		map<node_ptr_t, value_ptr_t> id_ssa_cache;

		
	};

}


