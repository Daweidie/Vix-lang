#!/bin/bash
set -e
gcc -c src/helper.c -o helper.o $(llvm-config --cflags)
gcc -c src/runtime.c -o runtime.o
./vixc -obj src/main.vix -o vixc0.o
clang -o vixc0 vixc0.o helper.o runtime.o $(llvm-config --ldflags --libs)
