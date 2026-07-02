#!/bin/bash
set -e
gcc -c src/helper.c -o helper.o $(llvm-config --cflags)
gcc -c src/runtime.c -o runtime.o
./vixc0 -obj src/main.vix -o vixc1.o
clang -o vixc1 vixc1.o helper.o runtime.o $(llvm-config --ldflags --libs)
