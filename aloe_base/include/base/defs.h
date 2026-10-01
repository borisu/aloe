#pragma once

#define ALOE_MAX_LOG_LEN 4096

#define castptr(T, V) (std::static_pointer_cast<T>(V))

#define newptr(T,...)  T##_ptr (new T(##__VA_ARGS__))

#define defptr(T) typedef std::shared_ptr<T> T##_ptr

namespace aloe
{
	template <class T>
	struct proxy_t
	{
		proxy_t(T target) : target(target) {}

		T target;
	};

}


