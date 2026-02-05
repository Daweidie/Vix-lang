#include "../../include/struct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static const char* ast_type_to_qbe(ASTNode* t) {
    if (!t) return "w";
    switch (t->type) {
        case AST_TYPE_INT32: return "w";
        case AST_TYPE_INT64: return "l";
        case AST_TYPE_FLOAT32: case AST_TYPE_FLOAT64: return "d";
        case AST_TYPE_STRING: return "l";
        case AST_TYPE_POINTER: return "l";
        default: return "w";
    }
}

void qbe_register_struct_def(QbeGenState* state, const char* name, char** field_names, char** field_types, int field_count) {
    if (state->struct_count >= state->struct_capacity) {
        state->struct_capacity *= 2;
        state->struct_names = realloc(state->struct_names, state->struct_capacity * sizeof(char*));
        state->struct_field_names = realloc(state->struct_field_names, state->struct_capacity * sizeof(char**));
        state->struct_field_types = realloc(state->struct_field_types, state->struct_capacity * sizeof(char**));
        state->struct_field_counts = realloc(state->struct_field_counts, state->struct_capacity * sizeof(int));
    }

    int idx = state->struct_count++;
    state->struct_names[idx] = strdup(name);
    state->struct_field_counts[idx] = field_count;
    state->struct_field_names[idx] = malloc(sizeof(char*) * field_count);
    state->struct_field_types[idx] = malloc(sizeof(char*) * field_count);
    for (int i = 0; i < field_count; i++) {
        state->struct_field_names[idx][i] = strdup(field_names[i]);
        state->struct_field_types[idx][i] = strdup(field_types[i]);
    }
}

void qbe_emit_struct_def(QbeGenState* state, struct ASTNode* node) {
    if (!node || node->type != AST_STRUCT_DEF) return;
    const char* name = node->data.struct_def.name;
    ASTNode* fields = node->data.struct_def.fields;
    if (!fields || fields->type != AST_EXPRESSION_LIST) return;

    int n = fields->data.expression_list.expression_count;
    char** names = malloc(sizeof(char*) * n);
    char** types = malloc(sizeof(char*) * n);
    for (int i = 0; i < n; i++) {
        ASTNode* a = fields->data.expression_list.expressions[i];
        if (a->type == AST_ASSIGN && a->data.assign.left->type == AST_IDENTIFIER) {
            names[i] = strdup(a->data.assign.left->data.identifier.name);
            types[i] = strdup(ast_type_to_qbe(a->data.assign.right));
        } else {
            names[i] = strdup("_anon");
            types[i] = strdup("w");
        }
    }
    qbe_register_struct_def(state, name, names, types, n);

    if (state->pending_struct_defs_count >= state->pending_struct_defs_capacity) {
        state->pending_struct_defs_capacity *= 2;
        state->pending_struct_defs = realloc(state->pending_struct_defs, state->pending_struct_defs_capacity * sizeof(char*));
    }
    int any_l = 0;
    for (int i = 0; i < n; i++) if (strcmp(types[i], "l") == 0 || strcmp(types[i], "d") == 0) any_l = 1;

    char* struct_def_str;
    int len = snprintf(NULL, 0, "type :%s = %s{ ", name, any_l ? "align 8 " : "");
    int all_same = 1;
    for (int i = 1; i < n; i++) if (strcmp(types[i], types[0]) != 0) { all_same = 0; break; }
    if (all_same) {
        len += snprintf(NULL, 0, "{ %s %d }", types[0], n);
    } else {
        int i = 0;
        while (i < n) {
            const char* t = types[i];
            int j = i+1;
            while (j < n && strcmp(types[j], t) == 0) j++;
            int run = j - i;
            len += snprintf(NULL, 0, "%s %d", t, run);
            if (j < n) len += snprintf(NULL, 0, ", ");
            i = j;
        }
        len += snprintf(NULL, 0, " }");
    }
    
    struct_def_str = malloc(len + 1);
    if (any_l) {
        snprintf(struct_def_str, len + 1, "type :%s = align 8 ", name);
    } else {
        snprintf(struct_def_str, len + 1, "type :%s = ", name);
    }

    if (all_same) {
        snprintf(struct_def_str + strlen(struct_def_str), len + 1 - strlen(struct_def_str), "{ %s %d }", types[0], n);
    } else {
        strcat(struct_def_str, "{ ");
        int i = 0;
        while (i < n) {
            const char* t = types[i];
            int j = i+1;
            while (j < n && strcmp(types[j], t) == 0) j++;
            int run = j - i;
            int remaining = len + 1 - strlen(struct_def_str);
            snprintf(struct_def_str + strlen(struct_def_str), remaining, "%s %d", t, run);
            if (j < n) strcat(struct_def_str, ", ");
            i = j;
        }
        strcat(struct_def_str, " }");
    }

    state->pending_struct_defs[state->pending_struct_defs_count] = struct_def_str;
    state->pending_struct_defs_count++;

    for (int i = 0; i < n; i++) { free(names[i]); free(types[i]); }
    free(names); free(types);
}

int qbe_get_struct_info(QbeGenState* state, const char* name, char*** out_field_names, char*** out_field_types, int* out_count) {
    for (int i = 0; i < state->struct_count; i++) {
        if (strcmp(state->struct_names[i], name) == 0) {
            *out_field_names = state->struct_field_names[i];
            *out_field_types = state->struct_field_types[i];
            *out_count = state->struct_field_counts[i];
            return 1;
        }
    }
    return 0;
}
