#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS 1024

typedef struct {
    char name[64];
    LLVMValueRef value;
} VarEntry;

static VarEntry vars[MAX_VARS];
static int var_count = 0;

void vix_reset_vars(void) {
    var_count = 0;
}

void vix_set_var(const char *name, LLVMValueRef value) {
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, 63);
        vars[var_count].name[63] = '\0';
        vars[var_count].value = value;
        var_count++;
    }
}

LLVMValueRef vix_get_var(const char *name) {
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    return NULL;
}
LLVMTypeRef vix_LLVMFunctionType0(LLVMTypeRef ret_ty, int is_var_arg) {
    return LLVMFunctionType(ret_ty, NULL, 0, is_var_arg);
}

LLVMTypeRef vix_LLVMFunctionType1(LLVMTypeRef ret_ty, LLVMTypeRef p0, int is_var_arg) {
    LLVMTypeRef params[1] = {p0};
    return LLVMFunctionType(ret_ty, params, 1, is_var_arg);
}

LLVMTypeRef vix_LLVMFunctionType2(LLVMTypeRef ret_ty, LLVMTypeRef p0, LLVMTypeRef p1, int is_var_arg) {
    LLVMTypeRef params[2] = {p0, p1};
    return LLVMFunctionType(ret_ty, params, 2, is_var_arg);
}

LLVMTypeRef vix_LLVMFunctionTypeI32N(LLVMTypeRef ret_ty, unsigned param_count, int is_var_arg) {
    LLVMTypeRef params[256];
    if (param_count > 256) param_count = 256;
    for (unsigned i = 0; i < param_count; i++) {
        params[i] = LLVMInt32Type();
    }
    return LLVMFunctionType(ret_ty, params, param_count, is_var_arg);
}

LLVMValueRef vix_LLVMConstInt(LLVMTypeRef ty, long long val, int sign_extend) {
    return LLVMConstInt(ty, (unsigned long long)val, sign_extend);
}

LLVMValueRef vix_LLVMAddFunction(LLVMModuleRef m, const char *name, LLVMTypeRef ty) {
    return LLVMAddFunction(m, name, ty);
}

LLVMValueRef vix_LLVMGetParam(LLVMValueRef fn, unsigned index) {
    return LLVMGetParam(fn, index);
}

LLVMValueRef vix_LLVMGetNamedFunction(LLVMModuleRef m, const char *name) {
    return LLVMGetNamedFunction(m, name);
}

LLVMValueRef vix_LLVMGetNamedGlobal(LLVMModuleRef m, const char *name) {
    return LLVMGetNamedGlobal(m, name);
}

LLVMValueRef vix_LLVMAddGlobal(LLVMModuleRef m, LLVMTypeRef ty, const char *name) {
    return LLVMAddGlobal(m, ty, name);
}

LLVMValueRef vix_LLVMConstString(const char *str, unsigned length, int dont_null_terminate) {
    return LLVMConstString(str, length, dont_null_terminate);
}

LLVMTypeRef vix_LLVMArrayType(LLVMTypeRef element_type, unsigned element_count) {
    return LLVMArrayType(element_type, element_count);
}

LLVMValueRef vix_LLVMAppendBasicBlock(LLVMValueRef fn, const char *name) {
    return (LLVMValueRef)LLVMAppendBasicBlock(fn, name);
}

LLVMValueRef vix_LLVMAppendBasicBlockInContext(LLVMContextRef ctx, LLVMValueRef fn, const char *name) {
    return (LLVMValueRef)LLVMAppendBasicBlockInContext(ctx, fn, name);
}

LLVMValueRef vix_LLVMBuildCall2(LLVMBuilderRef builder, LLVMTypeRef ty,
                                 LLVMValueRef fn, LLVMValueRef *args,
                                 unsigned num_args, const char *name) {
    return LLVMBuildCall2(builder, ty, fn, args, num_args, name);
}

LLVMValueRef vix_LLVMBuildGEP2(LLVMBuilderRef builder, LLVMTypeRef ty,
                                LLVMValueRef pointer, LLVMValueRef *indices,
                                unsigned num_indices, const char *name) {
    return LLVMBuildGEP2(builder, ty, pointer, indices, num_indices, name);
}

LLVMValueRef vix_LLVMBuildLoad2(LLVMBuilderRef builder, LLVMTypeRef ty,
                                 LLVMValueRef ptr_val, const char *name) {
    return LLVMBuildLoad2(builder, ty, ptr_val, name);
}

LLVMValueRef vix_LLVMBuildAlloca(LLVMBuilderRef builder, LLVMTypeRef ty, const char *name) {
    return LLVMBuildAlloca(builder, ty, name);
}

LLVMValueRef vix_LLVMBuildStore(LLVMBuilderRef builder, LLVMValueRef val, LLVMValueRef ptr) {
    return LLVMBuildStore(builder, val, ptr);
}

LLVMValueRef vix_LLVMBuildAdd(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildAdd(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildSub(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildSub(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildMul(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildMul(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildSDiv(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildSDiv(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildSRem(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildSRem(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildNeg(LLVMBuilderRef builder, LLVMValueRef val, const char *name) {
    return LLVMBuildNeg(builder, val, name);
}

LLVMValueRef vix_LLVMBuildRet(LLVMBuilderRef builder, LLVMValueRef val) {
    return LLVMBuildRet(builder, val);
}

LLVMValueRef vix_LLVMBuildRetVoid(LLVMBuilderRef builder) {
    return LLVMBuildRetVoid(builder);
}

LLVMValueRef vix_LLVMBuildICmp(LLVMBuilderRef builder, LLVMIntPredicate op,
                                LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildICmp(builder, op, l, r, name);
}

LLVMValueRef vix_LLVMBuildZExt(LLVMBuilderRef builder, LLVMValueRef val,
                                LLVMTypeRef dest_ty, const char *name) {
    return LLVMBuildZExt(builder, val, dest_ty, name);
}

LLVMValueRef vix_LLVMBuildBr(LLVMBuilderRef builder, LLVMValueRef dest) {
    return LLVMBuildBr(builder, (LLVMBasicBlockRef)dest);
}

LLVMValueRef vix_LLVMBuildCondBr(LLVMBuilderRef builder, LLVMValueRef cond,
                                  LLVMValueRef then_block, LLVMValueRef else_block) {
    return LLVMBuildCondBr(builder, cond, (LLVMBasicBlockRef)then_block, (LLVMBasicBlockRef)else_block);
}

void vix_LLVMPositionBuilderAtEnd(LLVMBuilderRef builder, LLVMValueRef block) {
    LLVMPositionBuilderAtEnd(builder, (LLVMBasicBlockRef)block);
}

LLVMValueRef vix_LLVMGetBasicBlockTerminator(LLVMValueRef block) {
    return LLVMGetBasicBlockTerminator((LLVMBasicBlockRef)block);
}

void vix_LLVMSetInitializer(LLVMValueRef global, LLVMValueRef val) {
    LLVMSetInitializer(global, val);
}

void vix_LLVMSetGlobalConstant(LLVMValueRef global, int is_constant) {
    LLVMSetGlobalConstant(global, is_constant);
}

void vix_LLVMSetLinkage(LLVMValueRef global, int linkage) {
    LLVMSetLinkage(global, (LLVMLinkage)linkage);
}
