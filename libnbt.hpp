#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <map>
#include <memory>
#include <variant>
#include <vector>

#if defined(_MSC_VER) && !defined(__clang__)
#define UNREACHABLE() __assume(false)
#else
#define UNREACHABLE() __builtin_unreachable()
#endif
namespace libnbt {
struct invalid_type : std::runtime_error {
  invalid_type(const std::string &what)
      : std::runtime_error("invalid type:" + what) {}
};
struct invalid_data : std::runtime_error {
  invalid_data(const std::string &what)
      : std::runtime_error("invalid data:" + what) {}
};
template <typename T>
concept input_buffer_like = requires(const T &a) {
  { a.size() } -> std::same_as<typename T::size_type>;
  { a[1] } -> std::same_as<const typename T::value_type &>;
};
template <typename T, std::endian E = std::endian::little> struct input;
template <input_buffer_like T, std::endian E> struct input<T, E> {
  const T &data;
  T::size_type index = 0;
  template <typename U> struct reader;
  template <> struct reader<char> {
    static constexpr char read(const T &data, T::size_type &index) {
      if (index >= data.size()) {
        throw invalid_data("out of range.");
      }
      return data[index++];
    }
  };
  template <std::integral U> struct reader<U> {
    static constexpr U read(const T &data, T::size_type &index) {
      U val{0};
      char *ptr = reinterpret_cast<char *>(std::addressof(val));
      for (std::size_t i = 0; i < sizeof(U); i++) {
        *(ptr + i) = reader<char>::read(data, index);
      }
      if (E != std::endian::native)
        std::reverse(reinterpret_cast<char *>(std::addressof(val)),
                     reinterpret_cast<char *>(std::addressof(val)) + sizeof(U));
      return val;
    }
  };
  template <std::floating_point U> struct reader<U> {
    static constexpr U read(const T &data, T::size_type &index) {
      U val{0};
      char *ptr = reinterpret_cast<char *>(std::addressof(val));
      for (std::size_t i = 0; i < sizeof(U); i++) {
        *(ptr + i) = reader<char>::read(data, index);
      }
      if (E != std::endian::native)
        std::reverse(ptr, ptr + sizeof(U));
      return val;
    }
  };
  template <typename U> constexpr U read() {
    return reader<U>::read(data, index);
  }
  constexpr void reset() { index = 0; }
};
template <typename T> input(const T &data) -> input<T>;

template <typename T>
concept output_buffer_like = input_buffer_like<T> && requires(T &a) {
  { a.resize(1) };
  { a[1] } -> std::same_as<typename T::value_type &>;
};
template <typename T, std::endian E = std::endian::little> struct output;
template <output_buffer_like T, std::endian E> struct output<T, E> {
  T &data;
  T::size_type index;
  template <typename U> struct writer;
  template <> struct writer<char> {
    static constexpr void write(char val, T &data, T::size_type &index) {
      if (data.size() >= index) {
        data.resize(data.size() + 1);
      }
      data[index++] = val;
    }
  };
  template <std::integral U> struct writer<U> {
    static constexpr void write(U val, T &data, T::size_type &index) {
      char *ptr = reinterpret_cast<char *>(std::addressof(val));
      if (E != std::endian::native)
        std::reverse(ptr, ptr + sizeof(U));
      for (std::size_t i = 0; i < sizeof(U); i++) {
        writer<char>::write(*(ptr + i), data, index);
      }
    }
  };
  template <std::floating_point U> struct writer<U> {
    static constexpr void write(U val, T &data, T::size_type &index) {
      char *ptr = reinterpret_cast<char *>(std::addressof(val));
      if (E != std::endian::native)
        std::reverse(ptr, ptr + sizeof(U));
      for (std::size_t i = 0; i < sizeof(U); i++) {
        writer<char>::write(*(ptr + i), data, index);
      }
    }
  };
  template <typename U> constexpr void write(U val) {
    return writer<std::remove_cvref_t<U>>::write(val, data, index);
  }
  constexpr void reset() { index = 0; }
  constexpr void to_fit() { data.resize(index); }
};
template <typename T> output(T &data) -> output<T>;

enum class nbt_type : std::int8_t {
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

template <typename Str, template <typename> typename Arr,
          template <typename, typename> typename Map>
struct nbt_any {
  using i8 = std::int8_t;
  using i16 = std::int16_t;
  using i32 = std::int32_t;
  using i64 = std::int64_t;
  using f32 = float;
  using f64 = double;
  struct any_wapper;
  template <typename T> struct typed_arr;
  struct any_wapper {
    nbt_any *data;
    template <typename U> U &as() { return data->as<U>(); }
    template <typename U> U &as() const { return data->as<U>(); }
    template <typename U> U *as_if() { return data->as_if<U>(); }
    template <typename U> U *as_if() const { return data->as_if<U>(); }
    constexpr any_wapper() : data(nullptr) {}
    constexpr any_wapper(const any_wapper &other)
        : data(new nbt_any(*other.data)) {}
    constexpr any_wapper &operator=(const any_wapper &other) {
      data = new nbt_any(*other.data);
      return *this;
    }
    template <typename U>
    constexpr any_wapper(const U &other) : data(new nbt_any(other)) {}
    template <typename U> constexpr any_wapper &operator=(const U &other) {
      data = new nbt_any(other);
      return *this;
    }
    constexpr ~any_wapper() { delete data; }
  };
  using nbt_variant =
      std::variant<std::monostate, i8, i16, i32, i64, f32, f64, typed_arr<i8>,
                   Str, typed_arr<any_wapper>, Map<Str, any_wapper>,
                   typed_arr<i32>, typed_arr<i64>>;
  template <typename T> struct typed_arr : Arr<any_wapper> {
    constexpr typed_arr() = default;
    constexpr typed_arr(std::initializer_list<T> data) {
      this->resize(data.size());
      for (std::size_t index = 0; auto i : data)
        access(index++) = i;
    }
    any_wapper &access(std::size_t index) {
      return this->Arr<any_wapper>::operator[](index);
    }
    const any_wapper &access(std::size_t index) const {
      return this->Arr<any_wapper>::operator[](index);
    }
    T &operator[](std::size_t index) {
      if constexpr (!std::same_as<T, any_wapper>) {
        return access(index).data->template as<T>();
      } else
        return access(index);
    }
    const T &operator[](std::size_t index) const {
      if constexpr (!std::same_as<T, any_wapper>)
        return access(index).data->template as<T>();
      else
        return access(index);
    }
  };

