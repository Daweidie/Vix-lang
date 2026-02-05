#ifndef BYTECODE_H
#define BYTECODE_H
#include "ast.h"

typedef enum {
    BC_LOAD_CONST_INT,
    BC_LOAD_CONST_FLOAT,
    BC_LOAD_CONST_STRING,
    BC_LOAD_NAME,
    BC_STORE_NAME,
    BC_PRINT,
    BC_INPUT,
    BC_TOINT,
    BC_TOFLOAT,
    BC_ADD,
    BC_SUB,   
    BC_MUL,
    BC_DIV,
    BC_MOD,
    BC_POW,
    BC_CONCAT,
    BC_REPEAT,
    BC_NEG,
    BC_POS,
    BC_EQ,
    BC_NE,
    BC_LT,
    BC_LE,
    BC_GT,
    BC_GE,
    BC_JUMP,
    BC_JUMP_IF_FALSE,
    BC_BREAK,
    BC_CONTINUE,
    BC_FOR_PREPARE,
    BC_FOR_LOOP,
    BC_FUNCTION_DEF,
    BC_CALL,
    BC_RETURN,
    BC_ADDRESS,
    BC_DEREF
} ByteCodeInstruction;

typedef struct {
    int* arg_indices;
    int arg_count;
} PrintArgs;

typedef struct {
    int address;
} JumpArgs;
typedef struct {
    int result;
    int operand1;
    int operand2; 
} TriAddrOperands;
typedef struct {
    char* name;
    int* param_indices;
    int param_count;
    int entry_point;
} FunctionDefArgs;
typedef struct {
    char* name;
    int* arg_indices;
    int arg_count;
    int result_index;
} CallArgs;

typedef struct {
    ByteCodeInstruction op;
    union {
        long long int_value;
        double float_value;
        char* string_value;
        int var_index;
        PrintArgs print_args;
        JumpArgs jump_args;
        TriAddrOperands triaddr;
        FunctionDefArgs func_def_args;
        CallArgs call_args;
    } operand;
} ByteCode;

typedef struct {
    ByteCode* codes;
    int count;
    int capacity;
} ByteCodeList;

typedef struct {
    ByteCodeList* bytecode;
    char** variables;
    int var_count;
    int var_capacity;
    int tmp_counter;  
} ByteCodeGen;

ByteCodeList* create_bytecode_list();
void free_bytecode_list(ByteCodeList* list);
ByteCodeGen* create_bytecode_gen();
void free_bytecode_gen(ByteCodeGen* gen);
void generate_bytecode(ByteCodeGen* gen, ASTNode* node);
void generate_bytecode_program(ByteCodeGen* gen, ASTNode* node);  
void generate_bytecode_print(ByteCodeGen* gen, ASTNode* node);   
int get_variable_index(ByteCodeGen* gen, const char* name);
void print_bytecode(ByteCodeList* list);

#endif // BYTECODE_H