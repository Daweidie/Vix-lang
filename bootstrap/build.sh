#!/bin/bash
set -e

mkdir -p build runtime

gcc -c src/helper.c -o build/helper.o $(llvm-config --cflags) -Wno-deprecated-declarations
gcc -c src/runtime.c -o runtime/runtime.o

./vixc src/main.vix -obj -o build/vixc.o
clang -o build/vixc build/vixc.o build/helper.o runtime/runtime.o $(llvm-config --ldflags --libs)
