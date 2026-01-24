/*
实现了Velox语言的字节码生成系统，
包含创建字节码，
管理字节码列表，
将AST转换为字节码以及打印字节码等功能
*/
#include "../include/bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ByteCode* create_bytecode(ByteCodeInstruction op) {
    ByteCode* bc = malloc(sizeof(ByteCode));
    bc->op = op;
    return bc;
}

ByteCodeList* create_bytecode_list() {
    ByteCodeList* list = malloc(sizeof(ByteCodeList));
    list->codes = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

void free_bytecode_list(ByteCodeList* list) {
    if (!list) 
        return;
    for (int i = 0; i < list->count; i++) {
        if (list->codes[i].op == BC_LOAD_CONST_STRING) {
            free(list->codes[i].operand.string_value);
        } else if (list->codes[i].op == BC_PRINT) {
            if (list->codes[i].operand.print_args.arg_indices) {
                free(list->codes[i].operand.print_args.arg_indices);
            }
        }
    }
    
    free(list->codes);
    free(list);
}

ByteCodeGen* create_bytecode_gen() {
    ByteCodeGen* gen = malloc(sizeof(ByteCodeGen));
    gen->bytecode = create_bytecode_list();
    gen->variables = NULL;
    gen->var_count = 0;
    gen->var_capacity = 0;
    gen->tmp_counter = 0;  // 初始化临时变量计数器
    return gen;
}

void free_bytecode_gen(ByteCodeGen* gen) {
    if (!gen) return;
    free_bytecode_list(gen->bytecode);
    for (int i = 0; i < gen->var_count; i++) {
        free(gen->variables[i]);
    }
    free(gen->variables);
    
    free(gen);
}

int get_variable_index(ByteCodeGen* gen, const char* name) {
    for (int i = 0; i < gen->var_count; i++) {
        if (strcmp(gen->variables[i], name) == 0) {
            return i;
        }
    }
    if (gen->var_count >= gen->var_capacity) {
        gen->var_capacity = gen->var_capacity == 0 ? 10 : gen->var_capacity * 2;
        gen->variables = realloc(gen->variables, sizeof(char*) * gen->var_capacity);
    }
    
    gen->variables[gen->var_count] = malloc(strlen(name) + 1);
    strcpy(gen->variables[gen->var_count], name);
    return gen->var_count++;
}

void add_bytecode_full(ByteCodeGen* gen, ByteCode* bc) {
    if (gen->bytecode->count >= gen->bytecode->capacity) {
        gen->bytecode->capacity = gen->bytecode->capacity == 0 ? 10 : gen->bytecode->capacity * 2;
        gen->bytecode->codes = realloc(gen->bytecode->codes, 
                                      sizeof(ByteCode) * gen->bytecode->capacity);
    }
    
    gen->bytecode->codes[gen->bytecode->count] = *bc;
    gen->bytecode->count++;
    free(bc);
}

void add_bytecode(ByteCodeGen* gen, ByteCodeInstruction op) {
    if (gen->bytecode->count >= gen->bytecode->capacity) {
        gen->bytecode->capacity = gen->bytecode->capacity == 0 ? 10 : gen->bytecode->capacity * 2;
        gen->bytecode->codes = realloc(gen->bytecode->codes, 
                                      sizeof(ByteCode) * gen->bytecode->capacity);
    }
    
    gen->bytecode->codes[gen->bytecode->count].op = op;
    gen->bytecode->count++;
}

void generate_bytecode_num_int(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_NUM_INT) return;
    
    add_bytecode(gen, BC_LOAD_CONST_INT);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.int_value = node->data.num_int.value;
    gen->tmp_counter++;  
}

void generate_bytecode_num_float(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_NUM_FLOAT) return;
    
    add_bytecode(gen, BC_LOAD_CONST_FLOAT);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.float_value = node->data.num_float.value;
    gen->tmp_counter++; 
}

