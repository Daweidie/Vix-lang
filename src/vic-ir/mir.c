#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "../include/ast.h"
#include "../include/vic-ir/mir.h"
typedef struct {
    char** strings;
    int* string_regs;
    int string_count;
    int string_capacity;
    
    long long* ints;
    int* int_regs;
    int int_count;
    int int_capacity;
    
    double* floats;
    int* float_regs;
    int float_count;
    int float_capacity;
} ConstantPool;
#define MAX_RECURSION_DEPTH 1000
static int recursion_depth = 0;

void preprocess_ast_for_constants(ConstantPool* pool, ASTNode* node) {
    if (!node) return;
    if (recursion_depth >= MAX_RECURSION_DEPTH) {
        return;
    }
    recursion_depth++;
    
    switch (node->type) {
        case AST_NUM_INT: {
            add_int_constant(pool, node->data.num_int.value);
            break;
        }
        case AST_NUM_FLOAT: {
            add_float_constant(pool, node->data.num_float.value);
            break;
        }
        case AST_STRING: {
            add_string_constant(pool, node->data.string.value);
            break;
        }
        case AST_BINOP: {
            preprocess_ast_for_constants(pool, node->data.binop.left);
            preprocess_ast_for_constants(pool, node->data.binop.right);
            break;
        }
        case AST_UNARYOP: {
            preprocess_ast_for_constants(pool, node->data.unaryop.expr);
            break;
        }
        case AST_CALL: {
            if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST) {
                for (int i = 0; i < node->data.call.args->data.expression_list.expression_count; i++) {
                    preprocess_ast_for_constants(pool, 
                                                node->data.call.args->data.expression_list.expressions[i]);
                }
            }
            if (node->data.call.func) {
                preprocess_ast_for_constants(pool, node->data.call.func);
            }
            break;
        }
        case AST_INPUT: {
            preprocess_ast_for_constants(pool, node->data.input.prompt);
            break;
        }
        case AST_TOINT: {
            preprocess_ast_for_constants(pool, node->data.toint.expr);
            break;
        }
        case AST_TOFLOAT: {
            preprocess_ast_for_constants(pool, node->data.tofloat.expr);
            break;
        }
        case AST_PRINT: {
            preprocess_ast_for_constants(pool, node->data.print.expr);
            break;
        }
        case AST_ASSIGN: {
            preprocess_ast_for_constants(pool, node->data.assign.right);
            if (node->data.assign.left) {
                preprocess_ast_for_constants(pool, node->data.assign.left);
            }
            break;
        }
        case AST_CONST: {
            preprocess_ast_for_constants(pool, node->data.assign.right);
            if (node->data.assign.left) {
                preprocess_ast_for_constants(pool, node->data.assign.left);
            }
            break;
        }
        case AST_IF: {
            preprocess_ast_for_constants(pool, node->data.if_stmt.condition);
            preprocess_ast_for_constants(pool, node->data.if_stmt.then_body);
            if (node->data.if_stmt.else_body) {
                preprocess_ast_for_constants(pool, node->data.if_stmt.else_body);
            }
            break;
        }
        case AST_WHILE: {
            preprocess_ast_for_constants(pool, node->data.while_stmt.condition);
            preprocess_ast_for_constants(pool, node->data.while_stmt.body);
            break;
        }
        case AST_FOR: {
            preprocess_ast_for_constants(pool, node->data.for_stmt.start);
            preprocess_ast_for_constants(pool, node->data.for_stmt.end);
            preprocess_ast_for_constants(pool, node->data.for_stmt.body);
            if (node->data.for_stmt.var) {
                preprocess_ast_for_constants(pool, node->data.for_stmt.var);
            }
            break;
        }
        case AST_RETURN: {
            if (node->data.return_stmt.expr) {
                preprocess_ast_for_constants(pool, node->data.return_stmt.expr);
            }
            break;
        }
        case AST_PROGRAM: {
            for (int i = 0; i < node->data.program.statement_count; i++) {
                preprocess_ast_for_constants(pool, node->data.program.statements[i]);
            }
            break;
        }
        case AST_FUNCTION: {
            if (node->data.function.body) {
                preprocess_ast_for_constants(pool, node->data.function.body);
            }
            break;
        }
        default:
            break;
    }
    recursion_depth--;
}

static int reg_counter = 0;
static int label_counter = 0;

