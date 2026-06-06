#pragma once
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "lang/aloe_exception.h"

using namespace llvm;

namespace aloe
{
	template<typename T2, typename T1> 
	T2* ir_sc(T1* ptr)
	{
		if (isa<T2>(ptr))
			return (T2*)(ptr);

		std::string typeStr;

		llvm::raw_string_ostream rso(typeStr);

		ptr->print(rso);

		throw aloe_exception_t("internal compiler error: failed to cast IR value of type %s to expected type", typeStr.c_str());
			
	}

}
	