#pragma once
#include <map>
#include "lang/ast/type.h"
#include "value.h"
#include "compiler_ctx.h"

using namespace llvm;
using namespace std;

namespace aloe
{
	class type_cache_t
	{
	public:

		virtual DIType* get_dit_type(compiler_ctx_t_ptr ctx, type_node_t_ptr atype);

		virtual Type* get_ir_type(compiler_ctx_t_ptr ctx, type_node_t_ptr node);

		virtual Type* get_layout_ir_type(compiler_ctx_t_ptr ctx, layout_info_t_ptr layout);

	private:

		map<type_node_t, DIType*> type_cache;

		map<type_node_t, Type*> ir_cache;

	};
	defptr(type_cache_t);
}


