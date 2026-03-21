#pragma once
#include "di_mapper.h"

using namespace llvm;

namespace aloe
{
	struct compiler_ctx_t {
		LLVMContext* llvm_ctx	= nullptr;
		Module* llvm_module		= nullptr;
		IRBuilder<>* llvm_ir	= nullptr;
		DIBuilder* llvm_di	    = nullptr;
		DIFile* llvm_di_file	= nullptr;

		di_mapper_t* di_mapper  = nullptr;
		ast_ptr_t ast;
	};

	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(
			ast_ptr_t ast,
			ostream& out) override;

	protected:

		virtual void walk_prog(compiler_ctx_t *ctx, prog_node_ptr_t node);

		virtual void walk_func(compiler_ctx_t* ctx, fun_node_ptr_t node);

		virtual void walk_exec_statement(compiler_ctx_t* ctx, node_ptr_t node);

		virtual void walk_expression(compiler_ctx_t* ctx, expr_node_ptr_t node);

		virtual void walk_literal(compiler_ctx_t* ctx, literal_expr_node_ptr_t node);
		
	};

}


