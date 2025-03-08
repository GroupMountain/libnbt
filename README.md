# libnbt
## usage
download ```libnbt.hpp``` add it to include path.
## example
```cpp
#include "libnbt.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

int main() {
  libnbt::nbt node{{
      {"a", 1},
      {"b", 2},
      {"c",
       {{
           {"123", 1},
           {"456", {{{"b", libnbt::nbt::int8list_t{2, 3, 4}}}}},
       }}},
      {"d", libnbt::nbt::list_t{libnbt::nbt::compound_t{
                {"a", 1},
                {"b",
                 libnbt::nbt::compound_t{
                     {"a", 1},
                     {"b", {"123546e4f864f6w"}},
                 }},
            }}},
  }};
  auto res = libnbt::write<libnbt::bedrock_endian>(node, "123");
  auto nbt = fopen("test.nbt", "wb");
  for (auto b : res) {
    fputc(b, nbt);
  }
  fclose(nbt);
  nbt = fopen("test.nbt", "rb");
  std::vector<unsigned char> buffer;
  auto c = (unsigned)fgetc(nbt);
  while (c != EOF) {
    buffer.emplace_back(c);
    c = (unsigned)fgetc(nbt);
  }
  auto begin = buffer.data();
  auto result =
      libnbt::read<libnbt::bedrock_endian,
                   libnbt::nbt_node<std::map, std::vector, std::string>>(
          begin, begin + buffer.size());
  std::cout << libnbt::to_string(result);
}
```
