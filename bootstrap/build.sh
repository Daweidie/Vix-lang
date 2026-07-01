#!/bin/bash
./vixc1 main.vix > main.ll
clang -c main.ll -o vixc1.o
clang -o vixc1 vixc1.o helper.o $(llvm-config --ldflags --libs)
