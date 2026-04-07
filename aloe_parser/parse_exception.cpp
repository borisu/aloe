#include "pch.h"
#include "parse_exception.h"

using namespace aloe;

aloe::parse_exeption_t::parse_exeption_t(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
}