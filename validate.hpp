#pragma once
#include "ids.hpp"
#include "macro.hpp"
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>

namespace libnbt {
namespace details {
template <::std::integral T>
constexpr T read_integral_unchecked(::std::span<const ::std::byte> buffer,
                                    ::std::endian endian) {
  switch (endian) {
  case ::std::endian::little:
    return static_cast<T>(
        [&]<::std::size_t... Is>(::std::index_sequence<Is...>) {
          return ([&](::std::size_t index) {
            return ::std::to_integer<::std::make_unsigned_t<T>>(buffer[index])
                   << index * 8;
          }(Is) | ...);
        }(::std::make_index_sequence<sizeof(T)>()));
  case ::std::endian::big:
    return static_cast<T>(
        [&]<::std::size_t... Is>(::std::index_sequence<Is...>) {
          return ([&](::std::size_t index) {
            return ::std::to_integer<::std::make_unsigned_t<T>>(
                       buffer[sizeof(T) - index])
                   << index * 8;
          }(Is) | ...);
        }(::std::make_index_sequence<sizeof(T)>()));
  default:
    LIBNBT_UNREACHABLE();
  }
}
} // namespace details
constexpr ::std::size_t invalid_length =
    ::std::numeric_limits<::std::size_t>::max();
namespace details {
constexpr auto validate(::std::byte id, ::std::span<const ::std::byte> buffer,
                        ::std::endian endian) {
  switch (id) {
  case ids::end:
    return static_cast<::std::size_t>(0);
  case ids::i8:
    if (buffer.size() > sizeof(::std::int8_t))
      return sizeof(::std::int8_t);
    else
      return invalid_length;
  case ids::i16:
    if (buffer.size() > sizeof(::std::int16_t))
      return sizeof(::std::int16_t);
    else
      return invalid_length;
  case ids::i32:
    if (buffer.size() > sizeof(::std::int32_t))
      return sizeof(::std::int32_t);
    else
      return invalid_length;
  case ids::i64:
    if (buffer.size() > sizeof(::std::int64_t))
      return sizeof(::std::int64_t);
    else
      return invalid_length;
  case ids::f32:
    if (buffer.size() > sizeof(float))
      return sizeof(float);
    else
      return invalid_length;
  case ids::f64:
    if (buffer.size() > sizeof(double))
      return sizeof(double);
    else
      return invalid_length;
  case ids::i8arr: {
    if (buffer.size() < sizeof(::std::int32_t))
      return invalid_length;
    auto length = read_integral_unchecked<::std::int32_t>(buffer, endian);
    if (buffer.size() < sizeof(::std::int32_t) + length)
      return invalid_length;
    if (length < 0)
      return invalid_length;
    return sizeof(::std::int32_t) + length;
  }
  case ids::str: {
    if (buffer.size() < sizeof(::std::int16_t))
      return invalid_length;
    auto length = read_integral_unchecked<::std::int16_t>(buffer, endian);
    if (buffer.size() < sizeof(::std::int16_t) + length)
      return invalid_length;
    if (length < 0)
      return invalid_length;
    return sizeof(::std::int16_t) + length;
  }
  case ids::arr: {
    if (buffer.size() < sizeof(std::byte) + sizeof(std::int32_t))
      return invalid_length;
    if (::std::to_integer<std::uint8_t>(buffer[0]) == 0)
      return sizeof(std::byte) + sizeof(std::int32_t);
    auto length = read_integral_unchecked<::std::int32_t>(
        buffer.subspan(sizeof(::std::byte)), endian);
    if (length < 0)
      return invalid_length;
    if (::std::to_integer<std::uint8_t>(buffer[0]) <= 6) {
      ::std::size_t size;
      switch (buffer[0]) {
      case ids::i8:
        size = sizeof(::std::int8_t);
        break;
      case ids::i16:
        size = sizeof(::std::int16_t);
        break;
      case ids::i32:
        size = sizeof(::std::int32_t);
        break;
      case ids::i64:
        size = sizeof(::std::int64_t);
        break;
      case ids::f32:
        size = sizeof(float);
        break;
      case ids::f64:
        size = sizeof(double);
        break;
      }
      auto result = sizeof(std::byte) + sizeof(std::int32_t) + length * size;
      if (buffer.size() < result)
        return invalid_length;
      return result;
    }
    auto size = sizeof(std::byte) + sizeof(std::int32_t);
    while (length--) {
      auto result = validate(buffer[0], buffer.subspan(size), endian);
      if (result == invalid_length)
        return invalid_length;
      size += result;
    }
    return size;
  }
  case ids::map: {
    std::size_t size = 0;
    while (size < buffer.size() && buffer[size] != ids::end) {
      if (buffer.size() < size + sizeof(::std::byte) + sizeof(::std::int16_t))
        return invalid_length;
      auto key_length = read_integral_unchecked<::std::int16_t>(
          buffer.subspan(size + sizeof(::std::byte)), endian);
      if (buffer.size() <
          size + sizeof(::std::byte) + sizeof(::std::int16_t) + key_length)
        return invalid_length;
      if (key_length < 0)
        return invalid_length;
      std::size_t value_size = 0;
      if (::std::to_integer<std::uint8_t>(buffer[size]) <= 6) {
        switch (buffer[size]) {
        case ids::i8:
          value_size = sizeof(::std::int8_t);
          break;
        case ids::i16:
          value_size = sizeof(::std::int16_t);
          break;
        case ids::i32:
          value_size = sizeof(::std::int32_t);
          break;
        case ids::i64:
          value_size = sizeof(::std::int64_t);
          break;
        case ids::f32:
          value_size = sizeof(float);
          break;
        case ids::f64:
          value_size = sizeof(double);
          break;
        }
        if (buffer.size() < size + sizeof(::std::byte) +
                                sizeof(::std::int16_t) + key_length +
                                value_size)
          return invalid_length;
        size += sizeof(::std::byte) + sizeof(::std::int16_t) + key_length +
                value_size;
      } else {
        value_size =
            validate(buffer[size],
                     buffer.subspan(size + sizeof(::std::byte) +
                                    sizeof(::std::int16_t) + key_length),
                     endian);
        if (value_size == invalid_length)
          return invalid_length;
        size += sizeof(::std::byte) + sizeof(::std::int16_t) + key_length +
                value_size;
      }
    }
    if (buffer[size] != ids::end)
      return invalid_length;
    return size + 1;
  }
  case ids::i32arr: {
    if (buffer.size() < sizeof(::std::int32_t))
      return invalid_length;
    auto length = read_integral_unchecked<::std::int32_t>(buffer, endian);
    if (buffer.size() < sizeof(::std::int32_t) + length * sizeof(std::int32_t))
      return invalid_length;
    if (length < 0)
      return invalid_length;
    return sizeof(::std::int32_t) + length * sizeof(std::int32_t);
  }
  case ids::i64arr: {
    if (buffer.size() < sizeof(::std::int32_t))
      return invalid_length;
    auto length = read_integral_unchecked<::std::int32_t>(buffer, endian);
    if (buffer.size() < sizeof(::std::int32_t) + length * sizeof(std::int64_t))
      return invalid_length;
    if (length < 0)
      return invalid_length;
    return sizeof(::std::int32_t) + length * sizeof(std::int64_t);
  }
  default:
    return invalid_length;
  }
}
} // namespace details
constexpr auto validate(::std::span<const ::std::byte> buffer,
                        ::std::endian endian) {
  if (buffer.size() < sizeof(::std::byte) + sizeof(::std::int16_t))
    return invalid_length;
  auto root_name_length = details::read_integral_unchecked<::std::int16_t>(
      buffer.subspan(sizeof(::std::byte)), endian);
  if (buffer.size() <
      sizeof(::std::byte) + sizeof(::std::int16_t) + root_name_length)
    return invalid_length;
  return sizeof(::std::byte) + sizeof(::std::int16_t) + root_name_length +
         details::validate(buffer[0],
                           buffer.subspan(sizeof(::std::byte) +
                                          sizeof(::std::int16_t) +
                                          root_name_length),
                           endian);
}
} // namespace libnbt