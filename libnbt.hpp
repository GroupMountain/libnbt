#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace libnbt {
enum class nbt_type : ::std::int8_t {
  end,
  i8,
  i16,
  i32,
  i64,
  f32,
  f64,
  i8arr,
  str,
  arr,
  map,
  i32arr,
  i64arr
};
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
struct nbt_t {
  struct ptr_holder_t {
    nbt_t *data;
    constexpr ptr_holder_t();
    constexpr ptr_holder_t(const nbt_t &);
    constexpr ptr_holder_t(nbt_t &&);
    constexpr ptr_holder_t(const ptr_holder_t &);
    constexpr ~ptr_holder_t();
    constexpr nbt_t &operator*();
    constexpr const nbt_t &operator*() const;
    constexpr ptr_holder_t &operator=(const nbt_t &);
    constexpr ptr_holder_t &operator=(nbt_t &&);
    constexpr ptr_holder_t &operator=(const ptr_holder_t &);
    template <typename T> constexpr T &as() { return data->as<T>(); }
    template <typename T> constexpr const T &as() const {
      return data->as<T>();
    }
    template <typename T> constexpr T *as_if() { return data->as_if<T>(); }
    template <typename T> constexpr const T *as_if() const {
      return data->as_if<T>();
    }
    template <typename T> constexpr bool is() const { return data->is<T>(); }
  };
  using end_t = ::std::nullptr_t;
  using i8_t = ::std::int8_t;
  using i16_t = ::std::int16_t;
  using i32_t = ::std::int32_t;
  using i64_t = ::std::int64_t;
  using f32_t = float;
  using f64_t = double;
  using i8arr_t = Vec<i8_t>;
  using str_t = Str;
  using arr_t = Vec<ptr_holder_t>;
  using map_t = Map<str_t, ptr_holder_t>;
  using i32arr_t = Vec<i32_t>;
  using i64arr_t = Vec<i64_t>;
  template <typename T> constexpr static ::libnbt::nbt_type type_to_enum() {
    using enum ::libnbt::nbt_type;
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (requires() {                                                   \
                  {                                                            \
                    U##_t { ::std::declval<T>() }                              \
                  };                                                           \
                })                                                             \
  return U
    IF_CONSTEXPR(end);
    else IF_CONSTEXPR(i8);
    else IF_CONSTEXPR(i16);
    else IF_CONSTEXPR(i32);
    else IF_CONSTEXPR(i64);
    else IF_CONSTEXPR(f32);
    else IF_CONSTEXPR(f64);
    else IF_CONSTEXPR(i8arr);
    else IF_CONSTEXPR(str);
    else IF_CONSTEXPR(arr);
    else IF_CONSTEXPR(map);
    else IF_CONSTEXPR(i32arr);
    else IF_CONSTEXPR(i64arr);
    else static_assert(false, "unknown type.");
#pragma pop_macro("IF_CONSTEXPR")
  }
  ::libnbt::nbt_type type{::libnbt::nbt_type::end};
  union val_t {
    end_t end;
    i8_t i8;
    i16_t i16;
    i32_t i32;
    i64_t i64;
    f32_t f32;
    f64_t f64;
    i8arr_t i8arr;
    str_t str;
    arr_t arr;
    map_t map;
    i32arr_t i32arr;
    i64arr_t i64arr;
    constexpr ~val_t() {}
  } val{nullptr};
  constexpr ~nbt_t() {
    using enum ::libnbt::nbt_type;
    switch (type) {
    case i8arr:
      val.i8arr.~i8arr_t();
      break;
    case str:
      val.str.~str_t();
      break;
    case arr:
      val.arr.~arr_t();
      break;
    case map:
      val.map.~map_t();
      break;
    case i32arr:
      val.i32arr.~i32arr_t();
      break;
    case i64arr:
      val.i64arr.~i64arr_t();
      break;
    default:
      break;
    }
  }
  template <typename T> constexpr nbt_t(T &&val) {
    if constexpr (std::same_as<nbt_t, std::remove_cvref_t<T>>) {
      type = val.type;
#pragma push_macro("CASE")
#undef CASE
#define CASE(U)                                                                \
  case U:                                                                      \
    new (&this->val) U##_t{std::forward<decltype(val.val.U)>(val.val.U)};      \
    break
      using enum ::libnbt::nbt_type;
      switch (type) {
        CASE(end);
        CASE(i8);
        CASE(i16);
        CASE(i32);
        CASE(i64);
        CASE(f32);
        CASE(f64);
        CASE(i8arr);
        CASE(str);
        CASE(arr);
        CASE(map);
        CASE(i32arr);
        CASE(i64arr);
      }
#pragma pop_macro("CASE")
    } else {
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (requires() {                                                   \
                  {                                                            \
                    U##_t { ::std::declval<T>() }                              \
                  };                                                           \
                })                                                             \
  new (&this->val) U##_t{std::forward<U##_t>(val)}
      type = type_to_enum<std::remove_cvref_t<T>>();
      IF_CONSTEXPR(end);
      else IF_CONSTEXPR(i8);
      else IF_CONSTEXPR(i16);
      else IF_CONSTEXPR(i32);
      else IF_CONSTEXPR(i64);
      else IF_CONSTEXPR(f32);
      else IF_CONSTEXPR(f64);
      else IF_CONSTEXPR(i8arr);
      else IF_CONSTEXPR(str);
      else IF_CONSTEXPR(arr);
      else IF_CONSTEXPR(map);
      else IF_CONSTEXPR(i32arr);
      else IF_CONSTEXPR(i64arr);
      else static_assert(false, "unknown type.");
#pragma pop_macro("IF_CONSTEXPR")
    }
  }
  constexpr nbt_t() : type(nbt_type::end), val(nullptr) {}
  template <typename T> constexpr nbt_t &operator=(T &&val) {
    if constexpr (std::same_as<nbt_t, std::remove_cvref_t<T>>) {
      type = val.type;
#pragma push_macro("CASE")
#undef CASE
#define CASE(U)                                                                \
  case U:                                                                      \
    new (&this->val) U##_t{std::forward<decltype(val.val.U)>(val.val.U)};      \
    break
      using enum ::libnbt::nbt_type;
      switch (type) {
        CASE(end);
        CASE(i8);
        CASE(i16);
        CASE(i32);
        CASE(i64);
        CASE(f32);
        CASE(f64);
        CASE(i8arr);
        CASE(str);
        CASE(arr);
        CASE(map);
        CASE(i32arr);
        CASE(i64arr);
      }
#pragma pop_macro("CASE")
    } else {
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (requires() {                                                   \
                  {                                                            \
                    U##_t { ::std::declval<T>() }                              \
                  };                                                           \
                })                                                             \
  new (&this->val) U##_t{std::forward<U##_t>(val)}
      type = type_to_enum<std::remove_cvref_t<T>>();
      IF_CONSTEXPR(end);
      else IF_CONSTEXPR(i8);
      else IF_CONSTEXPR(i16);
      else IF_CONSTEXPR(i32);
      else IF_CONSTEXPR(i64);
      else IF_CONSTEXPR(f32);
      else IF_CONSTEXPR(f64);
      else IF_CONSTEXPR(i8arr);
      else IF_CONSTEXPR(str);
      else IF_CONSTEXPR(arr);
      else IF_CONSTEXPR(map);
      else IF_CONSTEXPR(i32arr);
      else IF_CONSTEXPR(i64arr);
      else static_assert(false, "unknown type.");
#pragma pop_macro("IF_CONSTEXPR")
    }
    return *this;
  }
  template <typename T> constexpr T &as() {
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (std::same_as<T, U##_t>) {                                      \
    if (type != type_to_enum<T>()) {                                           \
      throw std::runtime_error("type mismatch.");                              \
    }                                                                          \
    return val.U;                                                              \
  }
    // clang-format off
    IF_CONSTEXPR(end)
    else IF_CONSTEXPR(i8)
    else IF_CONSTEXPR(i16)
    else IF_CONSTEXPR(i32)
    else IF_CONSTEXPR(i64)
    else IF_CONSTEXPR(f32)
    else IF_CONSTEXPR(f64)
    else IF_CONSTEXPR(i8arr)
    else IF_CONSTEXPR(str)
    else IF_CONSTEXPR(arr)
    else IF_CONSTEXPR(map)
    else IF_CONSTEXPR(i32arr)
    else IF_CONSTEXPR(i64arr)
    else static_assert(false, "unknown type.");
    // clang-format on
#pragma pop_macro("IF_CONSTEXPR")
  }
  template <typename T> constexpr const T &as() const {
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (std::same_as<T, U##_t>) {                                      \
    if (type != type_to_enum<T>()) {                                           \
      throw std::runtime_error("type mismatch.");                              \
    }                                                                          \
    return val.U;                                                              \
  }
    // clang-format off
        IF_CONSTEXPR(end)
        else IF_CONSTEXPR(i8)
        else IF_CONSTEXPR(i16)
        else IF_CONSTEXPR(i32)
        else IF_CONSTEXPR(i64)
        else IF_CONSTEXPR(f32)
        else IF_CONSTEXPR(f64)
        else IF_CONSTEXPR(i8arr)
        else IF_CONSTEXPR(str)
        else IF_CONSTEXPR(arr)
        else IF_CONSTEXPR(map)
        else IF_CONSTEXPR(i32arr)
        else IF_CONSTEXPR(i64arr)
        else static_assert(false, "unknown type.");
    // clang-format on
#pragma pop_macro("IF_CONSTEXPR")
  }
  template <typename T> constexpr T *as_if() {
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (std::same_as<T, U##_t>) {                                      \
    if (type != type_to_enum<T>()) {                                           \
      return nullptr;                                                          \
    }                                                                          \
    return &val.U;                                                             \
  }
    // clang-format off
        IF_CONSTEXPR(end)
        else IF_CONSTEXPR(i8)
        else IF_CONSTEXPR(i16)
        else IF_CONSTEXPR(i32)
        else IF_CONSTEXPR(i64)
        else IF_CONSTEXPR(f32)
        else IF_CONSTEXPR(f64)
        else IF_CONSTEXPR(i8arr)
        else IF_CONSTEXPR(str)
        else IF_CONSTEXPR(arr)
        else IF_CONSTEXPR(map)
        else IF_CONSTEXPR(i32arr)
        else IF_CONSTEXPR(i64arr)
        else static_assert(false, "unknown type.");
    // clang-format on
#pragma pop_macro("IF_CONSTEXPR")
  }
  template <typename T> constexpr const T *as_if() const {
#pragma push_macro("IF_CONSTEXPR")
#undef IF_CONSTEXPR
#define IF_CONSTEXPR(U)                                                        \
  if constexpr (std::same_as<T, U##_t>) {                                      \
    if (type != type_to_enum<T>()) {                                           \
      return nullptr;                                                          \
    }                                                                          \
    return &val.U;                                                             \
  }
    // clang-format off
            IF_CONSTEXPR(end)
            else IF_CONSTEXPR(i8)
            else IF_CONSTEXPR(i16)
            else IF_CONSTEXPR(i32)
            else IF_CONSTEXPR(i64)
            else IF_CONSTEXPR(f32)
            else IF_CONSTEXPR(f64)
            else IF_CONSTEXPR(i8arr)
            else IF_CONSTEXPR(str)
            else IF_CONSTEXPR(arr)
            else IF_CONSTEXPR(map)
            else IF_CONSTEXPR(i32arr)
            else IF_CONSTEXPR(i64arr)
            else static_assert(false, "unknown type.");
    // clang-format on
#pragma pop_macro("IF_CONSTEXPR")
  }
  template <typename T> constexpr const bool is() const {
    return as_if<T>() != nullptr;
  }
};
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t::ptr_holder_t() {
  data = new nbt_t<Str, Vec, Map>();
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t::~ptr_holder_t() {
  delete data;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t::ptr_holder_t(
    nbt_t<Str, Vec, Map> &&nbt) {
  data = new nbt_t<Str, Vec, Map>();
  *data = std::move(nbt);
}

template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t::ptr_holder_t(
    const nbt_t<Str, Vec, Map> &nbt) {
  data = new nbt_t<Str, Vec, Map>();
  *data = nbt;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map> &
nbt_t<Str, Vec, Map>::ptr_holder_t::operator*() {
  return *data;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr const nbt_t<Str, Vec, Map> &
nbt_t<Str, Vec, Map>::ptr_holder_t::operator*() const {
  return *data;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t &
nbt_t<Str, Vec, Map>::ptr_holder_t::operator=(const nbt_t<Str, Vec, Map> &nbt) {
  *data = nbt_t<Str, Vec, Map>(nbt);
  return *this;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t &
nbt_t<Str, Vec, Map>::ptr_holder_t::operator=(nbt_t<Str, Vec, Map> &&nbt) {
  *data = nbt_t<Str, Vec, Map>(std::move(nbt));
  return *this;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t::ptr_holder_t(
    const ptr_holder_t &nbt) {
  data = new nbt_t<Str, Vec, Map>();
  *data = *nbt.data;
}
template <typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr nbt_t<Str, Vec, Map>::ptr_holder_t &
nbt_t<Str, Vec, Map>::ptr_holder_t::operator=(const ptr_holder_t &nbt) {
  *data = *nbt.data;
  return *this;
}

namespace binary {
namespace details {
template <typename T>
concept IsDirectlyCopyable = requires(T a) {
  { a.data() } -> ::std::same_as<typename T::value_type>;
};

template <typename T> struct copy_helper {
  constexpr static void copy(const typename T::value_type *data,
                             ::std::size_t size, T &res) {
    if (size < sizeof(typename T::value_type) * res.size()) [[unlikely]]
      throw ::std::out_of_range("out of range.");
    for (::std::size_t i = 0; i < res.size(); i++)
      res[i] = data[i];
  }
  constexpr static void copy(const unsigned char *data, ::std::size_t size,
                             T &res) {
    if (size < sizeof(typename T::value_type) * res.size()) [[unlikely]]
      throw ::std::out_of_range("out of range.");
    ::std::array<unsigned char, sizeof(typename T::value_type)> buffer;
    for (::std::size_t i = 0; i < res.size(); i++) {
      const auto begin_address = i * sizeof(typename T::value_type);
      for (::std::size_t j = 0; j < sizeof(typename T::value_type); j++)
        buffer[j] = data[begin_address + j];
      res[i] = ::std::bit_cast<typename T::value_type>(buffer);
    }
  }
};

template <::libnbt::binary::details::IsDirectlyCopyable T>
struct copy_helper<T> {
  constexpr static void copy(const typename T::value_type *data,
                             ::std::size_t size, T &res) {
    if (size < sizeof(typename T::value_type) * res.size()) [[unlikely]]
      throw ::std::out_of_range("out of range.");
    if consteval {
      for (::std::size_t i = 0; i < res.size(); i++)
        res[i] = data[i];
    } else {
      ::std::memcpy(res.data(), data,
                    sizeof(typename T::value_type) * res.size());
    }
  }
  constexpr static void copy(const unsigned char *data, ::std::size_t size,
                             T &res) {
    if (size < sizeof(typename T::value_type) * res.size()) [[unlikely]]
      throw ::std::out_of_range("out of range.");
    if consteval {
      ::std::array<unsigned char, sizeof(typename T::value_type)> buffer;
      for (::std::size_t i = 0; i < res.size(); i++) {
        const auto begin_address = i * sizeof(typename T::value_type);
        for (::std::size_t j = 0; j < sizeof(typename T::value_type); j++)
          buffer[j] = data[begin_address + j];
        res[i] = ::std::bit_cast<typename T::value_type>(buffer);
      }
    } else {
      ::std::memcpy(res.data(), data,
                    sizeof(typename T::value_type) * res.size());
    }
  }
};

template <::std::endian E, typename T> struct read_number;
template <::std::endian E, ::std::integral T> struct read_number<E, T> {
  constexpr static T read(const unsigned char *buffer, ::std::size_t size) {
    if (size >= sizeof(T)) [[likely]] {
      ::std::array<unsigned char, sizeof(T)> value;
      if consteval {
        for (::std::size_t i = 0; i < sizeof(T); i++)
          value[i] = buffer[i];
      } else {
        ::std::memcpy(value.data(), buffer, sizeof(T));
      }
      if (E == ::std::endian::native)
        return ::std::bit_cast<T>(value);
      else
        return ::std::byteswap(::std::bit_cast<T>(value));
    }
    throw ::std::out_of_range("out of range.");
  }
};
template <::std::endian E, ::std::floating_point T> struct read_number<E, T> {
  constexpr static T read(const unsigned char *buffer, ::std::size_t size) {
    if (size >= sizeof(T)) [[likely]] {
      ::std::array<unsigned char, sizeof(T)> value;
      if consteval {
        for (::std::size_t i = 0; i < sizeof(T); i++)
          value[i] = buffer[i];
      } else {
        ::std::memcpy(value.data(), buffer, sizeof(T));
      }
      if (E == ::std::endian::native)
        return ::std::bit_cast<T>(value);
      else {
        ::std::reverse(value.begin(), value.end());
        return ::std::bit_cast<T>(value);
      }
    }
    throw ::std::out_of_range("out of range.");
  }
};

template <::std::endian E, typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr ::std::size_t
read_no_type(::libnbt::nbt_type type, nbt_t<Str, Vec, Map> &nbt,
             const unsigned char *buffer, ::std::size_t size) {
  using enum ::libnbt::nbt_type;
  switch (type) {
  case end:
    break;
#pragma push_macro("CASE")
#undef CASE
#define CASE(U)                                                                \
  case U: {                                                                    \
    typename nbt_t<Str, Vec, Map>::U##_t value;                                \
    value = read_number<E, typename nbt_t<Str, Vec, Map>::U##_t>::read(buffer, \
                                                                       size);  \
    nbt = value;                                                               \
  }                                                                            \
    return sizeof(typename nbt_t<Str, Vec, Map>::U##_t)
    CASE(i8);
    CASE(i16);
    CASE(i32);
    CASE(i64);
    CASE(f32);
    CASE(f64);
#undef CASE
#define CASE(U)                                                                \
  case U: {                                                                    \
    ::std::int32_t length =                                                    \
        read_number<E, ::std::int32_t>::read(buffer, size);                    \
    buffer += sizeof(::std::int32_t);                                          \
    if (size < length) [[unlikely]]                                            \
      throw ::std::out_of_range("out of range.");                              \
    size -= sizeof(::std::int32_t);                                            \
    typename nbt_t<Str, Vec, Map>::U##_t value;                                \
    value.resize(length);                                                      \
    copy_helper<typename nbt_t<Str, Vec, Map>::U##_t>::copy(buffer, length,    \
                                                            value);            \
    nbt = value;                                                               \
    return length * sizeof(typename nbt_t<Str, Vec, Map>::U##_t);              \
  }
    CASE(i8arr)
    CASE(i32arr)
    CASE(i64arr)
#undef CASE
#pragma pop_macro("CASE")
  case arr: {
    ::libnbt::nbt_type type = ::std::bit_cast<::libnbt::nbt_type>(buffer[0]);
    buffer++;
    size--;
    ::std::int32_t length = read_number<E, ::std::int32_t>::read(buffer, size);
    buffer += sizeof(::std::int32_t);
    size -= sizeof(::std::int32_t);
    ::std::size_t res_size = 1 + sizeof(::std::int32_t);
    typename nbt_t<Str, Vec, Map>::arr_t value;
    value.resize(length);
    for (auto &nbt : value) {
      auto element_size =
          ::libnbt::binary::details::read_no_type<E>(type, *nbt, buffer, size);
      buffer += element_size;
      size -= element_size;
      res_size += element_size;
    }
    return res_size;
  }
  case str: {
    ::std::int16_t length = read_number<E, ::std::int16_t>::read(buffer, size);
    buffer += sizeof(::std::int16_t);
    size -= sizeof(::std::int16_t);
    ::std::size_t res_size = sizeof(::std::int16_t);
    typename nbt_t<Str, Vec, Map>::str_t value;
    value.resize(length);
    copy_helper<typename nbt_t<Str, Vec, Map>::str_t>::copy(buffer, size,
                                                            value);
    res_size += length;
    return res_size;
  }
  case map: {
    ::libnbt::nbt_type type = ::std::bit_cast<::libnbt::nbt_type>(buffer[0]);
    buffer++;
    size--;
    ::std::size_t res_size = 1;
    typename nbt_t<Str, Vec, Map>::map_t value;
    while (type != end) {
      ::std::int16_t length =
          read_number<E, ::std::int16_t>::read(buffer, size);
      buffer += sizeof(::std::int16_t);
      size -= sizeof(::std::int16_t);
      res_size += sizeof(::std::int16_t);
      typename nbt_t<Str, Vec, Map>::str_t key;
      key.resize(length);
      copy_helper<typename nbt_t<Str, Vec, Map>::str_t>::copy(buffer, length,
                                                              key);
      buffer += length;
      if (size < length) [[unlikely]]
        throw ::std::out_of_range("out of range.");
      size -= length;
      res_size += length;
      nbt_t<Str, Vec, Map> val;
      auto element_size =
          ::libnbt::binary::details::read_no_type<E>(type, val, buffer, size);
      buffer += element_size;
      size -= element_size;
      res_size += element_size;
      value.emplace(key, val);
      type = ::std::bit_cast<::libnbt::nbt_type>(buffer[0]);
    }
    res_size++;
    return res_size;
  }
  }
}
} // namespace details
template <::std::endian E, typename Str, template <typename> typename Vec,
          template <typename, typename> typename Map>
constexpr Str read(nbt_t<Str, Vec, Map> &nbt, const unsigned char *buffer,
                   ::std::size_t size) {
  if (size >= 1) [[likely]] {
    auto type = ::std::bit_cast<::libnbt::nbt_type>(buffer[0]);
    nbt_t<Str, Vec, Map> tmp;
    ::libnbt::binary::details::read_no_type<E>(::libnbt::nbt_type::str, tmp,
                                               buffer + 1, size - 1);
    auto name = tmp.template as<Str>();
    ::libnbt::binary::details::read_no_type<E>(
        type, nbt, buffer + 1 + name.size(), size - 1 - name.size());
    return name;
  }
  throw ::std::out_of_range("out of range.");
}
}; // namespace binary
} // namespace libnbt
