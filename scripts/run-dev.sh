#!/usr/bin/env bash

cmake -S . -B build && cmake --build build
./build/io test.io
echo $?

./out
echo $?
