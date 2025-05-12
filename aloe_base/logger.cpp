#include "pch.h"
#include "aloe/logger.h"

using namespace aloe;

#define MAX_LOG_LEN 1024

static 
void logva(LOG_LEVEL log_level, const char* format, va_list args)
{
	char result[MAX_LOG_LEN];
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
aloe::logi(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	logva(INFO, format, args);

	va_end(args);
}