static char* escape_str(const char* src);
int add_string_constant(ConstantPool* pool, const char* str);
int add_int_constant(ConstantPool* pool, long long value);
int add_float_constant(ConstantPool* pool, double value);
int generate_expr(ConstantPool* pool, ASTNode* node, FILE* fp);
ConstantPool* create_constant_pool() {
    ConstantPool* pool = malloc(sizeof(ConstantPool));
    pool->string_count = 0;
    pool->string_capacity = 16;
    pool->strings = malloc(sizeof(char*) * pool->string_capacity);
    pool->string_regs = malloc(sizeof(int) * pool->string_capacity);
    
    pool->int_count = 0;
    pool->int_capacity = 16;
    pool->ints = malloc(sizeof(long long) * pool->int_capacity);
    pool->int_regs = malloc(sizeof(int) * pool->int_capacity);
    
    pool->float_count = 0;
    pool->float_capacity = 16;
    pool->floats = malloc(sizeof(double) * pool->float_capacity);
    pool->float_regs = malloc(sizeof(int) * pool->float_capacity);
    
    return pool;
}
void free_constant_pool(ConstantPool* pool) {
    for (int i = 0; i < pool->string_count; i++) {
        free(pool->strings[i]);
    }
    free(pool->strings);
    free(pool->string_regs);
    free(pool->ints);
    free(pool->int_regs);
    free(pool->floats);
    free(pool->float_regs);
    free(pool);
}
void generate_data_section(ConstantPool* pool, FILE* fp) {
    for (int i = 0; i < pool->string_count; i++) {
        char* esc = escape_str(pool->strings[i]);
        fprintf(fp, "data $s%d = { i32 0, [ %zu * i8 ] \"%s\" }\n", 
                pool->string_regs[i], strlen(pool->strings[i]) + 1, esc ? esc : "");
        if (esc) free(esc);
    }
    for (int i = 0; i < pool->int_count; i++) {
        fprintf(fp, "data $i%d = { i64 %lld }\n", pool->int_regs[i], pool->ints[i]);
    }
    for (int i = 0; i < pool->float_count; i++) {
        fprintf(fp, "data $f%d = { f64 %g }\n", pool->float_regs[i], pool->floats[i]);
    }
}

int add_string_constant(ConstantPool* pool, const char* str) {
    for (int i = 0; i < pool->string_count; i++) {
        if (strcmp(pool->strings[i], str) == 0) {
            return pool->string_regs[i];
        }
    }
    if (pool->string_count >= pool->string_capacity) {
        pool->string_capacity *= 2;
        pool->strings = realloc(pool->strings, sizeof(char*) * pool->string_capacity);
        pool->string_regs = realloc(pool->string_regs, sizeof(int) * pool->string_capacity);
    }
    pool->strings[pool->string_count] = strdup(str);
    int reg = reg_counter++;
    pool->string_regs[pool->string_count] = reg;
    pool->string_count++;
    return reg;
}
int add_int_constant(ConstantPool* pool, long long value) {
    for (int i = 0; i < pool->int_count; i++) {
        if (pool->ints[i] == value) {
            return pool->int_regs[i];
        }
    }
    if (pool->int_count >= pool->int_capacity) {
        pool->int_capacity *= 2;
        pool->ints = realloc(pool->ints, sizeof(long long) * pool->int_capacity);
        pool->int_regs = realloc(pool->int_regs, sizeof(int) * pool->int_capacity);
    }
    pool->ints[pool->int_count] = value;
    int reg = reg_counter++;
    pool->int_regs[pool->int_count] = reg;
    pool->int_count++;
    return reg;
}
int add_float_constant(ConstantPool* pool, double value) {
    for (int i = 0; i < pool->float_count; i++) {
        if (pool->floats[i] == value) {
            return pool->float_regs[i];
        }
    }
    if (pool->float_count >= pool->float_capacity) {
        pool->float_capacity *= 2;
        pool->floats = realloc(pool->floats, sizeof(double) * pool->float_capacity);
        pool->float_regs = realloc(pool->float_regs, sizeof(int) * pool->float_capacity);
    }
    pool->floats[pool->float_count] = value;
    int reg = reg_counter++;
    pool->float_regs[pool->float_count] = reg;
    pool->float_count++;
    return reg;
}

