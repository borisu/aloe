#include "pch.h"
#include "di_mapper.h"
#include "compiler_exception.h"


using namespace aloe;
using namespace llvm;

di_mapper_t::di_mapper_t(DIBuilder& dib, Module& m, LLVMContext& ctx)
	: dib(dib), module(m), ctx(ctx) {}


Type* 
di_mapper_t::to_ir(type_node_ptr_t node){

	if (node->ref_count > 1)
	{
		return PointerType::getUnqual(ctx);
	};
	switch (node->type_type)
	{
		case TYPE_INT:
		{
			return  Type::getInt32Ty(ctx);
			break;
		}
		case TYPE_CHAR:       
		{
			return  Type::getInt8Ty(ctx);
			break;
		}
		case TYPE_VOID:
		{
			return  Type::getVoidTy(ctx);
			break;
		}
		default:
		{
			throw compiler_exeption_t("%s:%d:%d error: unsupported type %d", node->line, node->pos, node->type_type);
		}
	}
}

FunctionType* 
di_mapper_t::fun_to_ir(fun_node_ptr_t fun)
{
	Type* ir_ret_type = to_ir(fun->ret_type);

	std::vector<Type*> ir_var_types;
	
	for (auto& var : fun->var_list->vars)
	{
		auto var_type = to_ir(var.second->type);
		ir_var_types.push_back(var_type);
	}

	auto ir_fun_type = FunctionType::get(ir_ret_type, ir_var_types, false);
	return ir_fun_type;
	
}

DISubroutineType* di_mapper_t::fun_to_di(fun_node_ptr_t node)
{	
	SmallVector<Metadata*, 8> types;

	// return type
	types.push_back(to_di(node->ret_type));

	// params
	for (auto p : node->var_list->vars)
	{
		types.push_back(to_di(p.second->type));
	}
		
	return dib.createSubroutineType(
		dib.getOrCreateTypeArray(types)
	);
}

DIType* 
di_mapper_t::to_di(type_node_ptr_t node){

	if (cache.count(node))
	{
		return cache[node];
	}

	// handle pointer types
	if (node->ref_count > 1)
	{
		type_node_ptr_t deref_node = make_shared<type_node_t>(*node);
		deref_node->ref_count--;

		DIType* base = to_di(deref_node);

		uint64_t size =
			module.getDataLayout().getPointerSizeInBits();

		DIType* di_type = dib.createPointerType(
			base,
			size);

		cache[node] = di_type;
		return di_type;
		
	}

	DIType* di_type = nullptr;
	// handle basic types
	switch (node->type_type)
	{
		case TYPE_INT:
		{
			unsigned bits = module.getDataLayout().getTypeSizeInBits(to_ir(node));

			di_type = dib.createBasicType(
				"int",
				bits,
				dwarf::DW_ATE_signed
			);

			break;
		}
		case TYPE_CHAR:       
		{
			unsigned bits = module.getDataLayout().getTypeSizeInBits(to_ir(node));

			di_type = dib.createBasicType(
				"char",
				bits,
				dwarf::DW_ATE_unsigned_char
			);

			break;
		}
		case TYPE_FUNCTION:
		{
			di_type = fun_to_di(std::static_pointer_cast<fun_node_t>(node->type_definition));
			break;
		}
		case TYPE_VOID:
		{
			di_type = dib.createBasicType(
				"void",
				0,
				dwarf::DW_ATE_address
			);
			break;
		}
		default:
		{
			throw compiler_exeption_t("%s:%d:%d error: unsupported type %d", node->line, node->pos, node->type_type);
		}
	}

	return di_type;

}