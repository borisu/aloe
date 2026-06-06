#pragma once
#include <exception>
#include "base/defs.h"

namespace aloe
{
	struct aloe_exception_t : public std::exception {

		aloe_exception_t(const char* format, ...);

		const char* what() const noexcept override;

	private:

		char buffer[ALOE_MAX_LOG_LEN];

	};

}


