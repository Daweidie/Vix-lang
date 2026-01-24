CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fno-stack-protector
CPPFLAGS = -I../include -D_POSIX_C_SOURCE=200809L
BISON = bison
FLEX = flex
TARGET = vixc
AST_SRC = ast/ast.c ast/type_inference.c
SEMANTIC_SRC = semantic/semantic.c
BYTECODE_SRC = bytecode/bytecode.c
COMPILER_SRC = compiler/backend-cpp/atc.c
PARSER_SRC = parser/parser.tab.c parser/lex.yy.c
IR_SRC = qbe-ir/ir.c vic-ir/mir.c
LLVM_SRC = compiler/backend-llvm/emit.c
OPT_SRC = vic-ir/opt/opt.c
UTILS_SRC = utils/error.c
SRC = main.c $(AST_SRC) $(SEMANTIC_SRC) $(BYTECODE_SRC) $(COMPILER_SRC) $(PARSER_SRC) $(IR_SRC) $(LLVM_SRC) $(OPT_SRC) $(UTILS_SRC)
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

parser/parser.tab.c parser/parser.tab.h: parser/parser.y
	cd parser && $(BISON) -d parser.y

parser/lex.yy.c: parser/lexer.l
	cd parser && $(FLEX) lexer.l
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
main.o: main.c ../include/ast.h ../include/parser.h ../include/bytecode.h ../include/compiler.h ../include/qbe-ir/ir.h ../include/vic-ir/mir.h ../include/semantic.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ast/ast.o: ast/ast.c ../include/ast.h parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ast/type_inference.o: ast/type_inference.c ../include/type_inference.h ../include/bytecode.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

semantic/semantic.o: semantic/semantic.c ../include/semantic.h ../include/type_inference.h parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

bytecode/bytecode.o: bytecode/bytecode.c ../include/bytecode.h ../include/ast.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

utils/error.o: utils/error.c ../include/compiler.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

compiler/backend-cpp/atc.o: compiler/backend-cpp/atc.c ../include/compiler.h ../include/bytecode.h ../include/type_inference.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

compiler/backend-llvm/emit.o: compiler/backend-llvm/emit.c ../include/llvm_emit.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

qbe-ir/ir.o: qbe-ir/ir.c ../include/qbe-ir/ir.h ../include/bytecode.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

vic-ir/mir.o: vic-ir/mir.c ../include/vic-ir/mir.h ../include/bytecode.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

vic-ir/opt/opt.o: vic-ir/opt/opt.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

parser/parser.tab.o: parser/parser.tab.c parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

parser/lex.yy.o: parser/lex.yy.c parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) 
	rm -f parser/parser.tab.c parser/parser.tab.h parser/lex.yy.c

.PHONY: all clean install uninstall
