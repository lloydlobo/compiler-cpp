# io Language Compiler

## `asm` with linking

```shell
nasm -felf64 test.asm
ld test.o -o test
./test
echo $?
```

## Build

```shell
cmake -S . -B build && cmake --build build
./build/io test.io
echo $?

./out
echo $?
```

## Credits

[orosmatthew/hydrogen-cpp](https://github.com/orosmatthew/hydrogen-cpp/blob/master/src/main.cpp)
