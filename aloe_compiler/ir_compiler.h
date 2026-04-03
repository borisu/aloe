#pragma once
#include "type.h"
#include "value.h"

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
	};

	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(
			ast_ptr_t ast,
			ostream& out) override;

	protected:

		virtual void walk_prog(compiler_ctx_t *ctx, prog_node_ptr_t node);

		virtual type_ptr_t walk_type(compiler_ctx_t* ctx, type_node_ptr_t node);

		virtual type_ptr_t walk_built_in(compiler_ctx_t* ctx, builtin_node_ptr_t node);

		virtual type_ptr_t walk_func(compiler_ctx_t* ctx, fun_node_ptr_t node);

		virtual void walk_exec_statement(compiler_ctx_t* ctx, node_ptr_t node);

		virtual value_ptr_t walk_expression(compiler_ctx_t* ctx, expr_node_ptr_t node);

		virtual value_ptr_t walk_fun_call (compiler_ctx_t* ctx, funcall_expr_node_ptr_t node);

		virtual value_ptr_t walk_expr_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node);

		virtual value_ptr_t walk_literal(compiler_ctx_t* ctx, literal_node_ptr_t node);

		virtual value_ptr_t walk_identifier(compiler_ctx_t* ctx, identifier_expr_node_ptr_t node);

	private:

		typedef std::pair<size_t, Type*> di_cache_key_t;
		map<di_cache_key_t, DIType*> di_cache;

		typedef std::pair<size_t, node_ptr_t> type_cache_key_t;
		map<type_cache_key_t, type_ptr_t> type_cache;
	    

		DIType* get_dit(di_cache_key_t key, DIType* dit);

		
	};

}


