#include "pch.h"
#include <cstdarg>
#include <cstdio>
#include "lang/aloe_exception.h"

using namespace aloe;


const char* aloe_exception_t::what() const noexcept {
	return buffer;
}

aloe_exception_t::aloe_exception_t(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
}


