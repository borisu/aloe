#include "pch.h"
#include "type_mapper.h"
#include "compiler_exception.h"
#include "aloe\defs.h"


using namespace aloe;
using namespace llvm;

type_mapper_t::type_mapper_t(DIBuilder& dib, Module& m, LLVMContext& ctx)
	: dib(dib), module(m), ctx(ctx) {}


Type* 
type_mapper_t::node_ir_type(node_ptr_t node){
	switch (node->type)
	{
		case LITERAL_NODE:
		{
			return  node_ir_type(PCAST(literal_expr_node_t,node));
			break;
		}
		case FUNCTION_NODE:
		{
			return  fun_ir_type(PCAST(fun_node_t, node));
			break;
		}
		default:
		{
			throw compiler_exeption_t("%s:%d:%d error: cannot convert type %d to IR type", node->line, node->pos, node->type);
		}
	}
}

FunctionType*
type_mapper_t::fun_ir_type(fun_node_ptr_t node)
{
	Type* ir_ret_type = node_ir_type(node->ret_type);

	std::vector<Type*> ir_var_types;
	
	for (auto& var : node->var_list->vars_m)
	{
		auto var_type = node_ir_type(var.second->type);
		ir_var_types.push_back(var_type);
	}

	auto ir_fun_type = FunctionType::get(ir_ret_type, ir_var_types, false);
	return ir_fun_type;
	
}

Type*
type_mapper_t::literal_ir_type(literal_node_ptr_t node)
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

DIType*
type_mapper_t::node_di_type(node_ptr_t node)
{
	DIType* di_type = nullptr;
	// handle basic types
	switch (node->type)
	{
	case SYN_FUNCTION:
	{
		di_type = fun_di_type(PCAST(fun_node_t, node));
		break;
	}
	default:
	{
		throw compiler_exeption_t("%s:%d:%d error: unsupported type %d", node->line, node->pos, node->type);
	}
	}

	return di_type;

}

DISubroutineType* 
type_mapper_t::fun_di_type(fun_node_ptr_t node)
{	
	SmallVector<Metadata*, 8> types;

	// return type
	types.push_back(node_di_type(node->ret_type));

	// params
	for (auto p: node->var_list->vars_m)
	{
		types.push_back(node_di_type(p.second->type));
	}
		
	return dib.createSubroutineType(
		dib.getOrCreateTypeArray(types)
	);
}