void generate_bytecode_string(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_STRING) return;
    
    add_bytecode(gen, BC_LOAD_CONST_STRING);
    size_t len = strlen(node->data.string.value);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.string_value = malloc(len + 1);
    strcpy(gen->bytecode->codes[gen->bytecode->count - 1].operand.string_value, node->data.string.value);
    gen->tmp_counter++; 
}

void generate_bytecode_identifier(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_IDENTIFIER) return;
    
    int var_index = get_variable_index(gen, node->data.identifier.name);
    add_bytecode(gen, BC_LOAD_NAME);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.var_index = var_index;
    gen->tmp_counter++;  
}

void generate_bytecode_input(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_INPUT) return;
    generate_bytecode(gen, node->data.input.prompt);
    add_bytecode(gen, BC_INPUT);
    gen->tmp_counter++;  
}

void generate_bytecode_toint(ByteCodeGen* gen, ASTNode* node) {
    generate_bytecode(gen, node->data.toint.expr);
    add_bytecode(gen, BC_TOINT);
    gen->tmp_counter++;  
}

void generate_bytecode_tofloat(ByteCodeGen* gen, ASTNode* node) {
    generate_bytecode(gen, node->data.tofloat.expr);
    add_bytecode(gen, BC_TOFLOAT);
    gen->tmp_counter++;  
}

void generate_bytecode_program(ByteCodeGen* gen, ASTNode* node) {
    for (int i = 0; i < node->data.program.statement_count; i++) {
        generate_bytecode(gen, node->data.program.statements[i]);
    }
}

void generate_bytecode_print(ByteCodeGen* gen, ASTNode* node) {
    if (node->data.print.expr->type == AST_EXPRESSION_LIST) {
        ASTNode* expr_list = node->data.print.expr;
        int count = expr_list->data.expression_list.expression_count;
        int* produced_indices = malloc(sizeof(int) * count);
        for (int i = 0; i < count; i++) {
            generate_bytecode(gen, expr_list->data.expression_list.expressions[i]);
            // 获取正确的寄存器索引而不是指令索引
            int last_instr_index = gen->bytecode->count - 1;
            ByteCode* last_instr = &gen->bytecode->codes[last_instr_index];
            if (last_instr->op == BC_LOAD_NAME) {
                produced_indices[i] = last_instr->operand.var_index;
            } else if (last_instr->op == BC_LOAD_CONST_INT || 
                       last_instr->op == BC_LOAD_CONST_FLOAT || 
                       last_instr->op == BC_LOAD_CONST_STRING) {
                produced_indices[i] = last_instr_index;
            } else if (last_instr->op >= BC_ADD && last_instr->op <= BC_GE) {
                produced_indices[i] = last_instr->operand.triaddr.result;
            } else {
                produced_indices[i] = last_instr_index;
            }
        }
        ByteCode* bc = create_bytecode(BC_PRINT);
        bc->operand.print_args.arg_count = count;
        bc->operand.print_args.arg_indices = produced_indices;
        
        add_bytecode_full(gen, bc);
    } else {
        generate_bytecode(gen, node->data.print.expr);
        ByteCode* bc = create_bytecode(BC_PRINT);
        bc->operand.print_args.arg_count = 1;
        bc->operand.print_args.arg_indices = malloc(sizeof(int) * 1);
        int last_instr_index = gen->bytecode->count - 1;
        ByteCode* last_instr = &gen->bytecode->codes[last_instr_index];
        if (last_instr->op == BC_LOAD_NAME) {
            bc->operand.print_args.arg_indices[0] = last_instr->operand.var_index;
        } else if (last_instr->op == BC_LOAD_CONST_INT || last_instr->op == BC_LOAD_CONST_FLOAT || last_instr->op == BC_LOAD_CONST_STRING) {
            bc->operand.print_args.arg_indices[0] = last_instr_index;
        } else if (last_instr->op >= BC_ADD && last_instr->op <= BC_GE) {
            bc->operand.print_args.arg_indices[0] = last_instr->operand.triaddr.result;
        } else {
            bc->operand.print_args.arg_indices[0] = last_instr_index;
        }
        
        add_bytecode_full(gen, bc);
    }
}

