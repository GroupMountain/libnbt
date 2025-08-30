#pragma once
#include <cstddef>
namespace libnbt {
namespace ids {
constexpr auto end = static_cast<::std::byte>(0);
constexpr auto i8 = static_cast<::std::byte>(1);
constexpr auto i16 = static_cast<::std::byte>(2);
constexpr auto i32 = static_cast<::std::byte>(3);
constexpr auto i64 = static_cast<::std::byte>(4);
constexpr auto f32 = static_cast<::std::byte>(5);
constexpr auto f64 = static_cast<::std::byte>(6);
constexpr auto i8arr = static_cast<::std::byte>(7);
constexpr auto str = static_cast<::std::byte>(8);
constexpr auto arr = static_cast<::std::byte>(9);
constexpr auto map = static_cast<::std::byte>(10);
constexpr auto i32arr = static_cast<::std::byte>(11);
constexpr auto i64arr = static_cast<::std::byte>(12);
} // namespace ids
} // namespace libnbt