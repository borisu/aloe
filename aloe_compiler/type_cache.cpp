#include "pch.h"
#include "type_cache.h"
#include "compiler_exception.h"
#include "aloe\defs.h"


using namespace aloe;
using namespace llvm;

type_cache_t::type_cache_t(DIBuilder& dib, Module& m, LLVMContext& ctx)
	: dib(dib), module(m), ctx(ctx) {}

type_ptr_t 
type_cache_t::get_type(node_ptr_t node)
{
	type_ptr_t type(new type_t());

	type->irt	= ir_get_type(node);
	type->dit	= di_get_type(node);
	type->def		= node;
	return nullptr;
}

Type* 
type_cache_t::ir_get_type(node_ptr_t node){

	Type* ir_type = nullptr;

	switch (node->type)
	{
		case LITERAL_NODE:
		{
			ir_type = ir_get_type_lit(PCAST(literal_node_t,node));
			break;
		}
		case FUNCTION_NODE:
		{
			ir_type = ir_get_type_fun(PCAST(fun_node_t, node));
			break;
		}
		default:
		{
			throw compiler_exeption_t("%s:%d:%d error: cannot convert type %d to IR type", node->line, node->pos, node->type);
		}
	}
}

FunctionType*
type_cache_t::ir_get_type_fun(fun_node_ptr_t node)
{
	Type* ir_ret_type = ir_get_type(node->ret_type);

	std::vector<Type*> ir_var_types;
	
	for (auto& var : node->var_list->vars_m)
	{
		auto var_type = ir_get_type(var.second->type);
		ir_var_types.push_back(var_type);
	}

	auto ir_fun_type = FunctionType::get(ir_ret_type, ir_var_types, false);
	return ir_fun_type;
	
}

Type*
type_cache_t::ir_get_type_lit(literal_node_ptr_t node)
{
	switch (node->lit_type)
	{
	case LIT_INT:
	{
		return  Type::getInt32Ty(ctx);
		break;
	}
	case LIT_CHAR:
	{
		return  Type::getInt8Ty(ctx);
		break;
	}
	case LIT_STRING:
	{
		return  PointerType::getUnqual(ctx);
		break;
	}
	default:
	{
		throw compiler_exeption_t("%s:%d:%d error: unsupported type %d", node->line, node->pos, node->lit_type);
	}
	}
}

/*******
* 
*  DI 
* 
********/

DIType*
type_cache_t::di_get_type(node_ptr_t node)
{
	if (di_type_cache.find(node) != di_type_cache.end())
		return di_type_cache[node];

	DIType* di_type = nullptr;
	// handle basic types
	switch (node->type)
	{
	case FUNCTION_NODE:
	{
		di_type = di_get_type_fun(PCAST(fun_node_t, node));
		break;
	}
	default:
	{
		throw compiler_exeption_t("%s:%d:%d error: unsupported type %d", node->line, node->pos, node->type);
	}
	}

	di_type_cache[node] = di_type;

	return di_type;

}

DISubroutineType* 
type_cache_t::di_get_type_fun(fun_node_ptr_t node)
{	
	SmallVector<Metadata*, 8> types;

	// return type
	types.push_back(di_get_type(node->ret_type));

	// params
	for (auto p: node->var_list->vars_m)
	{
		types.push_back(di_get_type(p.second->type));
	}
		
	return dib.createSubroutineType(
		dib.getOrCreateTypeArray(types)
	);
}

