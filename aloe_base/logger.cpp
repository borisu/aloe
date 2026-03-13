#include "pch.h"
#include "aloe/logger.h"

using namespace aloe;

static 
void logva(LOG_LEVEL log_level, const char* format, va_list args)
{
	char result[ALOE_MAX_LOG_LEN];
	vsprintf_s(result, format, args);
	log1(stderr, result);
}


void
aloe::logf(LOG_LEVEL log_level, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	logva(log_level, format, args);

	va_end(args);
}

void
aloe::loginl(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	logva(INFO, format, args);
	log1nl(stderr);

	va_end(args);
}

void
aloe::logi(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	logva(INFO, format, args);

	va_end(args);
}

char* 
aloe::get_var_string(const char* format, ...)
{
	static char buffer[ALOE_MAX_LOG_LEN];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	return buffer;
}


