LLVM_CFLAGS = $(shell llvm-config --cflags)
TARGET = vixc0

all: $(TARGET)

helper.o: helper.c
	gcc -c $< -o $@ $(LLVM_CFLAGS)

vixc0.o: main.vix
	vixc $< -obj $@ -opt=l2

$(TARGET): helper.o vixc0.o
	vixc $^ -o $@ -L /usr/lib -l LLVM-22

clean:
	rm -f helper.o vixc0.o $(TARGET) test test.ll 

test: $(TARGET)
	python3 ../tests/vixc0_run.py

.PHONY: all clean test
