#pragma once
#pragma once
#include <stack>
#include "lang\aloe_type.h"
#include "lang\ast\ast.h"
#include "lang\compiler.h"
#include "value.h"
#include "di_cache.h"

using namespace llvm;

namespace aloe
{
	class compiler_ctx_t;
	typedef shared_ptr<compiler_ctx_t> compiler_ctx_ptr_t;

	class compiler_ctx_t
	{
	public:
		// llvm modifier
		virtual LLVMContext* ctx()			= 0;
		virtual Module* module()			= 0;
		virtual IRBuilder<>* builder()		= 0;
		virtual DIBuilder* di_builder()		= 0;
		virtual DIFile* di_file()			= 0;
		virtual DICompileUnit* llvm_cu()	= 0;

		// function modifer
		virtual Function* curr_fun()		= 0;

		// ast modifier
		virtual ast_ptr_t ast()				= 0;
	};

	class base_ctx_modifier_t : public virtual compiler_ctx_t
	{
	public:
		base_ctx_modifier_t(compiler_ctx_ptr_t prev);

		// llvm modifier
		virtual LLVMContext* ctx()			override;
		virtual Module* module()			override;
		virtual IRBuilder<>* builder()		override;
		virtual DIBuilder* di_builder()		override;
		virtual DIFile* di_file()			override;
		virtual DICompileUnit* llvm_cu()	override;

		// function modifer
		virtual Function* curr_fun()		override;

		// ast modifier
		virtual ast_ptr_t ast()				override;

	protected:

		compiler_ctx_ptr_t prev;

	};

	class llvm_ctx_modifier_t : public virtual base_ctx_modifier_t
	{
	public:

		llvm_ctx_modifier_t(
			LLVMContext* ctx,
			Module* module, 
			IRBuilder<>* builder, 
			DIBuilder* di_builder, 
			DIFile* di_file, 
			DICompileUnit* llvm_cu,
			compiler_ctx_ptr_t prev = nullptr);

		// llvm modifier
		virtual LLVMContext* ctx()			override;
		virtual Module* module()			override;
		virtual IRBuilder<>* builder()		override;
		virtual DIBuilder* di_builder()		override;
		virtual DIFile* di_file()			override;
		virtual DICompileUnit* llvm_cu()	override;

	protected:

		Module*			_module; 
		IRBuilder<>*	_builder; 
		DIBuilder*		_di_builder; 
		DIFile*			_di_file; 
		DICompileUnit*	_llvm_cu;	
		LLVMContext*	_ctx;

	};

	class ast_ctx_modifier_t : public virtual base_ctx_modifier_t
	{
	public:
		ast_ctx_modifier_t(
			ast_ptr_t ast,
			compiler_ctx_ptr_t prev = nullptr);

		// ast modifier
		virtual ast_ptr_t ast() override;

	protected:
		ast_ptr_t _ast;
	};

	class fun_ctx_modifier_t : public virtual base_ctx_modifier_t
	{
	public:
		fun_ctx_modifier_t(
			Function* curr_fun,
			compiler_ctx_ptr_t prev = nullptr);

		virtual Function* curr_fun() override;

	protected:

		Function* _curr_fun;


	};

};