void generate_bytecode_expression_list(ByteCodeGen* gen, ASTNode* node) {
    for (int i = 0; i < node->data.expression_list.expression_count; i++) {
        generate_bytecode(gen, node->data.expression_list.expressions[i]);
    }
}

void generate_bytecode_assign(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_ASSIGN) return;
    generate_bytecode(gen, node->data.assign.right);
    if (node->data.assign.left->type == AST_IDENTIFIER) {
        int var_index = get_variable_index(gen, node->data.assign.left->data.identifier.name);
        add_bytecode(gen, BC_STORE_NAME);
        gen->bytecode->codes[gen->bytecode->count - 1].operand.var_index = var_index;
    }
}

void generate_bytecode_binop(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_BINOP) return;
    generate_bytecode(gen, node->data.binop.left);
    int left_index = gen->bytecode->count - 1;
    generate_bytecode(gen, node->data.binop.right);
    int right_index = gen->bytecode->count - 1;
    ByteCode* bc = create_bytecode(BC_ADD);
    bc->operand.triaddr.result = gen->tmp_counter;
    gen->tmp_counter++;
    
    switch (node->data.binop.op) {
        case OP_ADD:
            bc->op = BC_ADD;
            break;
        case OP_SUB:
            bc->op = BC_SUB;
            break;
        case OP_MUL:
            bc->op = BC_MUL;
            break;
        case OP_DIV:
            bc->op = BC_DIV;
            break;
        case OP_MOD:
            bc->op = BC_MOD;
            break;
        case OP_POW:
            bc->op = BC_POW;
            break;
        case OP_CONCAT:
            bc->op = BC_CONCAT;
            break;
        case OP_REPEAT:
            bc->op = BC_REPEAT;
            break;
        case OP_EQ:
            bc->op = BC_EQ;
            break;
        case OP_NE:
            bc->op = BC_NE;
            break;
        case OP_LT:
            bc->op = BC_LT;
            break;
        case OP_LE:
            bc->op = BC_LE;
            break;
        case OP_GT:
            bc->op = BC_GT;
            break;
        case OP_GE:
            bc->op = BC_GE;
            break;
    }
    
    /*获取操作数的寄存器索引
    对于LOAD_NAME指令，寄存器索引就是指令索引本身
    对于其他指令，寄存器索引存储在triaddr.result中*/
    if (gen->bytecode->codes[left_index].op == BC_LOAD_NAME) {
        bc->operand.triaddr.operand1 = gen->bytecode->codes[left_index].operand.var_index;
    } else {
        bc->operand.triaddr.operand1 = gen->bytecode->codes[left_index].operand.triaddr.result;
    }
    
    if (gen->bytecode->codes[right_index].op == BC_LOAD_NAME) {
        bc->operand.triaddr.operand2 = gen->bytecode->codes[right_index].operand.var_index;
    } else {
        bc->operand.triaddr.operand2 = gen->bytecode->codes[right_index].operand.triaddr.result;
    }
    
    add_bytecode_full(gen, bc);
}

void generate_bytecode_unaryop(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_UNARYOP) return;
    generate_bytecode(gen, node->data.unaryop.expr);
    
    ByteCode* bc = create_bytecode(BC_NEG); // 默认值
    bc->operand.triaddr.result = gen->tmp_counter;
    bc->operand.triaddr.operand1 = gen->bytecode->count - 1;
    bc->operand.triaddr.operand2 = -1; // 一元运算只需要一个操作数
    
    switch (node->data.unaryop.op) {
        case OP_MINUS:
            bc->op = BC_NEG;
            break;
        case OP_PLUS:
            bc->op = BC_POS;
            break;
    }
    
    add_bytecode_full(gen, bc);
    gen->tmp_counter++;
}

