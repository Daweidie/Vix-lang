#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <string.h>
#include <stdlib.h>

#define MAX_VARS 1024
#define MAX_FIELDS 32
#define NAME_SIZE 64
#define TYPE_SIZE 64

typedef struct {
  char name[NAME_SIZE];
  LLVMValueRef value;
  char type[TYPE_SIZE];
} VarEntry;

typedef struct {
  char name[NAME_SIZE];
  char return_type[TYPE_SIZE];
  char param_types[16][TYPE_SIZE];
  int param_count;
  int is_var_arg;
} FunctionEntry;

typedef struct {
  char name[NAME_SIZE];
  char field_names[MAX_FIELDS][NAME_SIZE];
  char field_types[MAX_FIELDS][TYPE_SIZE];
  int field_count;
  LLVMTypeRef llvm_type;
  int body_set;
} StructEntry;

static VarEntry vars[MAX_VARS];
static int var_count = 0;
static FunctionEntry funcs[MAX_VARS];
static int func_count = 0;
static StructEntry structs[MAX_VARS];
static int struct_count = 0;

const char *vix_diag_red(void) { return "\033[31m"; }

const char *vix_diag_yellow(void) { return "\033[33m"; }

const char *vix_diag_reset(void) { return "\033[0m"; }

void vix_reset_vars(void) { var_count = 0; }

void vix_set_var(const char *name, LLVMValueRef value) {
  if (var_count < MAX_VARS) {
    strncpy(vars[var_count].name, name, NAME_SIZE - 1);
    vars[var_count].name[NAME_SIZE - 1] = '\0';
    vars[var_count].value = value;
    vars[var_count].type[0] = '\0';
    var_count++;
  }
}

void vix_set_var_type(const char *name, const char *type) {
  for (int i = var_count - 1; i >= 0; i--) {
    if (strcmp(vars[i].name, name) == 0) {
      strncpy(vars[i].type, type, TYPE_SIZE - 1);
      vars[i].type[TYPE_SIZE - 1] = '\0';
      return;
    }
  }
}

