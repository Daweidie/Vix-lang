#!/bin/bash
set -e
./vixc0 -obj main.vix -o vixc1.o
clang -o vixc1 vixc1.o helper.o $(llvm-config --ldflags --libs)
