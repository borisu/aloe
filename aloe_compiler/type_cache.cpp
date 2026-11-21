#include "pch.h"
#include "type_cache.h"
#include  "i64_platform.h"

using namespace aloe;

DIType*
type_cache_t::get_dit_type(compiler_ctx_t_ptr ctx, type_node_t_ptr atype)
{
    switch (atype->type_id)
    {
    case ALOE_TYPE_INT:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {
            type_cache[*atype] = ctx->di_builder()->createBasicType("int", 32, dwarf::DW_ATE_signed);
        }

        break;
    }
    case ALOE_TYPE_CHAR:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {
            type_cache[*atype] = ctx->di_builder()-> createBasicType("char", ALOE_CHAR_SIZE, dwarf::DW_ATE_signed_char);
        }

        break;
    }
    case ALOE_TYPE_VOID:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {
            type_cache[*atype] = nullptr;
        }

        break;
    }
    case ALOE_TYPE_DOUBLE:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {
            type_cache[*atype] = ctx->di_builder()->createBasicType("double", 64, dwarf::DW_ATE_float);
        }
        
        break;
    }
    case ALOE_TYPE_PTR:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {
            type_cache[*atype] = ctx->di_builder()->createPointerType(
				get_dit_type(ctx, atype->ptr->pointee_type()),
                ALOE_PTR_SIZE);
        }

        break;
    }
    case ALOE_TYPE_FUNCTION:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {

            SmallVector<Metadata*, 8>   dit_args;
            dit_args.push_back(get_dit_type(ctx, atype->fun->ret_type()));

            for (auto var : atype->fun->params->v)
            {
                dit_args.push_back(get_dit_type(ctx,var->var_type()));
            };

            type_cache[*atype] = ctx->di_builder()->createSubroutineType(ctx->di_builder()->getOrCreateTypeArray(dit_args));
        }

        break;

    }
    case ALOE_TYPE_ARRAY:
    {
        if (type_cache.find(*atype) == type_cache.end())
        {
            DISubrange* range = ctx->di_builder()->getOrCreateSubrange(0, atype->arr->size);
            DINodeArray subscripts = ctx->di_builder()->getOrCreateArray(range);

			type_cache[*atype] = ctx->di_builder()->createArrayType(
				atype->arr->size,
				ALOE_CHAR_SIZE,
				get_dit_type(ctx, atype->arr->elem_type()),
                subscripts);
        }
        break;
    }
    default:
    {
        assert(false && "(internal compiler error): unknown type id");
    }
    }

    return type_cache[*atype];
}

Type* 
type_cache_t::get_ir_type(compiler_ctx_t_ptr ctx, type_node_t_ptr node)
{
	Type* out = nullptr;

	if (ir_cache.find(*node) != ir_cache.end())
	{
		return ir_cache[*node];
	}

    switch (node->type_id)
    {
    case ALOE_TYPE_INT:
    {
        out = Type::getInt64Ty(*ctx->ctx());
        break;
    }
    case ALOE_TYPE_CHAR:
    {
        out = Type::getInt8Ty(*ctx->ctx());
        break;
    }
    case ALOE_TYPE_VOID:
    {
        out = Type::getVoidTy(*ctx->ctx());
        break;
    }
    case ALOE_TYPE_DOUBLE:
    {
        out = Type::getDoubleTy(*ctx->ctx());
        break;
    }
    case ALOE_TYPE_PTR:
    {
        out = PointerType::getUnqual(*ctx->ctx()); // collapse pointer types, we will preserve the original type in debug info for each level of indirection
        break;
    }
    case ALOE_TYPE_ARRAY:
    {
        Type* arr_type = get_ir_type(ctx, node->arr->elem_type());
        out = ArrayType::get(arr_type, node->arr->size);
        break;
    }
    case ALOE_TYPE_FUNCTION:
    {
        Type* ir_type = get_ir_type(ctx, node->fun->ret_type());

        std::vector<Type*>  irt_args;
        for (auto var : node->fun->params->v)
        {
            Type* argt = get_ir_type(ctx, var->var_type());
            irt_args.push_back(argt);
        };

        out = FunctionType::get(ir_type, irt_args, false);
        break;

    }
    case ALOE_TYPE_LAYOUT:
    {
        out = get_layout_ir_type(ctx, node->layout);
        break;
    }
    default:
    {
        assert(false && "(internal error): unknown type id");
    }
    };

    ir_cache[*node] = out;

    return out;
}

Type*
type_cache_t::get_layout_ir_type(compiler_ctx_t_ptr ctx, layout_info_t_ptr layout)
{
    Type* out = nullptr;

    std::vector<Type*>  irt_members;

    // push gt chain first 

	if (layout->gt_chain)
	{
        for (auto gt : layout->gt_chain->v)
        {
            Type* mt = get_ir_type(ctx, gt->type());
            irt_members.push_back(mt);
        };
		
	}

    for (auto field : layout->fields->v)
    {
        Type* mt = get_ir_type(ctx, field->var_type());
        irt_members.push_back(mt);
    };
    out = StructType::create(*ctx->ctx(), irt_members, layout->long_name);

    return out;
}
