#include "pch.h"
#include "di_cache.h"
#include  "i64_platform.h"

using namespace aloe;

di_cache_t::di_cache_t(DIBuilder& dib) : 
    di_builder(dib)
{

}

DIType*
di_cache_t::get_dit_type(aloe_type_t_ptr atype)
{
    switch (atype->type_id)
    {
    case ALOE_TYPE_INT:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = di_builder.createBasicType("int", 32, dwarf::DW_ATE_signed);
        }

        break;
    }
    case ALOE_TYPE_CHAR:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = di_builder.createBasicType("char", ALOE_CHAR_SIZE, dwarf::DW_ATE_signed_char);
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
            di_cache[*atype] = di_builder.createBasicType("double", 64, dwarf::DW_ATE_float);
        }
        
        break;
    }
    case ALOE_TYPE_PTR:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            di_cache[*atype] = di_builder.createPointerType(
				get_dit_type(atype->ptr_type),
                ALOE_PTR_SIZE);
        }

        break;
    }
    case ALOE_TYPE_FUNCTION:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {

            SmallVector<Metadata*, 8>   dit_args;
            dit_args.push_back(get_dit_type(atype->fun->ret_type));

            for (auto p : atype->fun->params->v)
            {
                dit_args.push_back(get_dit_type(p));
            };

            di_cache[*atype] = di_builder.createSubroutineType(di_builder.getOrCreateTypeArray(dit_args));
        }

        break;

    }
    case ALOE_TYPE_ARRAY:
    {
        if (di_cache.find(*atype) == di_cache.end())
        {
            DISubrange* range = di_builder.getOrCreateSubrange(0, atype->arr_size);
            DINodeArray subscripts = di_builder.getOrCreateArray(range);

			di_cache[*atype] = di_builder.createArrayType(
				atype->arr_size,
				ALOE_CHAR_SIZE,
				get_dit_type(atype->arr_type),
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

