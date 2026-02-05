#ifndef TYPE_INFERENCE_H
#define TYPE_INFERENCE_H
#include "bytecode.h"
#include <stdio.h>
typedef enum {
    TYPE_UNKNOWN,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_LIST,
    TYPE_POINTER      // 添加指针类型
} InferredType;
typedef struct {
    char* name;
    InferredType type;
    InferredType element_type;
    InferredType pointer_target_type;  // 指针指向的类型
} VariableInfo;
typedef struct {
    VariableInfo* variables;
    int count;
    int capacity;
} TypeInferenceContext;
TypeInferenceContext* create_type_inference_context();
void free_type_inference_context(TypeInferenceContext* ctx);
InferredType infer_type(TypeInferenceContext* ctx, ASTNode* node);
InferredType get_variable_type(TypeInferenceContext* ctx, const char* var_name);
void set_variable_type(TypeInferenceContext* ctx, const char* var_name, InferredType type);
void set_variable_list_type(TypeInferenceContext* ctx, const char* var_name, InferredType list_type, InferredType element_type);
void set_variable_pointer_type(TypeInferenceContext* ctx, const char* var_name, InferredType target_type);  // 设置指针类型
InferredType get_variable_pointer_target_type(TypeInferenceContext* ctx, const char* var_name);  // 获取指针指向的类型
const char* type_to_cpp_string(InferredType type);
int has_variable(TypeInferenceContext* ctx, const char* var_name);
#endif//TYPE_INFERENCE_H