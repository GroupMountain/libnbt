#pragma once
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202207L
#define LIBNBT_CONSTEXPR constexpr
#else
#define LIBNBT_CONSTEXPR
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#include <utility>
#define LIBNBT_UNREACHABLE() std::unreachable()
#elif defined(_MSC_VER) && !defined(__clang__)
#define LIBNBT_UNREACHABLE() __assume(false)
#else
#define LIBNBT_UNREACHABLE() __builtin_unreachable()
#endif