const char *vix_get_var_type(const char *name) {
  for (int i = var_count - 1; i >= 0; i--) {
    if (strcmp(vars[i].name, name) == 0) {
      if (vars[i].type[0] == '\0')
        return "i32";
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

void vix_reset_function_sigs(void) { func_count = 0; }

static int vix_find_struct_index(const char *name) {
  for (int i = struct_count - 1; i >= 0; i--) {
    if (strcmp(structs[i].name, name) == 0)
      return i;
  }
  return -1;
}

void vix_reset_struct_sigs(void) { struct_count = 0; }

void vix_declare_struct_sig(const char *name) {
  if (vix_find_struct_index(name) >= 0)
    return;
  if (struct_count >= MAX_VARS)
    return;
  int idx = struct_count++;
  strncpy(structs[idx].name, name, NAME_SIZE - 1);
  structs[idx].name[NAME_SIZE - 1] = '\0';
  structs[idx].field_count = 0;
  structs[idx].llvm_type = LLVMStructCreateNamed(LLVMGetGlobalContext(), name);
  structs[idx].body_set = 0;
}

int vix_is_struct_type(const char *name) {
  return vix_find_struct_index(name) >= 0;
}

LLVMTypeRef vix_get_llvm_type_for_struct(const char *name) {
  int idx = vix_find_struct_index(name);
  if (idx >= 0)
    return structs[idx].llvm_type;
  return LLVMInt32Type();
}

static LLVMTypeRef vix_llvm_type_for_name(const char *type) {
  if (strcmp(type, "void") == 0)
    return LLVMVoidType();
  if (strcmp(type, "bool") == 0)
    return LLVMInt32Type();
  if (strcmp(type, "i64") == 0)
    return LLVMInt64Type();
  if (strcmp(type, "f32") == 0)
    return LLVMFloatType();
  if (strcmp(type, "f64") == 0)
    return LLVMDoubleType();
  if (type[0] == '[' || strncmp(type, "Option[", 7) == 0) {
    return LLVMPointerType(LLVMInt8Type(), 0);
  }
  if (strcmp(type, "string") == 0 || strcmp(type, "ptr") == 0 ||
      strncmp(type, "ptr:", 4) == 0) {
    return LLVMPointerType(LLVMInt8Type(), 0);
  }
  int idx = vix_find_struct_index(type);
  if (idx >= 0)
    return structs[idx].llvm_type;
  return LLVMInt32Type();
}

void vix_register_struct_sig(const char *name, const char **field_names,
                             const char **field_types, int field_count) {
  vix_declare_struct_sig(name);
  int idx = vix_find_struct_index(name);
  if (idx < 0)
    return;
  if (field_count > MAX_FIELDS)
    field_count = MAX_FIELDS;
  structs[idx].field_count = field_count;
  LLVMTypeRef llvm_fields[MAX_FIELDS];
  for (int i = 0; i < field_count; i++) {
    strncpy(structs[idx].field_names[i], field_names[i], NAME_SIZE - 1);
    structs[idx].field_names[i][NAME_SIZE - 1] = '\0';
    strncpy(structs[idx].field_types[i], field_types[i], TYPE_SIZE - 1);
    structs[idx].field_types[i][TYPE_SIZE - 1] = '\0';
    llvm_fields[i] = vix_llvm_type_for_name(field_types[i]);
  }
  if (!structs[idx].body_set) {
    LLVMStructSetBody(structs[idx].llvm_type, llvm_fields, field_count, 0);
    structs[idx].body_set = 1;
  }
}

int vix_get_struct_field_index_in_struct(int struct_idx, const char *field_name);
int vix_find_field_in_any_struct(const char *field_name);

int vix_get_struct_field_index(const char *struct_name,
                                const char *field_name) {
  const char *actual_struct = struct_name;
  if (strncmp(struct_name, "ptr:", 4) == 0)
    actual_struct = struct_name + 4;
  int idx = vix_find_struct_index(actual_struct);
  if (idx < 0) {
    if (strncmp(struct_name, "ptr:", 4) == 0) {
      int fi = vix_find_field_in_any_struct(field_name);
      if (fi >= 0) return vix_get_struct_field_index_in_struct(fi, field_name);
    }
    return -1;
  }
  for (int i = 0; i < structs[idx].field_count; i++) {
    if (strcmp(structs[idx].field_names[i], field_name) == 0)
      return i;
  }
  return -1;
}

const char *vix_get_struct_field_type(const char *struct_name,
                                       const char *field_name) {
  const char *actual_struct = struct_name;
  if (strncmp(struct_name, "ptr:", 4) == 0)
    actual_struct = struct_name + 4;
  if (strncmp(struct_name, "ptr<", 4) == 0) {
    actual_struct = struct_name + 4;
    int len = strlen(actual_struct);
    if (len > 0 && actual_struct[len - 1] == '>')
      ; /* handled by copying to temp below */
  }
  int idx = vix_find_struct_index(actual_struct);
  if (idx < 0) {
    if (strncmp(struct_name, "ptr:", 4) == 0 || strncmp(struct_name, "ptr<", 4) == 0) {
      /* Search all structs for the field as fallback */
      int fi = vix_find_field_in_any_struct(field_name);
      if (fi >= 0) {
        return structs[fi].field_types[vix_get_struct_field_index_in_struct(fi, field_name)];
      }
    }
    return "unknown";
  }
  for (int i = 0; i < structs[idx].field_count; i++) {
    if (strcmp(structs[idx].field_names[i], field_name) == 0)
      return structs[idx].field_types[i];
  }
  return "unknown";
}

int vix_get_struct_field_index_in_struct(int struct_idx, const char *field_name) {
  for (int i = 0; i < structs[struct_idx].field_count; i++) {
    if (strcmp(structs[struct_idx].field_names[i], field_name) == 0)
      return i;
  }
  return -1;
}

int vix_find_field_in_any_struct(const char *field_name) {
  for (int s = 0; s < struct_count; s++) {
    for (int i = 0; i < structs[s].field_count; i++) {
      if (strcmp(structs[s].field_names[i], field_name) == 0)
        return s;
    }
  }
  return -1;
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
    if (func_count >= MAX_VARS)
      return;
    idx = func_count++;
  }
  strncpy(funcs[idx].name, name, NAME_SIZE - 1);
  funcs[idx].name[NAME_SIZE - 1] = '\0';
  strncpy(funcs[idx].return_type, return_type, TYPE_SIZE - 1);
  funcs[idx].return_type[TYPE_SIZE - 1] = '\0';
  if (param_count > 16)
    param_count = 16;
  funcs[idx].param_count = param_count;
  funcs[idx].is_var_arg = is_var_arg;
  for (int i = 0; i < param_count; i++) {
    strncpy(funcs[idx].param_types[i], param_types[i], TYPE_SIZE - 1);
    funcs[idx].param_types[i][TYPE_SIZE - 1] = '\0';
  }
}

void vix_register_function_sig(const char *name, const char *return_type,
                               const char **param_types, int param_count) {
  vix_register_function_sig_vararg(name, return_type, param_types, param_count,
                                   0);
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

LLVMTypeRef vix_LLVMFunctionType1(LLVMTypeRef ret_ty, LLVMTypeRef p0,
                                  int is_var_arg) {
  LLVMTypeRef params[1] = {p0};
  return LLVMFunctionType(ret_ty, params, 1, is_var_arg);
}

LLVMTypeRef vix_LLVMFunctionType2(LLVMTypeRef ret_ty, LLVMTypeRef p0,
                                  LLVMTypeRef p1, int is_var_arg) {
  LLVMTypeRef params[2] = {p0, p1};
  return LLVMFunctionType(ret_ty, params, 2, is_var_arg);
}

LLVMTypeRef vix_LLVMFunctionTypeI32N(LLVMTypeRef ret_ty, unsigned param_count,
                                     int is_var_arg) {
  LLVMTypeRef params[256];
  if (param_count > 256)
    param_count = 256;
  for (unsigned i = 0; i < param_count; i++) {
    params[i] = LLVMInt32Type();
  }
  return LLVMFunctionType(ret_ty, params, param_count, is_var_arg);
}

LLVMTypeRef vix_LLVMFunctionTypeTypedN(LLVMTypeRef ret_ty, LLVMTypeRef *params,
                                       unsigned param_count, int is_var_arg) {
  if (param_count > 256)
    param_count = 256;
  return LLVMFunctionType(ret_ty, params, param_count, is_var_arg);
}

LLVMValueRef vix_LLVMConstInt(LLVMTypeRef ty, long long val, int sign_extend) {
  return LLVMConstInt(ty, (unsigned long long)val, sign_extend);
}

LLVMValueRef vix_LLVMConstReal(LLVMTypeRef ty, double val) {
  return LLVMConstReal(ty, val);
}

LLVMValueRef vix_LLVMGetUndef(LLVMTypeRef ty) { return LLVMGetUndef(ty); }

LLVMValueRef vix_LLVMAddFunction(LLVMModuleRef m, const char *name,
                                 LLVMTypeRef ty) {
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

LLVMValueRef vix_LLVMAddGlobal(LLVMModuleRef m, LLVMTypeRef ty,
                               const char *name) {
  return LLVMAddGlobal(m, ty, name);
}

LLVMValueRef vix_LLVMConstString(const char *str, unsigned length,
                                 int dont_null_terminate) {
  return LLVMConstString(str, length, dont_null_terminate);
}

LLVMTypeRef vix_LLVMArrayType(LLVMTypeRef element_type,
                              unsigned element_count) {
  return LLVMArrayType(element_type, element_count);
}

LLVMValueRef vix_LLVMAppendBasicBlock(LLVMValueRef fn, const char *name) {
  return (LLVMValueRef)LLVMAppendBasicBlock(fn, name);
}

LLVMValueRef vix_LLVMAppendBasicBlockInContext(LLVMContextRef ctx,
                                               LLVMValueRef fn,
                                               const char *name) {
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

LLVMValueRef vix_LLVMBuildAlloca(LLVMBuilderRef builder, LLVMTypeRef ty,
                                 const char *name) {
  return LLVMBuildAlloca(builder, ty, name);
}

LLVMValueRef vix_LLVMBuildStore(LLVMBuilderRef builder, LLVMValueRef val,
                                LLVMValueRef ptr) {
  return LLVMBuildStore(builder, val, ptr);
}

LLVMValueRef vix_LLVMBuildInsertValue(LLVMBuilderRef builder, LLVMValueRef agg,
                                      LLVMValueRef val, unsigned index,
                                      const char *name) {
  return LLVMBuildInsertValue(builder, agg, val, index, name);
}

LLVMValueRef vix_LLVMBuildExtractValue(LLVMBuilderRef builder, LLVMValueRef agg,
                                       unsigned index, const char *name) {
  return LLVMBuildExtractValue(builder, agg, index, name);
}

LLVMValueRef vix_LLVMBuildAdd(LLVMBuilderRef builder, LLVMValueRef l,
                              LLVMValueRef r, const char *name) {
  return LLVMBuildAdd(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildSub(LLVMBuilderRef builder, LLVMValueRef l,
                              LLVMValueRef r, const char *name) {
  return LLVMBuildSub(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildMul(LLVMBuilderRef builder, LLVMValueRef l,
                              LLVMValueRef r, const char *name) {
  return LLVMBuildMul(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildSDiv(LLVMBuilderRef builder, LLVMValueRef l,
                               LLVMValueRef r, const char *name) {
  return LLVMBuildSDiv(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildSRem(LLVMBuilderRef builder, LLVMValueRef l,
                               LLVMValueRef r, const char *name) {
  return LLVMBuildSRem(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFAdd(LLVMBuilderRef builder, LLVMValueRef l,
                               LLVMValueRef r, const char *name) {
  return LLVMBuildFAdd(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFSub(LLVMBuilderRef builder, LLVMValueRef l,
                               LLVMValueRef r, const char *name) {
  return LLVMBuildFSub(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFMul(LLVMBuilderRef builder, LLVMValueRef l,
                               LLVMValueRef r, const char *name) {
  return LLVMBuildFMul(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildFDiv(LLVMBuilderRef builder, LLVMValueRef l,
                               LLVMValueRef r, const char *name) {
  return LLVMBuildFDiv(builder, l, r, name);
}

LLVMValueRef vix_LLVMBuildNeg(LLVMBuilderRef builder, LLVMValueRef val,
                              const char *name) {
  return LLVMBuildNeg(builder, val, name);
}

LLVMValueRef vix_LLVMBuildFNeg(LLVMBuilderRef builder, LLVMValueRef val,
                               const char *name) {
  return LLVMBuildFNeg(builder, val, name);
}

LLVMValueRef vix_LLVMBuildRet(LLVMBuilderRef builder, LLVMValueRef val) {
  return LLVMBuildRet(builder, val);
}

LLVMValueRef vix_LLVMBuildRetVoid(LLVMBuilderRef builder) {
  return LLVMBuildRetVoid(builder);
}

LLVMValueRef vix_LLVMBuildICmp(LLVMBuilderRef builder, LLVMIntPredicate op,
                               LLVMValueRef l, LLVMValueRef r,
                               const char *name) {
  return LLVMBuildICmp(builder, op, l, r, name);
}

LLVMValueRef vix_LLVMBuildFCmp(LLVMBuilderRef builder, LLVMRealPredicate op,
                               LLVMValueRef l, LLVMValueRef r,
                               const char *name) {
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

LLVMValueRef vix_LLVMBuildBitCast(LLVMBuilderRef builder, LLVMValueRef val,
                                   LLVMTypeRef dest_ty, const char *name) {
  return LLVMBuildBitCast(builder, val, dest_ty, name);
}

LLVMValueRef vix_LLVMBuildBr(LLVMBuilderRef builder, LLVMValueRef dest) {
  return LLVMBuildBr(builder, (LLVMBasicBlockRef)dest);
}

LLVMValueRef vix_LLVMBuildCondBr(LLVMBuilderRef builder, LLVMValueRef cond,
                                 LLVMValueRef then_block,
                                 LLVMValueRef else_block) {
  return LLVMBuildCondBr(builder, cond, (LLVMBasicBlockRef)then_block,
                         (LLVMBasicBlockRef)else_block);
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

int vix_array_len(void *arr) {
  if (arr == NULL) return 0;
  int *header = (int *)((char *)arr - 8);
  return *header;
}

void *vix_array_push_i32(void *arr, int val) {
  int old_len = vix_array_len(arr);
  int new_len = old_len + 1;
  void *base = (arr == NULL) ? NULL : (void *)((char *)arr - 8);
  size_t data_bytes = new_len * sizeof(int);
  size_t total_bytes = 8 + data_bytes;
  void *new_block = realloc(base, total_bytes);
  if (new_block == NULL) return NULL;
  *(int *)new_block = new_len;
  int *data = (int *)((char *)new_block + 8);
  data[old_len] = val;
  return (void *)((char *)new_block + 8);
}

void *vix_string_concat(const char *a, const char *b) {
  if (a == NULL) a = "";
  if (b == NULL) b = "";
  size_t len_a = strlen(a);
  size_t len_b = strlen(b);
  size_t total = len_a + len_b + 1;
  char *result = (char *)malloc(total);
  if (result == NULL) return NULL;
  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);
  result[len_a + len_b] = '\0';
  return result;
}

int vix_is_ptr_type(const char *type) {
  if (strcmp(type, "ptr") == 0) return 1;
  if (strncmp(type, "ptr:", 4) == 0) return 1;
  return 0;
}

const char *vix_ptr_pointee_type(const char *type) {
  if (strncmp(type, "ptr:", 4) == 0) return type + 4;
  return "unknown";
}

int vix_is_ptr_to_struct(const char *type) {
  if (strncmp(type, "ptr:", 4) != 0) return 0;
  const char *pointee = type + 4;
  return vix_find_struct_index(pointee) >= 0;
}

void *vix_array_push_ptr(void *arr, void *val) {
  int old_len = vix_array_len(arr);
  int new_len = old_len + 1;
  void *base = (arr == NULL) ? NULL : (void *)((char *)arr - 8);
  size_t data_bytes = new_len * sizeof(void *);
  size_t total_bytes = 8 + data_bytes;
  void *new_block = realloc(base, total_bytes);
  if (new_block == NULL) return NULL;
  *(int *)new_block = new_len;
  void **data = (void **)((char *)new_block + 8);
  data[old_len] = val;
  return (void *)((char *)new_block + 8);
}
