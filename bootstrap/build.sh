#!/bin/bash
gcc -c helper.c -o helper.o $(llvm-config --cflags)
vixc main.vix -obj vixc0.o -opt=l2
vixc vixc0.o helper.o -o vixc0 -l LLVM-22