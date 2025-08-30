#pragma once
#include "validate.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace libnbt {
struct nbt_base {
  LIBNBT_CONSTEXPR nbt_base() = default;
  virtual LIBNBT_CONSTEXPR ~nbt_base() = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const = 0;
  virtual LIBNBT_CONSTEXPR std::byte id() const = 0;
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const = 0;
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in, std::endian end) = 0;
  LIBNBT_CONSTEXPR std::shared_ptr<nbt_base> clone() const;
  template <typename T> LIBNBT_CONSTEXPR T *as() {
    return static_cast<T *>(this);
  }
  template <typename T> LIBNBT_CONSTEXPR const T *as() const {
    return static_cast<T *>(this);
  }
};
LIBNBT_CONSTEXPR std::shared_ptr<nbt_base> from_id(std::byte id);
struct nbt_none final : nbt_base {
  LIBNBT_CONSTEXPR nbt_none() = default;
  virtual LIBNBT_CONSTEXPR ~nbt_none() override = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const override { return 0; }
  virtual LIBNBT_CONSTEXPR std::byte id() const override {
    return static_cast<std::byte>(0);
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const override {
    return 0;
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in,
                       std::endian end) override {
    return 0;
  }
};
template <std::byte I, typename T,
          typename = std::enable_if_t<std::is_integral_v<T> ||
                                      std::is_floating_point_v<T>>>
struct nbt_number final : nbt_base,
                          std::enable_shared_from_this<nbt_number<I, T>> {
  T data;
  LIBNBT_CONSTEXPR nbt_number() = default;
  LIBNBT_CONSTEXPR nbt_number(T data) : data(data) {}
  virtual LIBNBT_CONSTEXPR ~nbt_number() override = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const override {
    return sizeof(T);
  }
  virtual LIBNBT_CONSTEXPR std::byte id() const override { return I; }
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const override {
    std::array<std::byte, sizeof(T)> bytes =
        std::bit_cast<decltype(bytes)>(data);
    if (end != std::endian::native) {
      std::reverse(bytes.begin(), bytes.end());
    }
    for (std::size_t i = 0; auto byte : bytes) {
      out[i++] = byte;
    }
    return size_bytes();
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in,
                       std::endian end) override {
    if constexpr (std::is_integral_v<T>) {
      data = details::read_integral_unchecked<T>(in, end);
    } else {
      if constexpr (sizeof(T) == sizeof(std::int32_t)) {
        auto tmp = details::read_integral_unchecked<std::uint32_t>(in, end);
        data = std::bit_cast<T>(tmp);
      } else {
        auto tmp = details::read_integral_unchecked<std::uint64_t>(in, end);
        data = std::bit_cast<T>(tmp);
      }
    }
    return sizeof(T);
  }
};
using nbt_i8 = nbt_number<static_cast<std::byte>(1), std::int8_t>;
using nbt_i16 = nbt_number<static_cast<std::byte>(2), std::int16_t>;
using nbt_i32 = nbt_number<static_cast<std::byte>(3), std::int32_t>;
using nbt_i64 = nbt_number<static_cast<std::byte>(4), std::int64_t>;
using nbt_f32 = nbt_number<static_cast<std::byte>(5), float>;
using nbt_f64 = nbt_number<static_cast<std::byte>(6), double>;
template <std::byte I, typename T, typename U>
struct nbt_vec final : nbt_base, std::vector<T> {
  LIBNBT_CONSTEXPR nbt_vec() = default;
  using std::vector<T>::vector;
  virtual LIBNBT_CONSTEXPR ~nbt_vec() = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const override {
    return sizeof(U) + sizeof(T) * std::vector<T>::size();
  }
  virtual LIBNBT_CONSTEXPR std::byte id() const override { return I; }
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const override {
    std::size_t i = 0;
    std::array<std::byte, sizeof(U)> bytes =
        std::bit_cast<decltype(bytes)>(static_cast<U>(std::vector<T>::size()));
    if (end != std::endian::native) {
      std::reverse(bytes.begin(), bytes.end());
    }
    for (auto byte : bytes) {
      out[i++] = byte;
    }
    for (auto element : (*this)) {
      std::array<std::byte, sizeof(T)> bytes =
          std::bit_cast<decltype(bytes)>(element);
      if (end != std::endian::native) {
        std::reverse(bytes.begin(), bytes.end());
      }
      for (auto byte : bytes) {
        out[i++] = byte;
      }
    }
    return size_bytes();
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in,
                       std::endian end) override {
    std::size_t i = 0;
    std::array<std::byte, sizeof(U)> bytes;
    for (auto &byte : bytes) {
      byte = in[i++];
    }
    if (end != std::endian::native) {
      std::reverse(bytes.begin(), bytes.end());
    }
    U sz = std::bit_cast<U>(bytes);
    std::vector<T>::resize(sz);
    for (auto &element : (*this)) {
      std::array<std::byte, sizeof(T)> bytes;
      for (auto &byte : bytes) {
        byte = in[i++];
      }
      if (end != std::endian::native) {
        std::reverse(bytes.begin(), bytes.end());
      }
      element = std::bit_cast<T>(bytes);
    }
    return size_bytes();
  }
};
using nbt_i8arr = nbt_vec<static_cast<std::byte>(7), std::int8_t, std::int32_t>;
struct nbt_str final : nbt_base, std::string {
  LIBNBT_CONSTEXPR nbt_str() = default;
  using std::string::basic_string;
  virtual LIBNBT_CONSTEXPR ~nbt_str() = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const override {
    return sizeof(std::int16_t) + std::string::size();
  }
  virtual LIBNBT_CONSTEXPR std::byte id() const override {
    return static_cast<std::byte>(8);
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const override {
    std::size_t i = 0;
    std::array<std::byte, sizeof(std::int16_t)> bytes =
        std::bit_cast<decltype(bytes)>(
            static_cast<std::int16_t>(std::string::size()));
    if (end != std::endian::native) {
      std::reverse(bytes.begin(), bytes.end());
    }
    for (auto byte : bytes) {
      out[i++] = byte;
    }
    for (auto element : (*this)) {
      out[i++] = static_cast<std::byte>(element);
    }
    return size_bytes();
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in,
                       std::endian end) override {
    std::size_t i = 0;
    std::array<std::byte, sizeof(std::int16_t)> bytes;
    for (auto &byte : bytes) {
      byte = in[i++];
    }
    if (end != std::endian::native) {
      std::reverse(bytes.begin(), bytes.end());
    }
    std::int16_t sz = std::bit_cast<std::int16_t>(bytes);
    std::string::resize(sz);
    for (auto &element : (*this)) {
      element = static_cast<char>(in[i++]);
    }
    return size_bytes();
  }
};
struct nbt_arr final : nbt_base, std::vector<std::shared_ptr<nbt_base>> {
  LIBNBT_CONSTEXPR nbt_arr() = default;
  using std::vector<std::shared_ptr<nbt_base>>::vector;
  virtual LIBNBT_CONSTEXPR ~nbt_arr() override = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const override {
    if (empty() || data()[0]->id() == static_cast<std::byte>(0)) {
      return sizeof(std::byte) + sizeof(std::int32_t);
    }
    if (static_cast<unsigned char>(data()[0]->id()) < 7) {
      return sizeof(std::byte) + sizeof(std::int32_t) +
             data()[0]->size_bytes() *
                 std::vector<std::shared_ptr<nbt_base>>::size();
    }
    std::size_t res = sizeof(std::byte) + sizeof(std::int32_t);
    for (auto &element : (*this)) {
      res += element->size_bytes();
    }
    return res;
  }
  virtual LIBNBT_CONSTEXPR std::byte id() const override {
    return static_cast<std::byte>(9);
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const override {
    auto res = size_bytes();
    nbt_i32 len{static_cast<std::int32_t>(
        std::vector<std::shared_ptr<nbt_base>>::size())};
    if (empty() || data()[0]->id() == static_cast<std::byte>(0)) {
      out[0] = static_cast<std::byte>(0);
      len.data = 0;
      len.to_bytes_unchecked(out.subspan(1), end);
    } else if (static_cast<unsigned char>(data()[0]->id()) < 7) {
      out[0] = data()[0]->id();
      len.to_bytes_unchecked(out.subspan(1), end);
      auto sz = data()[0]->size_bytes();
      for (std::size_t i = sizeof(std::byte) + sizeof(std::int32_t);
           auto &element : (*this)) {
        element->to_bytes_unchecked(out.subspan(i), end);
        i += sz;
      }
    } else {
      out[0] = data()[0]->id();
      len.to_bytes_unchecked(out.subspan(1), end);
      for (std::size_t i = sizeof(std::byte) + sizeof(std::int32_t);
           auto &element : (*this)) {
        element->to_bytes_unchecked(out.subspan(i), end);
        i += element->size_bytes();
      }
    }
    return res;
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in,
                       std::endian end) override {
    std::size_t res = sizeof(std::byte) + sizeof(std::int32_t);
    auto type = in[0];
    if (type == static_cast<std::byte>(0)) {
      std::vector<std::shared_ptr<nbt_base>>::clear();
      return res;
    }
    nbt_i32 len;
    len.from_bytes_unchecked(in.subspan(1), end);
    std::vector<std::shared_ptr<nbt_base>>::resize(len.data);
    if (static_cast<unsigned char>(type) < 7) {
      auto sz = from_id(type)->size_bytes();
      // if (in.size() <
      //     sizeof(std::byte) + sizeof(std::int32_t) + sz * len.data) {
      //   std::vector<std::shared_ptr<nbt_base>>::clear();
      //   return invalid_size;
      // }
      for (auto &element : (*this)) {
        element = from_id(type);
        element->from_bytes_unchecked(in.subspan(res), end);
        res += sz;
      }
    } else {
      for (auto &element : (*this)) {
        element = from_id(type);
        // if (element->from_bytes(in.subspan(res), end) == invalid_size) {
        //   std::vector<std::shared_ptr<nbt_base>>::clear();
        //   return invalid_size;
        // }
        res += element->size_bytes();
      }
    }
    return res;
  }
};
struct nbt_map final
    : nbt_base,
      std::unordered_map<std::string, std::shared_ptr<nbt_base>> {
  LIBNBT_CONSTEXPR nbt_map() = default;
  using std::unordered_map<std::string,
                           std::shared_ptr<nbt_base>>::unordered_map;
  LIBNBT_CONSTEXPR ~nbt_map() override = default;
  virtual LIBNBT_CONSTEXPR std::size_t size_bytes() const override {
    std::size_t res = 0;
    for (auto &[key, val] : (*this)) {
      res += sizeof(std::byte) + sizeof(std::int16_t) + key.size() +
             val->size_bytes();
    }
    res += 1;
    return res;
  }
  virtual LIBNBT_CONSTEXPR std::byte id() const override {
    return static_cast<std::byte>(10);
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  to_bytes_unchecked(std::span<std::byte> out, std::endian end) const override {
    auto sz = size_bytes();
    std::size_t i = 0;
    for (auto &[key, val] : (*this)) {
      out[i++] = val->id();
      nbt_i16 len{static_cast<std::int16_t>(key.size())};
      i += len.to_bytes_unchecked(out.subspan(i), end);
      for (auto c : key)
        out[i++] = static_cast<std::byte>(c);
      i += val->to_bytes_unchecked(out.subspan(i), end);
    }
    out[i++] = static_cast<std::byte>(0);
    return sz;
  }
  virtual LIBNBT_CONSTEXPR std::size_t
  from_bytes_unchecked(std::span<const std::byte> in,
                       std::endian end) override {
    std::size_t i = 0;
    while (i < in.size()) {
      auto type = in[i++];
      if (type == static_cast<std::byte>(0)) {
        break;
      }
      nbt_i16 len;

      i += len.from_bytes_unchecked(in.subspan(i), end);

      std::string key(len.data, '\0');
      // if (i + len.data >= in.size()) {
      //   std::unordered_map<std::string, std::shared_ptr<nbt_base>>::clear();
      //   return invalid_size;
      // }
      for (auto &c : key) {
        c = static_cast<char>(in[i++]);
      }
      auto val = from_id(type);
      i += val->from_bytes_unchecked(in.subspan(i), end);

      std::unordered_map<std::string, std::shared_ptr<nbt_base>>::emplace(key,
                                                                          val);
    }
    return i;
  }
};
using nbt_i32arr =
    nbt_vec<static_cast<std::byte>(11), std::int32_t, std::int32_t>;
using nbt_i64arr =
    nbt_vec<static_cast<std::byte>(12), std::int64_t, std::int32_t>;
LIBNBT_CONSTEXPR std::shared_ptr<nbt_base> from_id(std::byte id) {
  switch (id) {
  case static_cast<std::byte>(0):
    return std::make_shared<nbt_none>();
  case static_cast<std::byte>(1):
    return std::make_shared<nbt_i8>();
  case static_cast<std::byte>(2):
    return std::make_shared<nbt_i16>();
  case static_cast<std::byte>(3):
    return std::make_shared<nbt_i32>();
  case static_cast<std::byte>(4):
    return std::make_shared<nbt_i64>();
  case static_cast<std::byte>(5):
    return std::make_shared<nbt_f32>();
  case static_cast<std::byte>(6):
    return std::make_shared<nbt_f64>();
  case static_cast<std::byte>(7):
    return std::make_shared<nbt_i8arr>();
  case static_cast<std::byte>(8):
    return std::make_shared<nbt_str>();
  case static_cast<std::byte>(9):
    return std::make_shared<nbt_arr>();
  case static_cast<std::byte>(10):
    return std::make_shared<nbt_map>();
  case static_cast<std::byte>(11):
    return std::make_shared<nbt_i32arr>();
  case static_cast<std::byte>(12):
    return std::make_shared<nbt_i64arr>();
  default:
    LIBNBT_UNREACHABLE();
  }
}
LIBNBT_CONSTEXPR std::shared_ptr<nbt_base> nbt_base::clone() const {
  switch (id()) {
  case static_cast<std::byte>(0):
    return std::make_shared<nbt_none>();
  case static_cast<std::byte>(1):
    return std::make_shared<nbt_i8>(static_cast<const nbt_i8 *>(this)->data);
  case static_cast<std::byte>(2):
    return std::make_shared<nbt_i16>(static_cast<const nbt_i16 *>(this)->data);
  case static_cast<std::byte>(3):
    return std::make_shared<nbt_i32>(static_cast<const nbt_i32 *>(this)->data);
  case static_cast<std::byte>(4):
    return std::make_shared<nbt_i64>(static_cast<const nbt_i64 *>(this)->data);
  case static_cast<std::byte>(5):
    return std::make_shared<nbt_f32>(static_cast<const nbt_f32 *>(this)->data);
  case static_cast<std::byte>(6):
    return std::make_shared<nbt_f64>(static_cast<const nbt_f64 *>(this)->data);
  case static_cast<std::byte>(7):
    return std::make_shared<nbt_i8arr>(*static_cast<const nbt_i8arr *>(this));
  case static_cast<std::byte>(8):
    return std::make_shared<nbt_str>(*static_cast<const nbt_str *>(this));
  case static_cast<std::byte>(9): {
    auto res = std::make_shared<nbt_arr>();
    auto self = static_cast<const nbt_arr *>(this);
    res->resize(self->size());
    for (std::size_t i = 0; i < self->size(); i++) {
      (*res)[i] = (*self)[i]->clone();
    }
    return res;
  }
  case static_cast<std::byte>(10): {
    auto res = std::make_shared<nbt_map>();
    auto self = static_cast<const nbt_map *>(this);
    for (auto &[key, val] : *self) {
      res->emplace(key, val->clone());
    }
    return res;
  }
  case static_cast<std::byte>(11):
    return std::make_shared<nbt_i32arr>(*static_cast<const nbt_i32arr *>(this));
  case static_cast<std::byte>(12):
    return std::make_shared<nbt_i64arr>(*static_cast<const nbt_i64arr *>(this));
  default:
    LIBNBT_UNREACHABLE();
  }
}
struct nbt_root {
  std::byte id;
  std::string key;
  std::shared_ptr<nbt_base> val;

  LIBNBT_CONSTEXPR nbt_root() = default;
  LIBNBT_CONSTEXPR nbt_root(std::shared_ptr<nbt_base> val,
                            std::string_view root = "")
      : id(val->id()), key(root), val(val) {}
  LIBNBT_CONSTEXPR std::size_t size_bytes() const {
    return sizeof(std::byte) + sizeof(std::int16_t) + key.size() +
           val->size_bytes();
  }
  LIBNBT_CONSTEXPR std::size_t to_bytes(std::span<std::byte> out,
                                        std::endian end) const {
    auto sz = size_bytes();
    if (out.size() < sz)
      return invalid_length;
    std::size_t i = 0;
    out[i++] = id;
    nbt_i16 len{static_cast<std::int16_t>(key.size())};
    i += len.to_bytes_unchecked(out.subspan(i), end);
    for (auto c : key)
      out[i++] = static_cast<std::byte>(c);
    i += val->to_bytes_unchecked(out.subspan(i), end);
    return sz;
  }
  LIBNBT_CONSTEXPR std::size_t from_bytes(std::span<const std::byte> in,
                                          std::endian end) {
    if (validate(in, end) == invalid_length)
      return invalid_length;
    id = in[0];
    nbt_i16 len;
    len.from_bytes_unchecked(in.subspan(1), end);
    std::string k(len.data, '\0');
    for (auto i = sizeof(std::byte) + sizeof(std::int16_t); auto &c : k)
      c = static_cast<char>(in[i++]);
    key = k;
    val = from_id(id);
    return sizeof(std::byte) + sizeof(std::int16_t) + len.data +
           val->from_bytes_unchecked(
               in.subspan(sizeof(std::byte) + sizeof(std::int16_t) + len.data),
               end);
  }
};
} // namespace libnbt