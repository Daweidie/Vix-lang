#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "ast.h"
#include "type_inference.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_CONSTANT,
    SYMBOL_FUNCTION
} SymbolType;

typedef struct Symbol {
    char* name;
    SymbolType type;
    InferredType inferred_type;
    struct Symbol* next;
} Symbol;

typedef struct SymbolTable {
    Symbol* head;
    struct SymbolTable* parent;
} SymbolTable;

SymbolTable* create_symbol_table(SymbolTable* parent);
void destroy_symbol_table(SymbolTable* symbol_table);
int add_symbol(SymbolTable* table, const char* name, SymbolType type, InferredType inferred_type);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
int check_undefined_symbols(ASTNode* node);
int check_undefined_symbols_in_node(ASTNode* node, SymbolTable* table);
int check_unused_variables(ASTNode* node, SymbolTable* table);
int is_variable_used_in_node(ASTNode* node, const char* var_name);

#endif // SEMANTIC_H

