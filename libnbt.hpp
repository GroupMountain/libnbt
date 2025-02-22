#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
namespace libnbt {
#define THROW(...) throw __VA_ARGS__
enum class nbt_type : std::int8_t {
  end,
  int8,
  int16,
  int32,
  int64,
  float32,
  float64,
  int8list,
  string,
  list,
  compound,
  int32list,
  int64list
};

template <template <typename, typename> typename Map,
          template <typename> typename Array, typename String>
struct nbt_node {
  template <typename T> using array_t = Array<T>;
  template <typename K, typename V> using map_t = Map<K, V>;
  using string_t = String;
  using int8list_t = array_t<std::int8_t>;
  using int32list_t = array_t<std::int32_t>;
  using int64list_t = array_t<std::int64_t>;
  using list_t = array_t<nbt_node>;
  using compound_t = map_t<string_t, nbt_node>;
  template <typename T>
  constexpr static auto type_to_nbt_type = [] { static_assert(false); }();
  template <> constexpr auto type_to_nbt_type<std::nullptr_t> = nbt_type::end;
  template <> constexpr auto type_to_nbt_type<std::int8_t> = nbt_type::int8;
  template <> constexpr auto type_to_nbt_type<std::int16_t> = nbt_type::int16;
  template <> constexpr auto type_to_nbt_type<std::int32_t> = nbt_type::int32;
  template <> constexpr auto type_to_nbt_type<std::int64_t> = nbt_type::int64;
  template <> constexpr auto type_to_nbt_type<float> = nbt_type::float32;
  template <> constexpr auto type_to_nbt_type<double> = nbt_type::float64;
  template <>
  constexpr auto type_to_nbt_type<Array<std::int8_t>> = nbt_type::int8list;
  template <> constexpr auto type_to_nbt_type<String> = nbt_type::string;
  template <> constexpr auto type_to_nbt_type<Array<nbt_node>> = nbt_type::list;
  template <>
  constexpr auto type_to_nbt_type<Map<String, nbt_node>> = nbt_type::compound;
  template <>
  constexpr auto type_to_nbt_type<Array<std::int32_t>> = nbt_type::int32list;
  template <>
  constexpr auto type_to_nbt_type<Array<std::int64_t>> = nbt_type::int64list;
  nbt_type type;
  union {
    std::nullptr_t end;
    std::int8_t int8;
    std::int16_t int16;
    std::int32_t int32;
    std::int64_t int64;
    float float32;
    double float64;
    Array<std::int8_t> *int8list;
    String *string;
    Array<nbt_node> *list;
    Map<String, nbt_node> *compound;
    Array<std::int32_t> *int32list;
    Array<std::int64_t> *int64list;
  };
  constexpr ~nbt_node() {
    switch (type) {
    case nbt_type::int8list:
      delete int8list;
      break;
    case nbt_type::string:
      delete string;
      break;
    case nbt_type::list:
      delete list;
      break;
    case nbt_type::compound:
      delete compound;
      break;
    case nbt_type::int32list:
      delete int32list;
      break;
    case nbt_type::int64list:
      delete int64list;
      break;
    default:
      break;
    }
  }
  constexpr nbt_node(const nbt_node &other) {
    type = other.type;
    switch (type) {
    case nbt_type::end:
      end = other.end;
      break;
    case nbt_type::int8:
      int8 = other.int8;
      break;
    case nbt_type::int16:
      int16 = other.int16;
      break;
    case nbt_type::int32:
      int32 = other.int32;
      break;
    case nbt_type::int64:
      int64 = other.int64;
      break;
    case nbt_type::float32:
      float32 = other.float32;
      break;
    case nbt_type::float64:
      float64 = other.float64;
      break;
    case nbt_type::int8list:
      int8list = new Array<std::int8_t>(*other.int8list);
      break;
    case nbt_type::string:
      string = new String(*other.string);
      break;
    case nbt_type::list:
      list = new Array<nbt_node>(*other.list);
      break;
    case nbt_type::compound:
      compound = new Map<String, nbt_node>(*other.compound);
      break;
    case nbt_type::int32list:
      int32list = new Array<std::int32_t>(*other.int32list);
      break;
    case nbt_type::int64list:
      int64list = new Array<std::int64_t>(*other.int64list);
      break;
    }
  }
  constexpr nbt_node(std::nullptr_t end = nullptr)
      : end(end), type(nbt_type::end) {}
  constexpr nbt_node(std::int8_t int8) : int8(int8), type(nbt_type::int8) {}
  constexpr nbt_node(std::int16_t int16)
      : int16(int16), type(nbt_type::int16) {}
  constexpr nbt_node(std::int32_t int32)
      : int32(int32), type(nbt_type::int32) {}
  constexpr nbt_node(std::int64_t int64)
      : int64(int64), type(nbt_type::int64) {}
  constexpr nbt_node(float float32)
      : float32(float32), type(nbt_type::float32) {}
  constexpr nbt_node(double float64)
      : float64(float64), type(nbt_type::float64) {}
  constexpr nbt_node(Array<std::int8_t> &&int8list)
      : int8list(new Array<std::int8_t>(std::forward(int8list))),
        type(nbt_type::int8list) {}
  constexpr nbt_node(String &&string)
      : string(new String(std::forward<String &&>(string))),
        type(nbt_type::string) {}
  constexpr nbt_node(Array<nbt_node> &&list)
      : list(new Array<nbt_node>(std::forward<Array<nbt_node> &&>(list))),
        type(nbt_type::list) {}
  constexpr nbt_node(Map<String, nbt_node> &&compound)
      : compound(new Map<String, nbt_node>(
            std::forward<Map<String, nbt_node> &&>(compound))),
        type(nbt_type::compound) {}
  constexpr nbt_node(Array<std::int32_t> &&int32list)
      : int32list(new Array<std::int32_t>(
            std::forward<Array<std::int32_t> &&>(int32list))),
        type(nbt_type::int32list) {}
  constexpr nbt_node(Array<std::int64_t> &&int64list)
      : int64list(new Array<std::int64_t>(
            std::forward<Array<std::int64_t> &&>(int64list))),
        type(nbt_type::int64list) {}
  template <typename T> T &as() {
    if (type_to_nbt_type<T> != type) {
      THROW(std::runtime_error("type mismatch"));
    }
    if constexpr (std::is_same_v<T, decltype(end)>)
      return end;
    else if constexpr (std::is_same_v<T, decltype(int8)>)
      return int8;
    else if constexpr (std::is_same_v<T, decltype(int16)>)
      return int16;
    else if constexpr (std::is_same_v<T, decltype(int32)>)
      return int32;
    else if constexpr (std::is_same_v<T, decltype(int64)>)
      return int64;
    else if constexpr (std::is_same_v<T, decltype(float32)>)
      return float32;
    else if constexpr (std::is_same_v<T, decltype(float64)>)
      return float64;
    else if constexpr (std::is_same_v<T, decltype(*int8list)>)
      return *int8list;
    else if constexpr (std::is_same_v<T, decltype(*string)>)
      return *string;
    else if constexpr (std::is_same_v<T, decltype(*list)>)
      return *list;
    else if constexpr (std::is_same_v<T, decltype(*compound)>)
      return *compound;
    else if constexpr (std::is_same_v<T, decltype(*int32list)>)
      return *int32list;
    else if constexpr (std::is_same_v<T, decltype(*int64list)>)
      return *int64list;
    std::unreachable();
  }
  template <typename T> const T &as() const {
    if (type_to_nbt_type<T> != type) {
      THROW(std::runtime_error("type mismatch"));
    }
    if constexpr (std::is_same_v<T, decltype(end)>)
      return end;
    else if constexpr (std::is_same_v<T, decltype(int8)>)
      return int8;
    else if constexpr (std::is_same_v<T, decltype(int16)>)
      return int16;
    else if constexpr (std::is_same_v<T, decltype(int32)>)
      return int32;
    else if constexpr (std::is_same_v<T, decltype(int64)>)
      return int64;
    else if constexpr (std::is_same_v<T, decltype(float32)>)
      return float32;
    else if constexpr (std::is_same_v<T, decltype(float64)>)
      return float64;
    else if constexpr (std::is_same_v<T, decltype(*int8list)>)
      return *int8list;
    else if constexpr (std::is_same_v<T, decltype(*string)>)
      return *string;
    else if constexpr (std::is_same_v<T, decltype(*list)>)
      return *list;
    else if constexpr (std::is_same_v<T, decltype(*compound)>)
      return *compound;
    else if constexpr (std::is_same_v<T, decltype(*int32list)>)
      return *int32list;
    else if constexpr (std::is_same_v<T, decltype(*int64list)>)
      return *int64list;
    std::unreachable();
  }
  template <typename T> operator T &() {
    if (type_to_nbt_type<T> != type) {
      THROW(std::runtime_error("type mismatch"));
    }
    if constexpr (std::is_same_v<T, decltype(end)>)
      return end;
    else if constexpr (std::is_same_v<T, decltype(int8)>)
      return int8;
    else if constexpr (std::is_same_v<T, decltype(int16)>)
      return int16;
    else if constexpr (std::is_same_v<T, decltype(int32)>)
      return int32;
    else if constexpr (std::is_same_v<T, decltype(int64)>)
      return int64;
    else if constexpr (std::is_same_v<T, decltype(float32)>)
      return float32;
    else if constexpr (std::is_same_v<T, decltype(float64)>)
      return float64;
    else if constexpr (std::is_same_v<T, decltype(*int8list)>)
      return *int8list;
    else if constexpr (std::is_same_v<T, decltype(*string)>)
      return *string;
    else if constexpr (std::is_same_v<T, decltype(*list)>)
      return *list;
    else if constexpr (std::is_same_v<T, decltype(*compound)>)
      return *compound;
    else if constexpr (std::is_same_v<T, decltype(*int32list)>)
      return *int32list;
    else if constexpr (std::is_same_v<T, decltype(*int64list)>)
      return *int64list;
    std::unreachable();
  }
  template <typename T> operator const T &() const {
    if (type_to_nbt_type<T> != type) {
      THROW(std::runtime_error("type mismatch"));
    }
    if constexpr (std::is_same_v<T, decltype(end)>)
      return end;
    else if constexpr (std::is_same_v<T, decltype(int8)>)
      return int8;
    else if constexpr (std::is_same_v<T, decltype(int16)>)
      return int16;
    else if constexpr (std::is_same_v<T, decltype(int32)>)
      return int32;
    else if constexpr (std::is_same_v<T, decltype(int64)>)
      return int64;
    else if constexpr (std::is_same_v<T, decltype(float32)>)
      return float32;
    else if constexpr (std::is_same_v<T, decltype(float64)>)
      return float64;
    else if constexpr (std::is_same_v<T, decltype(*int8list)>)
      return *int8list;
    else if constexpr (std::is_same_v<T, decltype(*string)>)
      return *string;
    else if constexpr (std::is_same_v<T, decltype(*list)>)
      return *list;
    else if constexpr (std::is_same_v<T, decltype(*compound)>)
      return *compound;
    else if constexpr (std::is_same_v<T, decltype(*int32list)>)
      return *int32list;
    else if constexpr (std::is_same_v<T, decltype(*int64list)>)
      return *int64list;
    std::unreachable();
  }
  constexpr bool is(auto type) const { return this->type == type; }
  template <typename T> constexpr bool is() const {
    return is(type_to_nbt_type<T>);
  }
  constexpr nbt_type get_type() const { return type; }
};
template <std::endian Endian, typename Array>
constexpr void swap_if_need(Array &bytes) {
  if constexpr (std::endian::native != Endian) {
    std::reverse(bytes.begin(), bytes.end());
  }
}
template <std::endian Endian, typename Result>
constexpr Result write_integer(std::integral auto number) {
  auto bytes = std::bit_cast<std::array<unsigned char, sizeof(number)>>(number);
  Result res(sizeof(number));
  std::copy(bytes.begin(), bytes.end(), res.begin());
  swap_if_need<Endian, Result>(res);
  return res;
}
template <std::endian Endian, typename Result>
constexpr Result write_float(std::floating_point auto number) {
  auto bytes = std::bit_cast<std::array<unsigned char, sizeof(number)>>(number);
  Result res(sizeof(number));
  std::copy(bytes.begin(), bytes.end(), res.begin());
  return res;
}
template <template <typename> typename Array>
constexpr Array<unsigned char> write_type(nbt_type Type) {
  return {std::bit_cast<unsigned char>(Type)};
}
template <typename T, typename... Ts>
constexpr auto concat(const T &a, const Ts &...rest) {
  std::size_t size = a.size();
  if constexpr (sizeof...(rest) != 0)
    size += (rest.size() + ...);
  T res(size);
  std::size_t i = 0;
  for (const auto &e : a)
    res[i++] = e;
  if constexpr (sizeof...(rest) != 0)
    for (const auto &e : concat(rest...))
      res[i++] = e;
  return res;
}
template <std::endian Endian, typename T, typename SizeType, typename Array,
          typename Result>
Result write_array(const Array &array) {
  Result res(array.size() * sizeof(T));
  for (std::size_t i = 0; i < array.size(); i++) {
    auto integer = write_integer<Endian, Result>(array[i]);
    std::copy(integer.begin(), integer.end(), res.begin() + i * sizeof(T));
  }
  return concat(
      write_integer<Endian, Result>(static_cast<SizeType>(array.size())), res);
}
template <std::endian Endian, typename Nbt>
constexpr Nbt::template array_t<unsigned char> write_no_type(const Nbt &nbt);
template <std::endian Endian, typename Nbt>
constexpr Nbt::template array_t<unsigned char> write_type(const Nbt &nbt,
                                                          bool isRoot = false) {
  if (isRoot)
    return concat(write_type<Nbt::template array_t>(nbt.get_type()),
                  typename Nbt::template array_t<unsigned char>{0, 0},
                  write_no_type<Endian>(nbt));
  else
    return concat(write_type<Nbt::template array_t>(nbt.get_type()),
                  write_no_type<Endian>(nbt));
}
template <std::endian Endian, typename Nbt>
constexpr Nbt::template array_t<unsigned char> write_no_type(const Nbt &nbt) {
  using Result = Nbt::template array_t<unsigned char>;
  switch (nbt.type) {
  case nbt_type::end:
    return {};
  case nbt_type::int8:
    return write_integer<Endian, Result>(nbt.template as<std::int8_t>());
  case nbt_type::int16:
    return write_integer<Endian, Result>(nbt.template as<std::int16_t>());
  case nbt_type::int32:
    return write_integer<Endian, Result>(nbt.template as<std::int32_t>());
  case nbt_type::int64:
    return write_integer<Endian, Result>(nbt.template as<std::int64_t>());
  case nbt_type::float32:
    return write_float<Endian, Result>(nbt.template as<float>());
  case nbt_type::float64:
    return write_float<Endian, Result>(nbt.template as<double>());
  case nbt_type::int8list:
    return write_array<Endian, std::int8_t, std::int32_t,
                       typename Nbt::int8list_t, Result>(
        nbt.template as<typename Nbt::int8list_t>());
  case nbt_type::string: {
    Result res(nbt.template as<typename Nbt::string_t>().size());
    std::copy(nbt.template as<typename Nbt::string_t>().begin(),
              nbt.template as<typename Nbt::string_t>().end(), res.begin());
    return concat(write_integer<Endian, Result>(static_cast<std::int16_t>(
                      nbt.template as<typename Nbt::string_t>().size())),
                  res);
  }
  case nbt_type::list: {
    auto res = concat(write_type<Nbt::template array_t>(
                          nbt.template as<typename Nbt::list_t>()[0].type),
                      write_integer<Endian, Result>(static_cast<std::int32_t>(
                          nbt.template as<typename Nbt::list_t>().size())));
    for (const auto &element : nbt.template as<typename Nbt::list_t>())
      res = concat(res, write_type<Endian>(element));
    return res;
  }
  case nbt_type::compound: {
    Result res;
    for (const auto &[k, v] : nbt.template as<typename Nbt::compound_t>()) {
      Result str(k.size());
      std::copy(k.begin(), k.end(), str.begin());
      str = concat(
          write_type<Nbt::template array_t>(v.get_type()),
          write_integer<Endian, Result>(static_cast<std::int16_t>(k.size())),
          str);
      res = concat(res, str, write_no_type<Endian>(v));
    }
    res = concat(res, write_type<Nbt::template array_t>(nbt_type::end));
    return res;
  }
  case nbt_type::int32list:
    return write_array<Endian, std::int32_t, std::int32_t,
                       typename Nbt::int32list_t, Result>(
        nbt.template as<typename Nbt::int32list_t>());
  case nbt_type::int64list:
    return write_array<Endian, std::int64_t, std::int32_t,
                       typename Nbt::int64list_t, Result>(
        nbt.template as<typename Nbt::int64list_t>());
  default:
    std::unreachable();
  }
}
} // namespace libnbt