void generate_bytecode_if(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_IF) return;
    generate_bytecode(gen, node->data.if_stmt.condition);
    add_bytecode(gen, BC_JUMP_IF_FALSE);
    int jump_if_false_index = gen->bytecode->count - 1;
    generate_bytecode(gen, node->data.if_stmt.then_body);
    
    if (node->data.if_stmt.else_body) {
        add_bytecode(gen, BC_JUMP);
        int jump_index = gen->bytecode->count - 1;
        gen->bytecode->codes[jump_if_false_index].operand.jump_args.address = gen->bytecode->count;
        generate_bytecode(gen, node->data.if_stmt.else_body);
        gen->bytecode->codes[jump_index].operand.jump_args.address = gen->bytecode->count;
    } else {
        gen->bytecode->codes[jump_if_false_index].operand.jump_args.address = gen->bytecode->count;
    }
}

void generate_bytecode_while(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_WHILE) return;
    int loop_start = gen->bytecode->count;
    generate_bytecode(gen, node->data.while_stmt.condition);
    add_bytecode(gen, BC_JUMP_IF_FALSE);
    int jump_if_false_index = gen->bytecode->count - 1;
    generate_bytecode(gen, node->data.while_stmt.body);
    add_bytecode(gen, BC_JUMP);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.jump_args.address = loop_start;
    gen->bytecode->codes[jump_if_false_index].operand.jump_args.address = gen->bytecode->count;
}

void generate_bytecode_break(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_BREAK) return;
    add_bytecode(gen, BC_BREAK);
}

void generate_bytecode_continue(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_CONTINUE) return;
    add_bytecode(gen, BC_CONTINUE);
}

void generate_bytecode_for(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_FOR) return;
    generate_bytecode(gen, node->data.for_stmt.var);
    generate_bytecode(gen, node->data.for_stmt.start);
    add_bytecode(gen, BC_STORE_NAME);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.var_index = get_variable_index(gen, node->data.for_stmt.var->data.identifier.name);
    int loop_start = gen->bytecode->count;
    generate_bytecode(gen, node->data.for_stmt.var);
    generate_bytecode(gen, node->data.for_stmt.end);
    ByteCode* cmp_bc = create_bytecode(BC_LE);
    cmp_bc->operand.triaddr.result = gen->tmp_counter;
    cmp_bc->operand.triaddr.operand1 = gen->bytecode->count - 2; // var
    cmp_bc->operand.triaddr.operand2 = gen->bytecode->count - 1; // end
    add_bytecode_full(gen, cmp_bc);
    gen->tmp_counter++;
    add_bytecode(gen, BC_JUMP_IF_FALSE);
    int jump_if_false_index = gen->bytecode->count - 1;
    generate_bytecode(gen, node->data.for_stmt.body);
    generate_bytecode(gen, node->data.for_stmt.var);
    add_bytecode(gen, BC_LOAD_CONST_INT);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.int_value = 1;
    ByteCode* add_bc = create_bytecode(BC_ADD);
    add_bc->operand.triaddr.result = gen->tmp_counter;
    add_bc->operand.triaddr.operand1 = gen->bytecode->count - 2; // var
    add_bc->operand.triaddr.operand2 = gen->bytecode->count - 1; // 1
    add_bytecode_full(gen, add_bc);
    gen->tmp_counter++;
    add_bytecode(gen, BC_STORE_NAME);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.var_index = get_variable_index(gen, node->data.for_stmt.var->data.identifier.name);
    add_bytecode(gen, BC_JUMP);
    gen->bytecode->codes[gen->bytecode->count - 1].operand.jump_args.address = loop_start;
    gen->bytecode->codes[jump_if_false_index].operand.jump_args.address = gen->bytecode->count;
}

