#include "pch.h"
#include "compiler_exception.h"
using namespace aloe;


const char* compiler_exeption_t::what() const noexcept {
	return buffer;
}

compiler_exeption_t::compiler_exeption_t(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
}


