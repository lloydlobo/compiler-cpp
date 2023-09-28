# io lang: C++ Language Compiler

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

- YouTube video series "[Creating a Compiler](https://www.youtube.com/playlist?list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs)" by Pixeled
- Source [orosmatthew/hydrogen-cpp](https://github.com/orosmatthew/hydrogen-cpp) by Pixeled
