# libnbt
## example
```cpp
#include "normal.hpp"
#include "view.hpp"
#include <chrono>
#include <iostream>

struct Timer {
  Timer() { start = std::chrono::high_resolution_clock::now(); }
  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
              << "\n";
  }
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};
int main() {
  std::size_t size = 0;
  std::byte *data = nullptr;
  {
    libnbt::nbt_map compound{
        {"a", std::make_shared<libnbt::nbt_i8>(1)},
        {"b", std::make_shared<libnbt::nbt_i16>(2)},
        {"c", std::make_shared<libnbt::nbt_i32>(3)},
        {"d", std::make_shared<libnbt::nbt_i64>(4)},
        {"e", std::make_shared<libnbt::nbt_f32>(5.f)},
        {"f", std::make_shared<libnbt::nbt_f64>(6)},
        {"g", std::make_shared<libnbt::nbt_i8arr>(libnbt::nbt_i8arr{7, 8})},
        {"h", std::make_shared<libnbt::nbt_str>("89")},
        {"i", std::make_shared<libnbt::nbt_arr>(libnbt::nbt_arr{
                  std::make_shared<libnbt::nbt_i8>(10),
                  std::make_shared<libnbt::nbt_i8>(11),
              })},
        {"j",
         std::make_shared<libnbt::nbt_map>(libnbt::nbt_map{
             {"a", std::make_shared<libnbt::nbt_i8>(1)},
             {"b", std::make_shared<libnbt::nbt_i16>(2)},
             {"c", std::make_shared<libnbt::nbt_i32>(3)},
             {"d", std::make_shared<libnbt::nbt_i64>(4)},
             {"e", std::make_shared<libnbt::nbt_f32>(5.f)},
             {"f", std::make_shared<libnbt::nbt_f64>(6)},
             {"g",
              std::make_shared<libnbt::nbt_i8arr>(libnbt::nbt_i8arr{7, 8})},
             {"h", std::make_shared<libnbt::nbt_str>("89")},
             {"i", std::make_shared<libnbt::nbt_arr>(libnbt::nbt_arr{
                       std::make_shared<libnbt::nbt_i8>(10),
                       std::make_shared<libnbt::nbt_i8>(11),
                   })},
             {"j", std::make_shared<libnbt::nbt_map>()},
             {"k",
              std::make_shared<libnbt::nbt_i32arr>(libnbt::nbt_i32arr{12, 13})},
             {"l",
              std::make_shared<libnbt::nbt_i64arr>(libnbt::nbt_i64arr{14, 15})},
         })},
        {"k", std::make_shared<libnbt::nbt_i32arr>(libnbt::nbt_i32arr{12, 13})},
        {"l", std::make_shared<libnbt::nbt_i64arr>(libnbt::nbt_i64arr{14, 15})},
    };

    int r = 10;
    while (r--) {
      Timer timer;
      compound.emplace(std::to_string(r), compound.clone());
      std::cout << r << "\n";
    }

    libnbt::nbt_root root{std::make_shared<libnbt::nbt_map>(compound)};
    size = root.size_bytes();
    data = new std::byte[size];
    root.to_bytes({data, size}, std::endian::native);
    for (std::size_t i = 0; i < size; i++)
      std::cout << std::hex << (int)data[i] << " ";
    std::cout << "\n";
    auto file = std::fopen("test.nbt", "wb");
    std::fwrite(data, sizeof(std::byte), size, file);
    std::fclose(file);
  }
  {
    Timer timer;
    libnbt::nbt_root root;
    auto res = root.from_bytes({data, size}, std::endian::native);
    std::cout << std::dec << size << " " << res << "\n";
  }
  {
    Timer timer;
    std::cout << libnbt::validate({data, size}, ::std::endian::native) << "\n";
  }
  {
    Timer timer;
    libnbt::nbt_view_root root;
    std::cout << root.from_bytes({data, size}, std::endian::native) << "\n";
  }
  delete[] data;
}

```
