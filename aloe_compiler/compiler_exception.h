#pragma once
namespace aloe
{
	struct compiler_exeption_t : public std::exception {

		compiler_exeption_t(const char* format, ...);

		const char* what() const noexcept override;

	private:

		char buffer[ALOE_MAX_LOG_LEN];

	};

}


