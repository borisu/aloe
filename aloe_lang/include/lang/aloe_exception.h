#pragma once
#include <exception>
#include "base/defs.h"

#define RAISE(fmt, ...) \
    throw aloe_exception_t("error: " fmt, ##__VA_ARGS__)

#define ASSERT(C, fmt, ...) \
    if (!(C)) { \
         RAISE(fmt, ##__VA_ARGS__); \
    }

namespace aloe
{
	struct aloe_exception_t : public std::exception {

		aloe_exception_t(const char* format, ...);

		const char* what() const noexcept override;

	private:

		char buffer[ALOE_MAX_LOG_LEN];

	};

}


