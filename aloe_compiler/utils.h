#pragma once
#include "llvm/IR/Function.h"
#include "lang/aloe_exception.h"

using namespace llvm;

namespace aloe
{
	template<typename T2, typename T1> 
	T2* ir_sc(T1* ptr)
	{
		if (isa<T2>(ptr))
			return (T2*)(ptr);

		throw aloe_exception_t("internal compiler error: failed to cast IR value to expected type");
			
	}

}
	