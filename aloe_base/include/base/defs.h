#pragma once

#define ALOE_MAX_LOG_LEN 4096

#define PCAST(T, V) (std::static_pointer_cast<T>(V))

#define _new(T,...)  T##_ptr_t (new T##_t(##__VA_ARGS__))

namespace aloe
{
	template <class T>
	struct proxy_t
	{
		proxy_t(T target) : target(target) {}

		T target;
	};

}


