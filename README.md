# libnbt
## usage
download ```libnbt.hpp``` add it to include path.
## example
```cpp
#include "iostream"

#include "libnbt.hpp"

int main() {
  libnbt::nbt a{
      {
          libnbt::map{
              {
                  "a",
                  libnbt::i8arr{1, 2, 3},
              },
              {
                  "b",
                  libnbt::map{
                      {
                          "c",
                          libnbt::arr{
                              libnbt::i64{1},
                              libnbt::i64{2},
                              libnbt::i64{3},
                          },
                      },
                      {
                          "d",
                          libnbt::arr{
                              libnbt::map{
                                  {
                                      "e",
                                      libnbt::arr{
                                          "f",
                                          "g",
                                      },
                                  },
                              },
                          },
                      },
                  },
              },
          },
      },
  };
  std::string buffer;
  libnbt::output output{buffer};
  libnbt::binary_nbt_writer writer{output};
  writer.write(a, "123");
  output.to_fit();
  auto file = std::fopen("test.nbt", "wb+");
  for (auto c : buffer) {
    std::cout << " " << std::hex << (int)c;
    fputc(c, file);
  }
  fclose(file);
}
```
