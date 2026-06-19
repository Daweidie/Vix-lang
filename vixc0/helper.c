#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <string.h>

#define MAX_VARS 1024

typedef struct {
    char name[64];
    LLVMValueRef value;
    char type[16];
} VarEntry;

typedef struct {
    char name[64];
    char return_type[16];
    char param_types[16][16];
    int param_count;
    int is_var_arg;
} FunctionEntry;

static VarEntry vars[MAX_VARS];
static int var_count = 0;
static FunctionEntry funcs[MAX_VARS];
static int func_count = 0;

void vix_reset_vars(void) {
    var_count = 0;
}

void vix_set_var(const char *name, LLVMValueRef value) {
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, 63);
        vars[var_count].name[63] = '\0';
        vars[var_count].value = value;
        vars[var_count].type[0] = '\0';
        var_count++;
    }
}

void vix_set_var_type(const char *name, const char *type) {
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(vars[i].name, name) == 0) {
            strncpy(vars[i].type, type, 15);
            vars[i].type[15] = '\0';
            return;
        }
    }
}

const char *vix_get_var_type(const char *name) {
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(vars[i].name, name) == 0) {
            if (vars[i].type[0] == '\0') return "i32";
            return vars[i].type;
        }
    }
    return "i32";
}

LLVMValueRef vix_get_var(const char *name) {
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    return NULL;
}

void vix_reset_function_sigs(void) {
    func_count = 0;
}

void vix_register_function_sig_vararg(const char *name, const char *return_type,
                                      const char **param_types, int param_count,
                                      int is_var_arg) {
    int idx = -1;
    for (int i = 0; i < func_count; i++) {
        if (strcmp(funcs[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        if (func_count >= MAX_VARS) return;
        idx = func_count++;
    }
    strncpy(funcs[idx].name, name, 63);
    funcs[idx].name[63] = '\0';
    strncpy(funcs[idx].return_type, return_type, 15);
    funcs[idx].return_type[15] = '\0';
    if (param_count > 16) param_count = 16;
    funcs[idx].param_count = param_count;
    funcs[idx].is_var_arg = is_var_arg;
    for (int i = 0; i < param_count; i++) {
        strncpy(funcs[idx].param_types[i], param_types[i], 15);
        funcs[idx].param_types[i][15] = '\0';
    }
}

void vix_register_function_sig(const char *name, const char *return_type,
                               const char **param_types, int param_count) {
    vix_register_function_sig_vararg(name, return_type, param_types, param_count, 0);
}

const char *vix_get_function_return_type(const char *name) {
    for (int i = func_count - 1; i >= 0; i--) {
        if (strcmp(funcs[i].name, name) == 0) {
            return funcs[i].return_type;
        }
    }
    return "i32";
}

const char *vix_get_function_param_type(const char *name, int index) {
    for (int i = func_count - 1; i >= 0; i--) {
        if (strcmp(funcs[i].name, name) == 0) {
            if (index >= 0 && index < funcs[i].param_count) {
                return funcs[i].param_types[index];
            }
            return "i32";
        }
    }
    return "i32";
}

int vix_get_function_param_count(const char *name) {
    for (int i = func_count - 1; i >= 0; i--) {
        if (strcmp(funcs[i].name, name) == 0) {
            return funcs[i].param_count;
        }
    }
    return 0;
}

int vix_get_function_is_var_arg(const char *name) {
    for (int i = func_count - 1; i >= 0; i--) {
        if (strcmp(funcs[i].name, name) == 0) {
            return funcs[i].is_var_arg;
        }
    }
    return 0;
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

LLVMTypeRef vix_LLVMFunctionTypeTypedN(LLVMTypeRef ret_ty, LLVMTypeRef *params,
                                       unsigned param_count, int is_var_arg) {
    if (param_count > 256) param_count = 256;
    return LLVMFunctionType(ret_ty, params, param_count, is_var_arg);
}

LLVMValueRef vix_LLVMConstInt(LLVMTypeRef ty, long long val, int sign_extend) {
    return LLVMConstInt(ty, (unsigned long long)val, sign_extend);
}

LLVMValueRef vix_LLVMConstReal(LLVMTypeRef ty, double val) {
    return LLVMConstReal(ty, val);
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

LLVMValueRef vix_LLVMBuildFAdd(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildFAdd(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFSub(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildFSub(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFMul(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildFMul(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFDiv(LLVMBuilderRef builder, LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildFDiv(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildNeg(LLVMBuilderRef builder, LLVMValueRef val, const char *name) {
    return LLVMBuildNeg(builder, val, name);
}

LLVMValueRef vix_LLVMBuildFNeg(LLVMBuilderRef builder, LLVMValueRef val, const char *name) {
    return LLVMBuildFNeg(builder, val, name);
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

LLVMValueRef vix_LLVMBuildFCmp(LLVMBuilderRef builder, LLVMRealPredicate op,
                                LLVMValueRef l, LLVMValueRef r, const char *name) {
    return LLVMBuildFCmp(builder, op, l, r, name);
}

LLVMValueRef vix_LLVMBuildZExt(LLVMBuilderRef builder, LLVMValueRef val,
                                LLVMTypeRef dest_ty, const char *name) {
    return LLVMBuildZExt(builder, val, dest_ty, name);
}

LLVMValueRef vix_LLVMBuildSIToFP(LLVMBuilderRef builder, LLVMValueRef val,
                                  LLVMTypeRef dest_ty, const char *name) {
    return LLVMBuildSIToFP(builder, val, dest_ty, name);
}

LLVMValueRef vix_LLVMBuildFPExt(LLVMBuilderRef builder, LLVMValueRef val,
                                 LLVMTypeRef dest_ty, const char *name) {
    return LLVMBuildFPExt(builder, val, dest_ty, name);
}

LLVMValueRef vix_LLVMBuildFPTrunc(LLVMBuilderRef builder, LLVMValueRef val,
                                   LLVMTypeRef dest_ty, const char *name) {
    return LLVMBuildFPTrunc(builder, val, dest_ty, name);
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
