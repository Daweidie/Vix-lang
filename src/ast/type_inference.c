#include "../include/type_inference.h"
#include "../include/bytecode.h"
#include "../include/compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern const char* current_input_filename;

TypeInferenceContext* create_type_inference_context() {
    TypeInferenceContext* ctx = malloc(sizeof(TypeInferenceContext));
    ctx->variables = NULL;
    ctx->count = 0;
    ctx->capacity = 0;
    return ctx;
}

void free_type_inference_context(TypeInferenceContext* ctx) {
    if (!ctx) return;
    
    for (int i = 0; i < ctx->count; i++) {
        free(ctx->variables[i].name);
    }
    free(ctx->variables);
    free(ctx);
}

InferredType infer_type(TypeInferenceContext* ctx, ASTNode* node) {
    if (!node) return TYPE_UNKNOWN;
    
    switch (node->type) {
        case AST_NUM_INT:
            return TYPE_INT;
            
        case AST_NUM_FLOAT:
            return TYPE_FLOAT;
            
        case AST_STRING:
            return TYPE_STRING;
            
        case AST_IDENTIFIER: {
            InferredType type = get_variable_type(ctx, node->data.identifier.name);
            if (type == TYPE_UNKNOWN) {
                report_undefined_variable_with_location(
                    node->data.identifier.name,
                    current_input_filename ? current_input_filename : "unknown",
                    node->location.first_line
                );
            }
            return type;
        }
            
        case AST_BINOP: {
            InferredType left_type = infer_type(ctx, node->data.binop.left);
            InferredType right_type = infer_type(ctx, node->data.binop.right);
            
            if (left_type == TYPE_STRING || right_type == TYPE_STRING) {
                return TYPE_STRING;
            }
            if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
                return TYPE_FLOAT;
            }
            if (left_type == TYPE_INT || right_type == TYPE_INT) {
                if (node->data.binop.op == OP_DIV) {
                    return TYPE_FLOAT;
                }
                return TYPE_INT;
            }
            
            return TYPE_UNKNOWN;
        }
            
        case AST_UNARYOP: {
            return infer_type(ctx, node->data.unaryop.expr);
        }
        
        case AST_INPUT: {
            return TYPE_STRING;
        }
        
        case AST_TOINT: {
            return TYPE_INT;
        }
        
        case AST_TOFLOAT: {
            return TYPE_FLOAT;
        }
        
        case AST_ASSIGN: {
            InferredType right_type = infer_type(ctx, node->data.assign.right);
            if (node->data.assign.left->type == AST_IDENTIFIER) {
                set_variable_type(ctx, node->data.assign.left->data.identifier.name, right_type);
            }
            return right_type;
        }
        case AST_CONST: {
            InferredType right_type = infer_type(ctx, node->data.assign.right);
            if (node->data.assign.left->type == AST_IDENTIFIER) {
                set_variable_type(ctx, node->data.assign.left->data.identifier.name, right_type);
            }
            return right_type;
        }
            
        default:
            return TYPE_UNKNOWN;
    }
}

InferredType get_variable_type(TypeInferenceContext* ctx, const char* var_name) {
    if (!ctx || !var_name) return TYPE_UNKNOWN;
    
    for (int i = 0; i < ctx->count; i++) {
        if (strcmp(ctx->variables[i].name, var_name) == 0) {
            return ctx->variables[i].type;
        }
    }
    
    return TYPE_UNKNOWN;
}

void set_variable_type(TypeInferenceContext* ctx, const char* var_name, InferredType type) {
    if (!ctx || !var_name) return;
    for (int i = 0; i < ctx->count; i++) {
        if (strcmp(ctx->variables[i].name, var_name) == 0) {
            ctx->variables[i].type = type;
            return;
        }
    }
    if (ctx->count >= ctx->capacity) {
        ctx->capacity = ctx->capacity == 0 ? 10 : ctx->capacity * 2;
        ctx->variables = realloc(ctx->variables, sizeof(VariableInfo) * ctx->capacity);
    }
    
    ctx->variables[ctx->count].name = malloc(strlen(var_name) + 1);
    strcpy(ctx->variables[ctx->count].name, var_name);
    ctx->variables[ctx->count].type = type;
    ctx->count++;
}

const char* type_to_cpp_string(InferredType type) {
    switch (type) {
        case TYPE_INT:
            return "long long";
        case TYPE_FLOAT:
            return "float";
        case TYPE_STRING:
            return "VString";
        default:
            return "auto"; 
    }
}

InferredType infer_type_from_value(const char* value) {
    if (!value) return TYPE_UNKNOWN;
    if (strlen(value) == 0) {
        return TYPE_STRING;
    }
    
    int has_quote = 0;
    int has_non_digit = 0;
    int has_dot = 0;
    
    for (int i = 0; value[i] != '\0'; i++) {
        if (value[i] == '"' || value[i] == '\'') {
            has_quote = 1;
        } else if (value[i] == '.') {
            has_dot = 1;
        } else if (!isdigit(value[i]) && value[i] != '-' && value[i] != '+') {
            has_non_digit = 1;
        }
    }
    
    if (has_quote) {
        return TYPE_STRING;
    }
    
    if (has_non_digit) {
        return TYPE_STRING;
    }
    
    if (has_dot) {
        return TYPE_FLOAT;
    }
    
    return TYPE_INT;
}