#pragma once
#include "validate.hpp"
#include <cstdint>
#include <map>
#include <string_view>
#include <variant>
#include <vector>

namespace libnbt {
template <typename StringView, template <typename> typename Span,
          template <typename> typename Vector,
          template <typename, typename> typename Map>
struct basic_nbt_view;

template <typename StringView, template <typename> typename Span,
          template <typename> typename Vector,
          template <typename, typename> typename Map>
using nbt_view_variant = ::std::variant<
    std::monostate, std::int8_t, std::int16_t, std::int32_t, std::int64_t,
    float, double, Span<std::int8_t>, StringView,
    Vector<basic_nbt_view<StringView, Span, Vector, Map>>,
    Map<StringView, basic_nbt_view<StringView, Span, Vector, Map>>,
    Span<std::int32_t>, Span<std::int64_t>>;

template <typename StringView, template <typename> typename Span,
          template <typename> typename Vector,
          template <typename, typename> typename Map>
struct basic_nbt_view : nbt_view_variant<StringView, Span, Vector, Map> {
  using nbt_view_variant<StringView, Span, Vector, Map>::operator=;
  using typename nbt_view_variant<StringView, Span, Vector, Map>::variant;
  std::size_t from_bytes_unchecked(std::byte id, Span<std::byte> buffer,
                                   std::endian endian) {
    switch (id) {
    case ids::end:
      (*this) = std::move(std::monostate{});
      return 0;
    case ids::i8:
      (*this) = std::move(
          details::read_integral_unchecked<std::int8_t>(buffer, endian));
      return sizeof(std::int8_t);
    case ids::i16:
      (*this) = std::move(
          details::read_integral_unchecked<std::int16_t>(buffer, endian));
      return sizeof(std::int16_t);
    case ids::i32:
      (*this) = std::move(
          details::read_integral_unchecked<std::int32_t>(buffer, endian));
      return sizeof(std::int32_t);
    case ids::i64:
      (*this) = std::move(
          details::read_integral_unchecked<std::int64_t>(buffer, endian));
      return sizeof(std::int64_t);
    case ids::f32:
      (*this) = std::move(std::bit_cast<float>(
          details::read_integral_unchecked<std::int32_t>(buffer, endian)));
      return sizeof(float);
    case ids::f64:
      (*this) = std::move(std::bit_cast<double>(
          details::read_integral_unchecked<std::int64_t>(buffer, endian)));
      return sizeof(double);
    case ids::i8arr: {
      const auto size =
          details::read_integral_unchecked<std::int32_t>(buffer, endian);
      (*this) = std::move(Span<std::int8_t>{
          reinterpret_cast<std::int8_t *>(buffer.data() + sizeof(std::int32_t)),
          static_cast<std::size_t>(size)});
      return sizeof(std::int32_t) + size;
    }
    case ids::str: {
      const auto size =
          details::read_integral_unchecked<std::int16_t>(buffer, endian);
      (*this) = std::move(StringView{
          reinterpret_cast<char *>(buffer.data() + sizeof(std::int16_t)),
          static_cast<std::size_t>(size)});
      return sizeof(std::int16_t) + size;
    }
    case ids::arr: {
      const auto id = buffer[0];
      const auto size = details::read_integral_unchecked<std::int32_t>(
          buffer.subspan(1), endian);
      Vector<basic_nbt_view> vec;
      vec.resize(size);
      std::size_t offset = sizeof(std::byte) + sizeof(std::int32_t);
      for (auto &i : vec)
        offset += i.from_bytes_unchecked(id, buffer.subspan(offset), endian);
      (*this) = std::move(vec);
      return offset;
    }
    case ids::map: {
      std::size_t offset = 0;
      Map<StringView, basic_nbt_view> map;
      while (buffer[offset] != ids::end) {
        const auto id = buffer[offset];
        const auto key_size = details::read_integral_unchecked<std::int16_t>(
            buffer.subspan(offset + sizeof(std::byte)), endian);
        const auto key = StringView{
            reinterpret_cast<char *>(buffer.data() + offset +
                                     sizeof(std::byte) + sizeof(std::int16_t)),
            static_cast<std::size_t>(key_size)};
        basic_nbt_view val;
        const auto val_size = val.from_bytes_unchecked(
            id,
            buffer.subspan(offset + sizeof(std::byte) + sizeof(std::int16_t) +
                           key_size),
            endian);
        map.emplace(key, std::move(val));
        offset +=
            sizeof(std::byte) + sizeof(std::int16_t) + key_size + val_size;
      }
      (*this) = std::move(map);
      return offset + sizeof(std::byte);
    }
    case ids::i32arr: {
      const auto size =
          details::read_integral_unchecked<std::int32_t>(buffer, endian);
      (*this) = std::move(
          Span<std::int32_t>{reinterpret_cast<std::int32_t *>(
                                 buffer.data() + sizeof(std::int32_t)),
                             static_cast<std::size_t>(size)});
      return sizeof(std::int32_t) + size * sizeof(std::int32_t);
    }
    case ids::i64arr: {
      const auto size =
          details::read_integral_unchecked<std::int32_t>(buffer, endian);
      (*this) = std::move(
          Span<std::int64_t>{reinterpret_cast<std::int64_t *>(
                                 buffer.data() + sizeof(std::int32_t)),
                             static_cast<std::size_t>(size)});
      return sizeof(std::int32_t) + size * sizeof(std::int64_t);
    }
    default:
      LIBNBT_UNREACHABLE();
    }
  }
};
template <typename StringView, template <typename> typename Span,
          template <typename> typename Vector,
          template <typename, typename> typename Map>
struct basic_nbt_view_root {
  using end = std::monostate;
  using i8 = std::int8_t;
  using i16 = std::int16_t;
  using i32 = std::int32_t;
  using i64 = std::int64_t;
  using f32 = float;
  using f64 = double;
  using i8arr = Span<std::int8_t>;
  using str = StringView;
  using arr = Vector<basic_nbt_view<StringView, Span, Vector, Map>>;
  using map = Map<StringView, basic_nbt_view<StringView, Span, Vector, Map>>;
  using i32arr = Span<std::int32_t>;
  using i64arr = Span<std::int64_t>;

  StringView root_name;
  basic_nbt_view<StringView, Span, Vector, Map> value;
  std::size_t from_bytes(Span<std::byte> buffer, std::endian endian) {
    if (auto res = validate(buffer, endian); res == invalid_length)
      return invalid_length;
    const auto id = buffer[0];
    const auto key_size = details::read_integral_unchecked<std::int16_t>(
        buffer.subspan(1), endian);
    const auto key =
        StringView{reinterpret_cast<char *>(buffer.data() + sizeof(std::byte) +
                                            sizeof(std::int16_t)),
                   static_cast<std::size_t>(key_size)};
    return sizeof(std::byte) + sizeof(std::int16_t) + key_size +
           value.from_bytes_unchecked(id,
                                      buffer.subspan(sizeof(std::byte) +
                                                     sizeof(std::int16_t) +
                                                     key_size),
                                      endian);
  }
};
using nbt_view_root =
    basic_nbt_view_root<std::string_view, std::span, std::vector, std::map>;
} // namespace libnbt