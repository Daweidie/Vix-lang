#ifndef QBE_STRUCT_H
#define QBE_STRUCT_H

#include "qbe-ir/ir.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Register a struct definition: names and types arrays are copied. */
void qbe_register_struct_def(QbeGenState* state, const char* name, char** field_names, char** field_types, int field_count);

/* Emit a struct type definition from an AST_STRUCT_DEF node. */
void qbe_emit_struct_def(QbeGenState* state, struct ASTNode* node);

/* Lookup struct definition: returns field names and types (arrays owned by state). */
int qbe_get_struct_info(QbeGenState* state, const char* name, char*** out_field_names, char*** out_field_types, int* out_count);

#ifdef __cplusplus
}
#endif

#endif // QBE_STRUCT_H
