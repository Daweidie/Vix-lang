#!/bin/bash
set -e
./vixc1 -obj main.vix -o vixc1.o
clang -o vixc2 vixc1.o helper.o $(llvm-config --ldflags --libs)
