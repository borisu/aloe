#include "pch.h"
#include "compiler_ctx.h"

using namespace aloe;


base_ctx_modifier_t::base_ctx_modifier_t(compiler_ctx_ptr_t prev)
{
	this->prev = prev;
}

LLVMContext* 
base_ctx_modifier_t::ctx()
{
	if (prev == nullptr)
		return nullptr;

	return prev->ctx();
}


// llvm modifier
Module* 
base_ctx_modifier_t::module()
{
	if (prev == nullptr)
		return nullptr;

	return prev->module();
}

IRBuilder<>* 
base_ctx_modifier_t::builder()
{
	if (prev == nullptr)
		return nullptr;
	return prev->builder();
}

DIBuilder* 
base_ctx_modifier_t::di_builder()
{
	if (prev == nullptr)
		return nullptr;
	return prev->di_builder();
}

DIFile* 
base_ctx_modifier_t::di_file()
{
	if (prev == nullptr)
		return nullptr;
	return prev->di_file();
}

DICompileUnit* 
base_ctx_modifier_t::llvm_cu()
{
	if (prev == nullptr)
		return nullptr;
	return prev->llvm_cu();
}

// function modifer
Function* 
base_ctx_modifier_t::curr_fun()
{
	if (prev == nullptr)
		return nullptr;
	return prev->curr_fun();
}	

// ast modifier
ast_ptr_t 
base_ctx_modifier_t::ast()
{
	if (prev == nullptr)
		return nullptr;
	return prev->ast();
}

llvm_ctx_modifier_t::llvm_ctx_modifier_t(
	LLVMContext* ctx,
	Module* module,
	IRBuilder<>* builder,
	DIBuilder* di_builder,
	DIFile* di_file,
	DICompileUnit* llvm_cu,
	compiler_ctx_ptr_t prev)
	: base_ctx_modifier_t(prev)
{
	this->_ctx			= ctx;
	this->_module		= module;
	this->_builder		= builder;
	this->_di_builder	= di_builder;
	this->_di_file		= di_file;
	this->_llvm_cu		= llvm_cu;
}

// llvm_ctx_modifier_t implementation
Module*
llvm_ctx_modifier_t::module()
{
	return _module;
}

LLVMContext*
llvm_ctx_modifier_t::ctx()
{
	return _ctx;
}

IRBuilder<>*
llvm_ctx_modifier_t::builder()
{
	return _builder;
}

DIBuilder*
llvm_ctx_modifier_t::di_builder()
{
	return _di_builder;
}

DIFile*
llvm_ctx_modifier_t::di_file()
{
	return _di_file;
}

DICompileUnit*
llvm_ctx_modifier_t::llvm_cu()
{
	return _llvm_cu;
}

// ast_ctx_modifier_t implementation
ast_ctx_modifier_t::ast_ctx_modifier_t(
	ast_ptr_t _ast,
	compiler_ctx_ptr_t prev)
	: base_ctx_modifier_t(prev)
{
	this->_ast = _ast;
}

ast_ptr_t 
ast_ctx_modifier_t::ast()
{
	return _ast;
}

fun_ctx_modifier_t::fun_ctx_modifier_t(
	Function* _curr_fun,
	compiler_ctx_ptr_t prev)
	: base_ctx_modifier_t(prev)
{
	this->_curr_fun = _curr_fun;
}

Function*
fun_ctx_modifier_t::curr_fun()
{
	return _curr_fun;
}