void generate_bytecode_function(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_FUNCTION) return;
    ByteCode* bc = create_bytecode(BC_FUNCTION_DEF);
    bc->operand.func_def_args.name = malloc(strlen(node->data.function.name) + 1);
    strcpy(bc->operand.func_def_args.name, node->data.function.name);
    if (node->data.function.params && node->data.function.params->type == AST_EXPRESSION_LIST) {
        bc->operand.func_def_args.param_count = node->data.function.params->data.expression_list.expression_count;
        bc->operand.func_def_args.param_indices = malloc(sizeof(int) * bc->operand.func_def_args.param_count);
        
        for (int i = 0; i < bc->operand.func_def_args.param_count; i++) {
            ASTNode* param = node->data.function.params->data.expression_list.expressions[i];
            if (param->type == AST_IDENTIFIER) {
                bc->operand.func_def_args.param_indices[i] = get_variable_index(gen, param->data.identifier.name);
            }
        }
    } else {
        bc->operand.func_def_args.param_count = 0;
        bc->operand.func_def_args.param_indices = NULL;
    }
    bc->operand.func_def_args.entry_point = gen->bytecode->count + 1;
    add_bytecode_full(gen, bc);
    if (node->data.function.body) {
        generate_bytecode(gen, node->data.function.body);
    }
}

void generate_bytecode_call(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_CALL) return;
    ByteCode* bc = create_bytecode(BC_CALL);
    if (node->data.call.func->type == AST_IDENTIFIER) {
        bc->operand.call_args.name = malloc(strlen(node->data.call.func->data.identifier.name) + 1);
        strcpy(bc->operand.call_args.name, node->data.call.func->data.identifier.name);
    }
    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST) {
        bc->operand.call_args.arg_count = node->data.call.args->data.expression_list.expression_count;
        bc->operand.call_args.arg_indices = malloc(sizeof(int) * bc->operand.call_args.arg_count);
        for (int i = 0; i < bc->operand.call_args.arg_count; i++) {
            generate_bytecode(gen, node->data.call.args->data.expression_list.expressions[i]);
            bc->operand.call_args.arg_indices[i] = gen->bytecode->count - 1;
        }
    } else {
        bc->operand.call_args.arg_count = 0;
        bc->operand.call_args.arg_indices = NULL;
    }
    bc->operand.call_args.result_index = gen->bytecode->count;
    
    add_bytecode_full(gen, bc);
}

void generate_bytecode_return(ByteCodeGen* gen, ASTNode* node) {
    if (node->type != AST_RETURN) return;
    if (node->data.assign.right) {
        generate_bytecode(gen, node->data.assign.right);
    }
    add_bytecode(gen, BC_RETURN);
}

void generate_bytecode(ByteCodeGen* gen, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            generate_bytecode_program(gen, node);
            break;
        case AST_PRINT:
            generate_bytecode_print(gen, node);
            break;
        case AST_CONST:
            generate_bytecode_assign(gen, node);
            break;
        case AST_ASSIGN:
            generate_bytecode_assign(gen, node);
            break;
        case AST_BINOP:
            generate_bytecode_binop(gen, node);
            break;
        case AST_UNARYOP:
            generate_bytecode_unaryop(gen, node);
            break;
        case AST_NUM_INT:
            generate_bytecode_num_int(gen, node);
            break;
        case AST_NUM_FLOAT:
            generate_bytecode_num_float(gen, node);
            break;
        case AST_STRING:
            generate_bytecode_string(gen, node);
            break;
        case AST_IDENTIFIER:
            generate_bytecode_identifier(gen, node);
            break;
        case AST_IF:
            generate_bytecode_if(gen, node);
            break;
        case AST_WHILE:
            generate_bytecode_while(gen, node);
            break;
        case AST_FOR:
            generate_bytecode_for(gen, node);
            break;
        case AST_BREAK:
            generate_bytecode_break(gen, node);
            break;
        case AST_CONTINUE:
            generate_bytecode_continue(gen, node);
            break;
        case AST_EXPRESSION_LIST:
            generate_bytecode_expression_list(gen, node);
            break;
        case AST_FUNCTION:
            generate_bytecode_function(gen, node);
            break;
        case AST_CALL:
            generate_bytecode_call(gen, node);
            break;
        case AST_RETURN:
            generate_bytecode_return(gen, node);
            break;
        default:
            break;
    }
}

