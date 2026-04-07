#pragma once
namespace aloe
{
	struct parse_exeption_t : public std::exception {

		parse_exeption_t(const char* format, ...);

		char buffer[ALOE_MAX_LOG_LEN];

		virtual const char* what() const noexcept override {
			return buffer;
		}
	};

}