  nbt_variant data;
  template <typename U> U &as() { return std::get<U>(data); }
  template <typename U> U &as() const { return std::get<U>(data); }
  template <typename U> U *as_if() { return std::get_if<U>(data); }
  template <typename U> U *as_if() const { return std::get_if<U>(data); }
  template <typename... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
  };
  template <typename Visitor> constexpr auto visit(Visitor &&vis) {
    return std::visit(std::forward<Visitor>(vis), data);
  }
  template <typename R, typename Visitor> constexpr R visit(Visitor &&vis) {
    return std::visit<R>(std::forward<Visitor>(vis), data);
  }
  template <typename Visitor> constexpr auto visit(Visitor &&vis) const {
    return std::visit(std::forward<Visitor>(vis), data);
  }
  template <typename R, typename Visitor>
  constexpr R visit(Visitor &&vis) const {
    return std::visit<R>(std::forward<Visitor>(vis), data);
  }
  constexpr nbt_any &operator[](const std::string &index) {
    return visit<nbt_any &>(overloaded{
        [](auto &&) -> nbt_any & {
          throw invalid_type("type is not a map.");
          UNREACHABLE();
        },
        [&](Map<Str, any_wapper> &val) -> nbt_any & {
          return *val[index].data;
        },
    });
  }
  constexpr nbt_any &operator[](std::size_t index) {
    return visit<nbt_any &>(overloaded{
        [](auto &&) -> nbt_any & {
          throw invalid_type("type is not an array.");
          UNREACHABLE();
        },
        [&](typed_arr<i8> &val) -> nbt_any & {
          return *val.access(index).data;
        },
        [&](typed_arr<i32> &val) -> nbt_any & {
          return *val.access(index).data;
        },
        [&](typed_arr<i64> &val) -> nbt_any & {
          return *val.access(index).data;
        },
        [&](typed_arr<any_wapper> &val) -> nbt_any & {
          return *val.access(index).data;
        },
    });
  }
  using str = Str;
  using i8arr = typed_arr<i8>;
  using i32arr = typed_arr<i32>;
  using i64arr = typed_arr<i64>;
  using arr = typed_arr<any_wapper>;
  using map = Map<Str, any_wapper>;
  using iterator_type =
      std::variant<typename map::iterator, typename str::iterator,
                   typename arr::iterator>;
  using const_iterator_type =
      std::variant<typename map::const_iterator, typename str::const_iterator,
                   typename arr::const_iterator>;
  template <typename U> struct iterator_holder {
    U iter;
    constexpr auto &operator++() {
      std::visit(overloaded{[](auto &iter) { ++iter; }}, iter);
      return *this;
    }
    constexpr auto &operator--() {
      std::visit(overloaded{[](auto &iter) { --iter; }}, iter);
      return *this;
    }
    constexpr auto operator++(int) {
      auto old = *this;
      std::visit(overloaded{[](auto &iter) { ++iter; }}, iter);
      return old;
    }
    constexpr auto operator--(int) {
      auto old = *this;
      std::visit(overloaded{[](auto &iter) { --iter; }}, iter);
      return old;
    }
    constexpr bool operator==(const iterator_holder &other) const {
      return std::visit<bool>(
          [](auto &a, auto &b) {
            if constexpr (std::same_as<decltype(a), decltype(b)>)
              return a == b;
            else
              return false;
          },
          iter, other.iter);
    }
    struct result_t {
      std::variant<typename map::value_type *, typename str::value_type *,
                   typename arr::value_type *>
          data;
      template <typename T> auto as() { return *std::get<T *>(data); }
      template <typename T> auto as() const { return *std::get<T *>(data); }
      template <typename T> auto as_if() {
        auto r = std::get_if<T *>(&data);
        return r ? *r : nullptr;
      }
      template <typename T> auto as_if() const {
        auto r = std::get_if<T *>(&data);
        return r ? *r : nullptr;
      }
    };
    result_t res0;
    constexpr auto &operator*() {
      res0 = std::visit<result_t>(
          [&](auto &i) -> result_t { return result_t{{&*i}}; }, iter);
      return res0;
    }
    const result_t res1;
    constexpr const auto &operator*() const {
      res1 = std::visit<const result_t>(
          [&](auto &i) -> const result_t { return result_t{{&*i}}; }, iter);
      return res1;
    }
  }; // namespace libnbt
  using iterator = iterator_holder<iterator_type>;
  using const_iterator = iterator_holder<const_iterator_type>;
  iterator begin() {
    return visit<iterator>(overloaded{[](auto &data) -> iterator {
      if constexpr (requires() { data.begin(); })
        return {data.begin()};
      throw invalid_type("type is not iterable.");
      UNREACHABLE();
    }});
  }
  const_iterator begin() const {
    return visit<const_iterator>(overloaded{[](auto &data) -> const_iterator {
      if constexpr (requires() { data.begin(); })
        return {data.begin()};
      throw invalid_type("type is not iterable.");
      UNREACHABLE();
    }});
  }
  iterator end() {
    return visit<iterator>(overloaded{[](auto &data) -> iterator {
      if constexpr (requires() { data.end(); })
        return {data.end()};
      throw invalid_type("type is not iterable.");
      UNREACHABLE();
    }});
  }
  const_iterator end() const {
    return visit<const_iterator>(overloaded{[](auto &data) -> const_iterator {
      if constexpr (requires() { data.end(); })
        return {data.end()};
      throw invalid_type("type is not iterable.");
      UNREACHABLE();
    }});
  }
};
using nbt = libnbt::nbt_any<std::string, std::vector, std::map>;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using f32 = float;
using f64 = double;
using str = nbt::str;
using i8arr = nbt::i8arr;
using i32arr = nbt::i32arr;
using i64arr = nbt::i64arr;
using arr = nbt::arr;
using map = nbt::map;
template <typename T> struct binary_nbt_writer {
  T &output;
  template <typename N> void write_no_type(const N &nbt) {
    nbt.visit(typename N::overloaded{
        [this](const std::integral auto &val) { output.template write<>(val); },
        [this](const std::floating_point auto &val) {
          output.template write<>(val);
        },
        [this](const N::str &val) {
          output.template write<i16>(static_cast<i16>(val.size()));
          for (auto c : val) {
            output.template write<char>(c);
          }
        },
        [this](const N::i8arr &val) {
          output.template write<i32>(static_cast<i32>(val.size()));
          for (auto e : val) {
            output.template write<i8>(e.template as<i8>());
          }
        },
        [this](const N::i32arr &val) {
          output.template write<i32>(static_cast<i32>(val.size()));
          for (auto e : val) {
            output.template write<i32>(e.template as<i32>());
          }
        },
        [this](const N::i64arr &val) {
          output.template write<i32>(static_cast<i32>(val.size()));
          for (auto e : val) {
            output.template write<i64>(e.template as<i64>());
          }
        },
        [this](const N::arr &val) {
          if (val.size() == 0) {
            output.template write<char>(static_cast<char>(nbt_type::end));
            output.template write<i32>(0);
          } else {
            output.template write<char>(
                static_cast<char>(val[0].data->data.index()));
            output.template write<i32>(static_cast<i32>(val.size()));
            for (auto e : val) {
              write_no_type(*e.data);
            }
          }
        },
        [this](const N::map &val) {
          for (auto &[n, e] : val) {
            output.template write<char>(e.data->data.index());
            write_no_type(N{n});
            write_no_type(*e.data);
          }
          output.template write<char>(static_cast<char>(nbt_type::end));
        },
        [this](const std::monostate &) {},
    });
  }
  template <typename N> void write(const N &nbt, const N::str &name = "") {
    output.template write<char>(static_cast<char>(nbt_type::map));
    write_no_type(N{name});
    write_no_type(nbt);
  }
};

template <typename T> struct binary_nbt_reader {
  T &input;
  template <typename N> N read_no_type(nbt_type type) {
    switch (type) {
    case nbt_type::end:
      return {};
    case nbt_type::i8:
      return {input.template read<i8>()};
    case nbt_type::i16:
      return {input.template read<i16>()};
    case nbt_type::i32:
      return {input.template read<i32>()};
    case nbt_type::i64:
      return {input.template read<i64>()};
    case nbt_type::f32:
      return {input.template read<f32>()};
    case nbt_type::f64:
      return {input.template read<f64>()};
    case nbt_type::i8arr: {
      auto size = input.template read<i32>();
      typename N::i8arr arr;
      arr.resize(size);
      for (std::size_t i = 0; i < size; i++) {
        arr.access(i) = input.template read<i8>();
      }
      return {arr};
    }
    case nbt_type::i32arr: {
      auto size = input.template read<i32>();
      typename N::i32arr arr;
      arr.resize(size);
      for (std::size_t i = 0; i < size; i++) {
        arr.access(i) = input.template read<i32>();
      }
      return {arr};
    }
    case nbt_type::i64arr: {
      auto size = input.template read<i32>();
      typename N::i64arr arr;
      arr.resize(size);
      for (std::size_t i = 0; i < size; i++) {
        arr.access(i) = input.template read<i64>();
      }
      return {arr};
    }
    case nbt_type::arr: {
      auto type = static_cast<nbt_type>(input.template read<char>());
      auto size = input.template read<i32>();
      typename N::arr arr;
      if (type != nbt_type::end) {
        arr.resize(size);
        for (std::size_t i = 0; i < size; i++) {
          arr[i] = read_no_type<N>(type);
        }
      }
      return {arr};
    }
    case nbt_type::map: {
      auto type = static_cast<nbt_type>(input.template read<char>());
      typename N::map map;
      while (type != nbt_type::end) {
        auto size = input.template read<i16>();
        typename N::str str;
        str.resize(size);
        for (std::size_t i = 0; i < size; i++) {
          str[i] = input.template read<char>();
        }
        N e = read_no_type<N>(type);
        map[str] = e;
        type = static_cast<nbt_type>(input.template read<char>());
      }
      return {map};
    }
    case nbt_type::str: {
      auto size = input.template read<i16>();
      typename N::str str;
      str.resize(size);
      for (std::size_t i = 0; i < size; i++) {
        str[i] = input.template read<char>();
      }
      return {str};
    }
    }
  }
  template <typename N> std::pair<N, typename N::str> read() {
    auto type = static_cast<nbt_type>(input.template read<char>());
    typename N::str name =
        read_no_type<N>(nbt_type::str).template as<typename N::str>();
    return {read_no_type<N>(type), name};
  }
};
} // namespace libnbt