static char* escape_str(const char* src) {
    if (!src) return NULL;
    size_t len = 0;
    for (const char* p = src; *p; p++) {
        if (*p == '"' || *p == '\\' || *p == '\n' || *p == '\r' || *p == '\t')
            len += 2;
        else
            len += 1;
    }
    char* out = malloc(len + 1);
    if (!out) return NULL;
    char* d = out;
    for (const char* p = src; *p; p++) {
        switch (*p) {
            case '"': *d++ = '\\'; *d++ = '"'; break;
            case '\\': *d++ = '\\'; *d++ = '\\'; break;
            case '\n': *d++ = '\\'; *d++ = 'n'; break;
            case '\r': *d++ = '\\'; *d++ = 'r'; break;
            case '\t': *d++ = '\\'; *d++ = 't'; break;
            default: *d++ = *p; break;
        }
    }
    *d = 0;
    return out;
}
int generate_expr(ConstantPool* pool, ASTNode* node, FILE* fp);

int generate_expr(ConstantPool* pool, ASTNode* node, FILE* fp) {
    if (!node) return -1;
    
    switch (node->type) {
        case AST_NUM_INT: {
            int reg = add_int_constant(pool, node->data.num_int.value);
            fprintf(fp, "    r%d = load_data $i%d\n", reg, reg);
            return reg;
        }
        case AST_NUM_FLOAT: {
            int reg = add_float_constant(pool, node->data.num_float.value);
            fprintf(fp, "    r%d = load_data $f%d\n", reg, reg);
            return reg;
        }
        case AST_STRING: {
            int reg = add_string_constant(pool, node->data.string.value);
            fprintf(fp, "    r%d = load_data $s%d\n", reg, reg);
            return reg;
        }
        case AST_IDENTIFIER: {
            int reg = reg_counter++;
            fprintf(fp, "    r%d = load_name %s\n", reg, node->data.identifier.name);
            return reg;
        }
        case AST_BINOP: {
            int left_reg = generate_expr(pool, node->data.binop.left, fp);
            int right_reg = generate_expr(pool, node->data.binop.right, fp);
            
            int result_reg = reg_counter++;
            switch (node->data.binop.op) {
                case OP_ADD:
                    fprintf(fp, "    r%d = add r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_SUB:
                    fprintf(fp, "    r%d = sub r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_MUL:
                    fprintf(fp, "    r%d = mul r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_DIV:
                    fprintf(fp, "    r%d = div r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_MOD:
                    fprintf(fp, "    r%d = mod r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_POW:
                    fprintf(fp, "    r%d = pow r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_EQ:
                    fprintf(fp, "    r%d = eq r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_NE:
                    fprintf(fp, "    r%d = ne r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_LT:
                    fprintf(fp, "    r%d = lt r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_LE:
                    fprintf(fp, "    r%d = le r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_GT:
                    fprintf(fp, "    r%d = gt r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_GE:
                    fprintf(fp, "    r%d = ge r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_CONCAT:
                    fprintf(fp, "    r%d = concat r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                case OP_REPEAT:
                    fprintf(fp, "    r%d = repeat r%d, r%d\n", result_reg, left_reg, right_reg);
                    break;
                default:
                    break;
            }
            return result_reg;
        }
        case AST_UNARYOP: {
            int expr_reg = generate_expr(pool, node->data.unaryop.expr, fp);
            int result_reg = reg_counter++;
            switch (node->data.unaryop.op) {
                case OP_MINUS:
                    fprintf(fp, "    r%d = neg r%d\n", result_reg, expr_reg);
                    break;
                case OP_PLUS:
                    fprintf(fp, "    r%d = copy r%d\n", result_reg, expr_reg);
                    break;
                default:
                    break;
            }
            return result_reg;
        }
        case AST_CALL: {
            int result_reg = reg_counter++;
            int* arg_regs = NULL;
            int arg_count = 0;
            
            if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST) {
                arg_count = node->data.call.args->data.expression_list.expression_count;
                arg_regs = malloc(sizeof(int) * arg_count);
                
                for (int i = 0; i < arg_count; i++) {
                    arg_regs[i] = generate_expr(pool, 
                                                node->data.call.args->data.expression_list.expressions[i], 
                                                fp);
                }
            }
            char* func_name = NULL;
            if (node->data.call.func->type == AST_IDENTIFIER) {
                func_name = node->data.call.func->data.identifier.name;
            } else {
                fprintf(fp, "    ; error: function name not an identifier\n");
                if (arg_regs) free(arg_regs);
                return result_reg;
            }
            fprintf(fp, "    r%d = call %s(", result_reg, func_name);
            for (int i = 0; i < arg_count; i++) {
                if (i > 0) fprintf(fp, ", ");
                fprintf(fp, "r%d", arg_regs[i]);
            }
            fprintf(fp, ")\n");
            
            if (arg_regs) free(arg_regs);
            return result_reg;
        }
        case AST_INPUT: {
            int prompt_reg = generate_expr(pool, node->data.input.prompt, fp);
            int result_reg = reg_counter++;
            fprintf(fp, "    r%d = call input(r%d)\n", result_reg, prompt_reg);
            return result_reg;
        }
        case AST_TOINT: {
            int expr_reg = generate_expr(pool, node->data.toint.expr, fp);
            int result_reg = reg_counter++;
            fprintf(fp, "    r%d = call toint(r%d)\n", result_reg, expr_reg);
            return result_reg;
        }
        case AST_TOFLOAT: {
            int expr_reg = generate_expr(pool, node->data.tofloat.expr, fp);
            int result_reg = reg_counter++;
            fprintf(fp, "    r%d = call tofloat(r%d)\n", result_reg, expr_reg);
            return result_reg;
        }
        default:
            fprintf(fp, "    ;unknown expr\n");
            return reg_counter++;
    }
}

void generate_statement(ConstantPool* pool, ASTNode* node, FILE* fp) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PRINT: {
            int expr_reg = generate_expr(pool, node->data.print.expr, fp);
            fprintf(fp, "    call print(r%d)\n", expr_reg);
            break;
        }
        case AST_ASSIGN: {
            int right_reg = generate_expr(pool, node->data.assign.right, fp);
            if (node->data.assign.left->type == AST_IDENTIFIER) {
                fprintf(fp, "    store_name %s, r%d\n", node->data.assign.left->data.identifier.name, right_reg);
            }
            break;
        }
        case AST_CONST: {
            int right_reg = generate_expr(pool, node->data.assign.right, fp);
            if (node->data.assign.left->type == AST_IDENTIFIER) {
                fprintf(fp, "    store_name %s, r%d\n", node->data.assign.left->data.identifier.name, right_reg);
            }
            break;
        }
        case AST_IF: {
            int cond_reg = generate_expr(pool, node->data.if_stmt.condition, fp);
            
            int else_label = label_counter++;
            int end_label = label_counter++;
            fprintf(fp, "    jmp_if_false r%d, @L%d\n", cond_reg, else_label);
            generate_statement(pool, node->data.if_stmt.then_body, fp);
            fprintf(fp, "    jmp @L%d\n", end_label);
            fprintf(fp, "  @L%d:\n", else_label);
            if (node->data.if_stmt.else_body) {
                generate_statement(pool, node->data.if_stmt.else_body, fp);
            }
            fprintf(fp, "  @L%d:\n", end_label);
            break;
        }
        case AST_WHILE: {
            int start_label = label_counter++;
            int body_label = label_counter++;
            int end_label = label_counter++;
            
            fprintf(fp, "    jmp @L%d\n", start_label);
            fprintf(fp, "  @L%d:\n", body_label);
            
            generate_statement(pool, node->data.while_stmt.body, fp);
            
            fprintf(fp, "  @L%d:\n", start_label);
            int cond_reg = generate_expr(pool, node->data.while_stmt.condition, fp);
            fprintf(fp, "    jmp_if_false r%d, @L%d\n", cond_reg, end_label);
            fprintf(fp, "    jmp @L%d\n", body_label);
            
            fprintf(fp, "  @L%d:\n", end_label);
            break;
        }
        case AST_FOR: {
            int start_reg = generate_expr(pool, node->data.for_stmt.start, fp);
            int end_reg = generate_expr(pool, node->data.for_stmt.end, fp);
            int loop_start = label_counter++;
            int loop_body = label_counter++;
            int loop_end = label_counter++;
            fprintf(fp, "    store_name %s, r%d\n", node->data.for_stmt.var->data.identifier.name, start_reg);
            fprintf(fp, "    jmp @L%d\n", loop_start);
            fprintf(fp, "  @L%d:\n", loop_body);
            generate_statement(pool, node->data.for_stmt.body, fp);
            int var_reg = reg_counter++;
            fprintf(fp, "    r%d = load_name %s\n", var_reg, node->data.for_stmt.var->data.identifier.name);
            int next_reg = reg_counter++;
            fprintf(fp, "    r%d = add r%d, 1\n", next_reg, var_reg);
            fprintf(fp, "    store_name %s, r%d\n", node->data.for_stmt.var->data.identifier.name, next_reg);
            fprintf(fp, "  @L%d:\n", loop_start);
            int current_reg = reg_counter++;
            fprintf(fp, "    r%d = load_name %s\n", current_reg, node->data.for_stmt.var->data.identifier.name);
            int cmp_reg = reg_counter++;
            fprintf(fp, "    r%d = le r%d, r%d\n", cmp_reg, current_reg, end_reg);
            fprintf(fp, "    jmp_if_false r%d, @L%d\n", cmp_reg, loop_end);
            fprintf(fp, "    jmp @L%d\n", loop_body);
            
            fprintf(fp, "  @L%d:\n", loop_end);
            break;
        }
        case AST_BREAK:
            fprintf(fp, "    break\n");
            break;
        case AST_CONTINUE:
            fprintf(fp, "    continue\n");
            break;
        case AST_RETURN: {
            if (node->data.return_stmt.expr) {
                int expr_reg = generate_expr(pool, node->data.return_stmt.expr, fp);
                fprintf(fp, "    ret r%d\n", expr_reg);
            } else {
                fprintf(fp, "    ret 0\n");
            }
            break;
        }
        case AST_PROGRAM: {
            for (int i = 0; i < node->data.program.statement_count; i++) {
                generate_statement(pool, node->data.program.statements[i], fp);
            }
            break;
        }
        case AST_FUNCTION: {
            fprintf(fp, "function %s(", node->data.function.name);
            if (node->data.function.params && node->data.function.params->type == AST_EXPRESSION_LIST) {
                int param_count = node->data.function.params->data.expression_list.expression_count;
                for (int i = 0; i < param_count; i++) {
                    if (i > 0) fprintf(fp, ", ");
                    if (node->data.function.params->data.expression_list.expressions[i]->type == AST_IDENTIFIER) {
                        fprintf(fp, "r%s", node->data.function.params->data.expression_list.expressions[i]->data.identifier.name);
                    }
                }
            }
            fprintf(fp, ") {\n");
            generate_statement(pool, node->data.function.body, fp);
            
            fprintf(fp, "  ret 0\n");
            fprintf(fp, "}\n\n");
            break;
        }
        default:
            break;
    }
}

//跳过main
void generate_functions(ConstantPool* pool, ASTNode* node, FILE* fp, int* has_main) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM: {
            for (int i = 0; i < node->data.program.statement_count; i++) {
                if (node->data.program.statements[i]->type == AST_FUNCTION) {
                    if (strcmp(node->data.program.statements[i]->data.function.name, "main") == 0) {
                        *has_main = 1;
                    } else {
                        generate_functions(pool, node->data.program.statements[i], fp, has_main);
                    }
                }
            }
            break;
        }
        case AST_FUNCTION: {
            fprintf(fp, "function %s(", node->data.function.name);
            if (node->data.function.params && node->data.function.params->type == AST_EXPRESSION_LIST) {
                int param_count = node->data.function.params->data.expression_list.expression_count;
                for (int i = 0; i < param_count; i++) {
                    if (i > 0) fprintf(fp, ", ");
                    if (node->data.function.params->data.expression_list.expressions[i]->type == AST_IDENTIFIER) {
                        fprintf(fp, "r%s", node->data.function.params->data.expression_list.expressions[i]->data.identifier.name);
                    }
                }
            }
            fprintf(fp, ") {\n");
            generate_statement(pool, node->data.function.body, fp);
            
            fprintf(fp, "  ret 0\n");
            fprintf(fp, "}\n\n");
            break;
        }
        default:
            break;
    }
}

void vic_gen(ASTNode* ast, FILE* fp) {
    if (!fp || !ast) return;
    
    reg_counter = 0;
    label_counter = 0;
    
    ConstantPool* pool = create_constant_pool();
    
    fprintf(fp, "; Vic ir from ast\n");
    preprocess_ast_for_constants(pool, ast);
    generate_data_section(pool, fp);
    int has_main = 0;
    generate_functions(pool, ast, fp, &has_main);
    fprintf(fp, "\nfunction main() {\n");
    if (ast && ast->type == AST_PROGRAM) {
        for (int i = 0; i < ast->data.program.statement_count; i++) {
            if (ast->data.program.statements[i]->type != AST_FUNCTION) {
                generate_statement(pool, ast->data.program.statements[i], fp);
            }
            else if (strcmp(ast->data.program.statements[i]->data.function.name, "main") == 0) {
                generate_statement(pool, ast->data.program.statements[i]->data.function.body, fp);
            }
        }
    } else {
        generate_statement(pool, ast, fp);
    }
    
    fprintf(fp, "    ret 0\n");
    fprintf(fp, "}\n");
    
    free_constant_pool(pool);
}
//\o/\o/\o/\o/\o/\o/