#pragma once

#define ALOE_MAX_LOG_LEN 4096

#define PCAST(T, V) (std::static_pointer_cast<T>(V))

#define _new(T,...)  T##_ptr_t (new T##_t(##__VA_ARGS__));
