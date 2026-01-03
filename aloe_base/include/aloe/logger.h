#pragma once

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

}

