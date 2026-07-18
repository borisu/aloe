#include "pch.h"
#include "utils.h"

using namespace aloe;

std::string 
aloe::type_to_str(llvm::Type* Ty)
{
    std::string Str;
    llvm::raw_string_ostream OS(Str);

    Ty->print(OS);
    return OS.str();
}