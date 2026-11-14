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
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = ctx->di_builder()->createBasicType("int", 32, dwarf::DW_ATE_signed);
        }

        break;
    }
    case ALOE_TYPE_CHAR:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = ctx->di_builder()-> createBasicType("char", ALOE_CHAR_SIZE, dwarf::DW_ATE_signed_char);
        }

        break;
    }
    case ALOE_TYPE_VOID:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = nullptr;
        }

        break;
    }
    case ALOE_TYPE_DOUBLE:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = ctx->di_builder()->createBasicType("double", 64, dwarf::DW_ATE_float);
        }
        
        break;
    }
    case ALOE_TYPE_PTR:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = ctx->di_builder()->createPointerType(
				get_dit_type(ctx, atype->ptr->pointee_type()),
                ALOE_PTR_SIZE);
        }

        break;
    }
    case ALOE_TYPE_FUNCTION:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {

            SmallVector<Metadata*, 8>   dit_args;
            dit_args.push_back(get_dit_type(ctx, atype->fun->ret_type()));

            for (auto var : atype->fun->params->v)
            {
                dit_args.push_back(get_dit_type(ctx,var->var_type()));
            };

            di_cache[*atype] = ctx->di_builder()->createSubroutineType(ctx->di_builder()->getOrCreateTypeArray(dit_args));
        }

        break;

    }
    case ALOE_TYPE_ARRAY:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            DISubrange* range = ctx->di_builder()->getOrCreateSubrange(0, atype->arr->size);
            DINodeArray subscripts = ctx->di_builder()->getOrCreateArray(range);

			di_cache[*atype] = ctx->di_builder()->createArrayType(
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

    return di_cache[*atype];
}