void print_bytecode(ByteCodeList* list) {
    printf("Bytecode:\n");
    for (int i = 0; i < list->count; i++) {
        printf("%4d: ", i);
        switch (list->codes[i].op) {
            case BC_LOAD_CONST_INT:
                printf("LOAD_CONST_INT %lld\n", list->codes[i].operand.int_value);
                break;
            case BC_LOAD_CONST_FLOAT:
                printf("LOAD_CONST_FLOAT %f\n", list->codes[i].operand.float_value);
                break;
            case BC_LOAD_CONST_STRING:
                printf("LOAD_CONST_STRING \"%s\"\n", list->codes[i].operand.string_value);
                break;
            case BC_LOAD_NAME:
                printf("LOAD_NAME %d\n", list->codes[i].operand.var_index);
                break;
            case BC_STORE_NAME:
                printf("STORE_NAME %d\n", list->codes[i].operand.var_index);
                break;
            case BC_INPUT:
                printf("INPUT\n");
                break;
            case BC_TOINT:
                printf("TOINT\n");
                break;
            case BC_TOFLOAT:
                printf("TOFLOAT\n");
                break;
            case BC_PRINT:
                printf("PRINT");
                if (list->codes[i].operand.print_args.arg_count > 0) {
                    for (int j = 0; j < list->codes[i].operand.print_args.arg_count; j++) {
                        printf(" %d", list->codes[i].operand.print_args.arg_indices[j]);
                    }
                }
                printf("\n");
                break;
            case BC_ADD:
                printf("ADD %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_SUB:
                printf("SUB %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_MUL:
                printf("MUL %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_DIV:
                printf("DIV %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_MOD:
                printf("MOD %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_POW:
                printf("POW %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_CONCAT:
                printf("CONCAT %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_REPEAT:
                printf("REPEAT %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_NEG:
                printf("NEG %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1);
                break;
            case BC_POS:
                printf("POS %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1);
                break;
            case BC_EQ:
                printf("EQ %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_NE:
                printf("NE %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_LT:
                printf("LT %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_LE:
                printf("LE %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_GT:
                printf("GT %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_GE:
                printf("GE %%r%d, %%r%d, %%r%d\n", 
                       list->codes[i].operand.triaddr.result,
                       list->codes[i].operand.triaddr.operand1,
                       list->codes[i].operand.triaddr.operand2);
                break;
            case BC_JUMP:
                printf("JUMP %d\n", list->codes[i].operand.jump_args.address);
                break;
            case BC_JUMP_IF_FALSE:
                printf("JUMP_IF_FALSE %d (condition: %d)\n", 
                       list->codes[i].operand.jump_args.address, 
                       list->codes[i].operand.var_index);
                break;
            case BC_BREAK:
                printf("BREAK\n");
                break;
            case BC_CONTINUE:
                printf("CONTINUE\n");
                break;
            case BC_FOR_PREPARE:
                printf("FOR_PREPARE\n");
                break;
            case BC_FOR_LOOP:
                printf("FOR_LOOP\n");
                break;
            case BC_FUNCTION_DEF:
                printf("FUNCTION_DEF %s (entry: %d)", 
                       list->codes[i].operand.func_def_args.name,
                       list->codes[i].operand.func_def_args.entry_point);
                if (list->codes[i].operand.func_def_args.param_count > 0) {
                    printf(" params:");
                    for (int j = 0; j < list->codes[i].operand.func_def_args.param_count; j++) {
                        printf(" %d", list->codes[i].operand.func_def_args.param_indices[j]);
                    }
                }
                printf("\n");
                break;
            case BC_CALL:
                printf("CALL %s -> %d", 
                       list->codes[i].operand.call_args.name,
                       list->codes[i].operand.call_args.result_index);
                if (list->codes[i].operand.call_args.arg_count > 0) {
                    printf(" args:");
                    for (int j = 0; j < list->codes[i].operand.call_args.arg_count; j++) {
                        printf(" %d", list->codes[i].operand.call_args.arg_indices[j]);
                    }
                }
                printf("\n");
                break;
            case BC_RETURN:
                printf("RETURN\n");
                break;
            default:
                printf("UNKNOWN (%d)\n", list->codes[i].op);
                break;
        }
    }
}
