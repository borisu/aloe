#pragma once

using namespace llvm;

namespace aloe
{
	struct compiler_ctx_t {
		LLVMContext* llvm_ctx	= nullptr;
		Module* llvm_module		= nullptr;
		IRBuilder<>* llvm_ir	= nullptr;
		ast_ptr_t ast;
	};

	struct compiler_exeption_t : public std::exception {

		compiler_exeption_t(const char *format, ...);

		char buffer[4096];

		virtual const char* what() const noexcept override {
			return buffer;
		}
		
	};

	class llvmir_compiler_t : public compiler_t
	{
		virtual bool compile(
			ast_ptr_t ast,
			ostream& out) override;

	protected:

		virtual void walk_prog(compiler_ctx_t *ctx, prog_node_ptr_t node);

		virtual void walk_func(compiler_ctx_t* ctx, fun_node_ptr_t node);

		virtual void get_fun_llvm_type(compiler_ctx_t* ctx, fun_node_ptr_t node, Type*& ret_type, std::vector<Type*>& param_types);

		virtual void get_llvm_type(compiler_ctx_t* ctx, type_node_ptr_t node, Type*& type);


	};

}


