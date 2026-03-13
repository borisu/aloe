#pragma once

#define ALOE_MAX_LOG_LEN 4096

namespace aloe
{
	extern void log1(FILE* stream, const char* text);

	extern void log1nl(FILE* stream);

	enum LOG_LEVEL
	{
		INFO,
		VERBOSE,
		TRACE
	};

	void logf(LOG_LEVEL, const char* format, ...);

	void logi(const char* format, ...);

	void loginl(const char* format, ...);

	char* get_var_string(const char* format, ...);

}

