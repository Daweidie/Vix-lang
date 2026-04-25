CC = clang
CXX = clang
CFLAGS = -Wall -Wextra -std=c11 -fno-stack-protector -Werror
CXXFLAGS = -Wall -Wextra -std=c++17 -fno-stack-protector -lstdc++
CPPFLAGS = -I../include -D_POSIX_C_SOURCE=200809L
BISON = bison
FLEX = flex
LLVM_CONFIG = llvm-config
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs)
TARGET = vixc
AST_SRC = ast/ast.c ast/typeinfer.c
SEMANTIC_SRC = semantic/semantic.c
PARSER_SRC = parser/parser.tab.c parser/lex.yy.c
LLVM_SRC = compiler/CodeGen.cpp
UTILS_SRC = utils/error.c
C_SRC = main.c $(AST_SRC) $(SEMANTIC_SRC) $(PARSER_SRC) $(OPT_SRC) $(UTILS_SRC)
CXX_SRC = $(LLVM_SRC)
C_OBJ = $(C_SRC:.c=.o)
CXX_OBJ = $(CXX_SRC:.cpp=.o)
OBJ = $(C_OBJ) $(CXX_OBJ)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LLVM_LDFLAGS) -o $@ $^ $(LLVM_LIBS) -lm

parser/parser.tab.c parser/parser.tab.h: parser/parser.y
	cd parser && $(BISON) -d parser.y

parser/lex.yy.c: parser/lexer.l
	cd parser && $(FLEX) lexer.l

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LLVM_CFLAGS) $(CPPFLAGS) -c $< -o $@

main.o: main.c ../include/ast.h ../include/parser.h ../include/compiler.h  ../include/semantic.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ast/ast.o: ast/ast.c ../include/ast.h parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ast/typeinfer.o: ast/typeinfer.c ../include/typeinfer.h 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

semantic/semantic.o: semantic/semantic.c ../include/semantic.h ../include/typeinfer.h parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

utils/error.o: utils/error.c ../include/compiler.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

compiler/CodeGen.o: compiler/CodeGen.cpp ../include/codegen.h
	$(CXX) $(CXXFLAGS) $(LLVM_CFLAGS) $(CPPFLAGS) -c $< -o $@

parser/parser.tab.o: parser/parser.tab.c parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

parser/lex.yy.o: parser/lex.yy.c parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(C_OBJ) $(CXX_OBJ)
	rm -f parser/parser.tab.c parser/parser.tab.h parser/lex.yy.c

.PHONY: all clean install uninstall