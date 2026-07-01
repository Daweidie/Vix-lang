#!/bin/bash
./vixc2 main.vix > main.ll
clang -c main.ll -o vixc1.o
clang -o vixc3 vixc1.o helper.o $(llvm-config --ldflags --libs)
