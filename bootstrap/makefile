LLVM_CFLAGS = $(shell llvm-config --cflags)
TARGET = vixc0

all: $(TARGET)

helper.o: helper.c
	gcc -c $< -o $@ $(LLVM_CFLAGS)

SOURCES = main.vix lexer.vix parser.vix ast.vix typeinfer.vix semantic.vix symtable.vix codegen.vix builder.vix error.vix

vixc0.o: $(SOURCES)
	vixc $< -obj $@ -opt=l2

$(TARGET): helper.o vixc0.o
	vixc $^ -o $@ -L /usr/lib -l LLVM-22

clean:
	rm -f helper.o vixc0.o $(TARGET) test test.ll 

test: $(TARGET)
	python3 ../tests/c0_test.py

.PHONY: all clean test
