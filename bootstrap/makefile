LLVM_CFLAGS = $(shell llvm-config --cflags)
LLVM_LIBDIR = $(shell llvm-config --libdir)
LLVM_LIBS = $(patsubst -l%,-l %,$(shell llvm-config --libs))
SRC_DIR = src
TARGET = vixc
BOOTSTRAP_TARGET = vixc0

all: $(TARGET)

bootstrap: $(BOOTSTRAP_TARGET)

helper.o: $(SRC_DIR)/helper.c
	gcc -c $< -o $@ $(LLVM_CFLAGS) -Wno-deprecated-declarations

runtime.o: $(SRC_DIR)/runtime.c
	gcc -c $< -o $@

SOURCES = $(addprefix $(SRC_DIR)/,main.vix lexer.vix parser.vix ast.vix desugar.vix typeinfer.vix semantic.vix symtable.vix codegen.vix mir.vix mir_builder.vix builder.vix error.vix)

vixc.o: $(SOURCES)
	../build/vixc $(SRC_DIR)/main.vix -obj $@ -opt=l2

$(TARGET): helper.o runtime.o vixc.o
	../build/vixc $^ -o $@ -L $(LLVM_LIBDIR) $(LLVM_LIBS)

$(BOOTSTRAP_TARGET): helper.o runtime.o vixc.o
	../build/vixc $^ -o $@ -L $(LLVM_LIBDIR) $(LLVM_LIBS)

clean:
	rm -f helper.o runtime.o vixc.o vixc1.o $(TARGET) $(BOOTSTRAP_TARGET) vixc1 test test.ll 

test: bootstrap
	./$(BOOTSTRAP_TARGET) ../tests/vixc0/let_and_print.vix -o test
	./test >/dev/null

pytest: bootstrap
	python3 -m pytest ../tests/vixc0_tests/*.py

.PHONY: all bootstrap clean test pytest
