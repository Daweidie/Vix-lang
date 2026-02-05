/* 
* 该实现解析由 vic_gen VIC IR(参见 src/vic-ir/ir.c)并生成llvm ir
* - data $sN = { ... "..." } 字符串常量
* - data $iN = { i64 <num> } 整数全局变量
* - data $vN = { i64 0 }` 变量槽
* - function name(r0, r1) { ... } 函数体，包含如下行
*   rN = consti intnum rN = constf flt rN = consts $sN
*   rD = add rA, rB rD = sub rA, rB call print(rN, ...) ret rN
*   rN = load_data $sN rN = load_data $iN rN = load_data $fN
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/llvm_emit.h"

typedef enum { RT_UNKNOWN=0, RT_I64, RT_F64, RT_STR } RegType;

static char *trim_inplace(char *s) {
    if (!s) return s;
    while (*s && isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
    return s;
}

static char *extract_quoted(const char *line) {
    const char *p = strchr(line, '"');
    if (!p) return NULL;
    const char *q = strrchr(line, '"');
    if (!q || q == p) return NULL;
    size_t n = q - p - 1;
    char *out = malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, p + 1, n);
    out[n] = '\0';
    return out;
}

static int parse_consti(const char *line, int *r, long long *v) {
    return sscanf(line, "r%d = consti %lld", r, v) == 2;
}

static int parse_constf(const char *line, int *r, double *v) {
    return sscanf(line, "r%d = constf %lf", r, v) == 2;
}

static int parse_consts_label(const char *line, int *r, int *sidx) {
    return sscanf(line, "r%d = consts $s%d", r, sidx) == 2;
}

static int parse_load_data(const char *line, int *r, char *label) {
    return sscanf(line, "r%d = load_data $%63s", r, label) == 2;
}

static int parse_binop(const char *line, const char *op, int *dst, int *a, int *b) {
    char fmt[64]; snprintf(fmt, sizeof(fmt), "r%%d = %s r%%d, r%%d", op);
    return sscanf(line, fmt, dst, a, b) == 3;
}

static int is_label(const char *s) { return s[0] == '@'; }

void llvm_emit_from_vic_fp(FILE *vic_fp, FILE *llvm_fp) {
    if (!vic_fp || !llvm_fp) return;
    fprintf(llvm_fp, ";vic->llvm emit\n\n");
    fprintf(llvm_fp, "target triple = \"x86_64-pc-linux-gnu\"\n\n");
    fprintf(llvm_fp, "declare i32 @printf(i8*, ...)\n");
    fprintf(llvm_fp, "declare i32 @puts(i8*)\n\n");
    char **defined_labels = NULL;
    size_t defined_labels_count = 0;
    size_t defined_labels_capacity = 0;
    char **var_names = NULL;
    size_t var_count = 0;
    size_t var_capacity = 0;
    char buf[4096];
    long pos = ftell(vic_fp);
    while (fgets(buf, sizeof(buf), vic_fp)) {
        char *line = trim_inplace(buf);
        if (!line || *line == '\0') continue;
        if (strncmp(line, "store_name ", 11) == 0) {
            char var_name[256];
            int src_reg;
            if (sscanf(line, "store_name %255[^,], r%d", var_name, &src_reg) == 2) {
                int var_found = 0;
                for (size_t i = 0; i < var_count; i++) {
                    if (strcmp(var_names[i], trim_inplace(var_name)) == 0) {
                        var_found = 1;
                        break;
                    }
                }
                if (!var_found) {
                    if (var_count >= var_capacity) {
                        var_capacity = var_capacity ? var_capacity * 2 : 16;
                        var_names = realloc(var_names, sizeof(char*) * var_capacity);
                    }
                    var_names[var_count] = strdup(trim_inplace(var_name));
                    var_count++;
                }
            }
        } else if (strncmp(line, "r", 1) == 0 && strstr(line, " = load_name ") != NULL) {
            int dst_reg;
            char var_name[256];
            if (sscanf(line, "r%d = load_name %255s", &dst_reg, var_name) == 2) {
                int var_found = 0;
                for (size_t i = 0; i < var_count; i++) {
                    if (strcmp(var_names[i], var_name) == 0) {
                        var_found = 1;
                        break;
                    }
                }
                if (!var_found) {
                    if (var_count >= var_capacity) {
                        var_capacity = var_capacity ? var_capacity * 2 : 16;
                        var_names = realloc(var_names, sizeof(char*) * var_capacity);
                    }
                    var_names[var_count] = strdup(var_name);
                    var_count++;
                }
            }
        }
    }
    fseek(vic_fp, pos, SEEK_SET);
    for (size_t i = 0; i < var_count; i++) {
        fprintf(llvm_fp, "@.%s = global i64 0\n", var_names[i]);
    }
    fprintf(llvm_fp, "\n");
    while (fgets(buf, sizeof(buf), vic_fp)) {
        char *line = trim_inplace(buf);
        if (!line || *line == '\0') continue;

        if (strncmp(line, "data ", 5) == 0) {
            if (strchr(line, '"')) {
                char lab[64]; if (sscanf(line, "data %63s =", lab) >= 1) {
                    int already_defined = 0;
                    for (size_t i = 0; i < defined_labels_count; i++) {
                        if (strcmp(defined_labels[i], lab) == 0) {
                            already_defined = 1;
                            break;
                        }
                    }
                    
                    if (!already_defined) {
                        if (defined_labels_count >= defined_labels_capacity) {
                            defined_labels_capacity = defined_labels_capacity ? 
                                defined_labels_capacity * 2 : 16;
                            defined_labels = realloc(defined_labels, 
                                sizeof(char*) * defined_labels_capacity);
                        }
                        defined_labels[defined_labels_count] = strdup(lab);
                        defined_labels_count++;
                        
                        char *s = extract_quoted(line);
                        if (!s) s = strdup("");
                        size_t len = strlen(s) + 1;
                        int idx = 0; if (lab[0] == '$' && lab[1] == 's') idx = atoi(lab + 2);
                        fprintf(llvm_fp, "@.str.%d = private constant [%zu x i8] c\"%s\\00\"\n", idx, len, s);
                        free(s);
                    }
                }
            } else if (strstr(line, "i64")) {
                char lab[64]; long long v; if (sscanf(line, "data %63s = { i64 %lld }", lab, &v) == 2) {
                    int already_defined = 0;
                    for (size_t i = 0; i < defined_labels_count; i++) {
                        if (strcmp(defined_labels[i], lab) == 0) {
                            already_defined = 1;
                            break;
                        }
                    }
                    
                    if (!already_defined) {
                        if (defined_labels_count >= defined_labels_capacity) {
                            defined_labels_capacity = defined_labels_capacity ? 
                                defined_labels_capacity * 2 : 16;
                            defined_labels = realloc(defined_labels, 
                                sizeof(char*) * defined_labels_capacity);
                        }
                        defined_labels[defined_labels_count] = strdup(lab);
                        defined_labels_count++;
                        
                        int idx = 0; 
                        if (lab[0] == '$') {
                            idx = atoi(lab + 2);
                            if (lab[1] == 'i') {
                                fprintf(llvm_fp, "@.i64.%d = global i64 %lld\n", idx, v);
                            } else if (lab[1] == 'v') {
                                fprintf(llvm_fp, "@.var.%d = global i64 %lld\n", idx, v);
                            } else {
                                fprintf(llvm_fp, "@.i64.%d = global i64 %lld\n", idx, v);
                            }
                        }
                    }
                }
            } else if (strstr(line, "f64")) {
                char lab[64]; double v; if (sscanf(line, "data %63s = { f64 %lf }", lab, &v) == 2) {
                    int already_defined = 0;
                    for (size_t i = 0; i < defined_labels_count; i++) {
                        if (strcmp(defined_labels[i], lab) == 0) {
                            already_defined = 1;
                            break;
                        }
                    }
                    
                    if (!already_defined) {
                        if (defined_labels_count >= defined_labels_capacity) {
                            defined_labels_capacity = defined_labels_capacity ? 
                                defined_labels_capacity * 2 : 16;
                            defined_labels = realloc(defined_labels, 
                                sizeof(char*) * defined_labels_capacity);
                        }
                        defined_labels[defined_labels_count] = strdup(lab);
                        defined_labels_count++;
                        
                        int idx = 0; 
                        if (lab[0] == '$') {
                            idx = atoi(lab + 2);
                            fprintf(llvm_fp, "@.f64.%d = global double %g\n", idx, v);
                        }
                    }
                }
            } else {
                char lab[64]; if (sscanf(line, "data %63s = { i64 %*d }", lab) == 1) {
                    int already_defined = 0;
                    for (size_t i = 0; i < defined_labels_count; i++) {
                        if (strcmp(defined_labels[i], lab) == 0) {
                            already_defined = 1;
                            break;
                        }
                    }
                    
                    if (!already_defined) {
                        if (defined_labels_count >= defined_labels_capacity) {
                            defined_labels_capacity = defined_labels_capacity ? 
                                defined_labels_capacity * 2 : 16;
                            defined_labels = realloc(defined_labels, 
                                sizeof(char*) * defined_labels_capacity);
                        }
                        defined_labels[defined_labels_count] = strdup(lab);
                        defined_labels_count++;
                        
                        int idx = 0; 
                        if (lab[0] == '$') {
                            idx = atoi(lab + 2);
                            fprintf(llvm_fp, "@.var.%d = global i64 0\n", idx);
                        }
                    }
                }
            }
            continue;
        }

        if (strncmp(line, "function ", 9) == 0) {
            char name[256]; char args[512]; args[0] = '\0';
            if (sscanf(line, "function %255[^(](%511[^)])", name, args) >= 1) {
                char *n = trim_inplace(name);
                fprintf(llvm_fp, "\ndefine i32 @%s(", n);
                trim_inplace(args);
                if (args[0]) {
                    char *a = args; char *tok = strtok(a, ","); int first = 1;
                    while (tok) {
                        char *t = trim_inplace(tok);
                        int r; if (sscanf(t, "r%d", &r) == 1) {
                            if (!first) fprintf(llvm_fp, ", ");
                            fprintf(llvm_fp, "i64 %%r%d", r);
                            first = 0;
                        }
                        tok = strtok(NULL, ",");
                    }
                }
                fprintf(llvm_fp, ") {\nentry:\n");

                /* reg typing tab */
                RegType *reg = NULL; int regcap = 0;

                while (fgets(buf, sizeof(buf), vic_fp)) {
                    char *bline = trim_inplace(buf);
                    if (!bline || *bline == '\0') continue;
                    if (*bline == '}') break;
                    if (is_label(bline)) { fprintf(llvm_fp, "; %s\n", bline); continue; }

                    int r; long long iv; double fv;
                    if (parse_consti(bline, &r, &iv)) {
                        if (r >= regcap) {
                            int newcap = regcap ? regcap*2 : 256; while (newcap <= r) newcap *= 2;
                            reg = realloc(reg, sizeof(RegType)*newcap); for (int i=regcap;i<newcap;i++) reg[i]=RT_UNKNOWN; regcap=newcap;
                        }
                        reg[r] = RT_I64;
                        fprintf(llvm_fp, "  %%r%d = add i64 0, %lld\n", r, iv);
                        continue;
                    }
                    if (parse_constf(bline, &r, &fv)) {
                        if (r >= regcap) {
                            int newcap = regcap ? regcap*2 : 256; while (newcap <= r) newcap *= 2;
                            reg = realloc(reg, sizeof(RegType)*newcap); for (int i=regcap;i<newcap;i++) reg[i]=RT_UNKNOWN; regcap=newcap;
                        }
                        reg[r] = RT_F64;
                        fprintf(llvm_fp, "  %%r%d = fadd double 0.0, %g\n", r, fv);
                        continue;
                    }
                    int sidx;
                    if (parse_consts_label(bline, &r, &sidx)) {
                        if (r >= regcap) {
                            int newcap = regcap ? regcap*2 : 256; while (newcap <= r) newcap *= 2;
                            reg = realloc(reg, sizeof(RegType)*newcap); for (int i=regcap;i<newcap;i++) reg[i]=RT_UNKNOWN; regcap=newcap;
                        }
                        reg[r] = RT_STR;
                        fprintf(llvm_fp, "  %%r%d = getelementptr inbounds [%d x i8], [%d x i8]* @.str.%d, i64 0, i64 0\n", r, sidx, sidx, sidx);
                        continue;
                    }
                    char label[64];
                    if (parse_load_data(bline, &r, label)) {
                        if (r >= regcap) {
                            int newcap = regcap ? regcap*2 : 256; while (newcap <= r) newcap *= 2;
                            reg = realloc(reg, sizeof(RegType)*newcap); for (int i=regcap;i<newcap;i++) reg[i]=RT_UNKNOWN; regcap=newcap;
                        }
                        if (label[0] == 's') {
                            int sidx = atoi(label + 1);
                            reg[r] = RT_STR;
                            fprintf(llvm_fp, "  %%r%d = getelementptr inbounds [%d x i8], [%d x i8]* @.str.%d, i64 0, i64 0\n", r, sidx, sidx, sidx);
                        } else if (label[0] == 'i') {
                            int iidx = atoi(label + 1);
                            reg[r] = RT_I64;
                            fprintf(llvm_fp, "  %%r%d = load i64, i64* @.i64.%d\n", r, iidx);
                        } else if (label[0] == 'f') {
                            int fidx = atoi(label + 1);
                            reg[r] = RT_F64;
                            fprintf(llvm_fp, "  %%r%d = load double, double* @.f64.%d\n", r, fidx);
                        } else {
                            fprintf(llvm_fp, "  ; unhandled load_data type for label: %s\n", label);
                        }
                        continue;
                    }

                    int d,a1,a2;
                    if (parse_binop(bline, "add", &d, &a1, &a2)) { if (d>=regcap){int nc=regcap?regcap*2:256;while(nc<=d)nc*=2;reg=realloc(reg,sizeof(RegType)*nc);for(int i=regcap;i<nc;i++)reg[i]=RT_UNKNOWN;regcap=nc;} reg[d]=RT_I64; fprintf(llvm_fp, "  %%r%d = add i64 %%r%d, %%r%d\n", d,a1,a2); continue; }
                    if (parse_binop(bline, "sub", &d, &a1, &a2)) { if (d>=regcap){int nc=regcap?regcap*2:256;while(nc<=d)nc*=2;reg=realloc(reg,sizeof(RegType)*nc);for(int i=regcap;i<nc;i++)reg[i]=RT_UNKNOWN;regcap=nc;} reg[d]=RT_I64; fprintf(llvm_fp, "  %%r%d = sub i64 %%r%d, %%r%d\n", d,a1,a2); continue; }
                    if (parse_binop(bline, "mul", &d, &a1, &a2)) { if (d>=regcap){int nc=regcap?regcap*2:256;while(nc<=d)nc*=2;reg=realloc(reg,sizeof(RegType)*nc);for(int i=regcap;i<nc;i++)reg[i]=RT_UNKNOWN;regcap=nc;} reg[d]=RT_I64; fprintf(llvm_fp, "  %%r%d = mul i64 %%r%d, %%r%d\n", d,a1,a2); continue; }
                    if (parse_binop(bline, "div", &d, &a1, &a2)) { if (d>=regcap){int nc=regcap?regcap*2:256;while(nc<=d)nc*=2;reg=realloc(reg,sizeof(RegType)*nc);for(int i=regcap;i<nc;i++)reg[i]=RT_UNKNOWN;regcap=nc;} reg[d]=RT_I64; fprintf(llvm_fp, "  %%r%d = sdiv i64 %%r%d, %%r%d\n", d,a1,a2); continue; }

                    if (strncmp(bline, "call print(", 11) == 0 || strstr(bline, "call print(") ) {
                        const char *p = strchr(bline, '(');
                        const char *q = strrchr(bline, ')');
                        if (p && q && q > p+1) {
                            char args[1024]; strncpy(args, p+1, (size_t)(q-p-1)); args[q-p-1] = '\0';
                            char *tok = strtok(args, ",");
                            while (tok) {
                                char *t = trim_inplace(tok);
                                int rid; if (sscanf(t, "r%d", &rid) == 1) {
                                    if (rid < regcap && reg && reg[rid] != RT_UNKNOWN) {
                                        if (reg[rid] == RT_STR) {
                                            fprintf(llvm_fp, "  call i32 @puts(i8* %%r%d)\n", rid);
                                        } else {
                                            fprintf(llvm_fp, "  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.fmt_i64, i64 0, i64 0), i64 %%r%d)\n", rid);
                                        }
                                    } else {
                                        fprintf(llvm_fp, "  ; Warning: register %%r%d is undefined, skipping print call\n", rid);
                                    }
                                }
                                tok = strtok(NULL, ",");
                            }
                        }
                        continue;
                    }

                    if (strncmp(bline, "store_name ", 11) == 0) {
                        char var_name[256];
                        int src_reg;
                        if (sscanf(bline, "store_name %255[^,], r%d", var_name, &src_reg) == 2) {
                            fprintf(llvm_fp, "  store i64 %%r%d, i64* @.%s\n", src_reg, trim_inplace(var_name));
                        } else {
                            fprintf(llvm_fp, "  ; unhandled store_name: %s\n", bline);
                        }
                        continue;
                    }
                    
                    if (strncmp(bline, "r", 1) == 0 && strstr(bline, " = load_name ") != NULL) {
                        int dst_reg;
                        char var_name[256];
                        if (sscanf(bline, "r%d = load_name %255s", &dst_reg, var_name) == 2) {
                            fprintf(llvm_fp, "  %%r%d = load i64, i64* @.%s\n", dst_reg, var_name);
                            if (dst_reg >= regcap) {
                                int newcap = regcap ? regcap*2 : 256; 
                                while (newcap <= dst_reg) newcap *= 2;
                                reg = realloc(reg, sizeof(RegType)*newcap); 
                                for (int i=regcap; i<newcap; i++) reg[i]=RT_UNKNOWN; 
                                regcap=newcap;
                            }
                            reg[dst_reg] = RT_I64;
                        } else {
                            fprintf(llvm_fp, "  ; unhandled load_name: %s\n", bline);
                        }
                        continue;
                    }

                    if (strncmp(bline, "r", 1) == 0 && strstr(bline, "= call ")) {
                        int dst; char fname[256]; if (sscanf(bline, "r%d = call %255[^\(]", &dst, fname) >= 1) {
                            fprintf(llvm_fp, "  %%r%d = call i64 @%s()\n", dst, trim_inplace(fname));
                        }
                        continue;
                    }

                    if (strncmp(bline, "ret", 3) == 0) {
                        fprintf(llvm_fp, "  ret i32 0\n");
                        continue;
                    }

                    fprintf(llvm_fp, "  ; unhandled: %s\n", bline);
                }
                fprintf(llvm_fp, "}\n");
                if (reg) {
                    free(reg);
                    reg = NULL;
                    regcap = 0;
                }
            }
            continue;
        }
    }
    fprintf(llvm_fp, "@.fmt_i64 = private constant [6 x i8] c\"%%lld\\0A\\00\"\n");
    if (defined_labels) {
        for (size_t i = 0; i < defined_labels_count; i++) {
            free(defined_labels[i]);
        }
        free(defined_labels);
    }
    if (var_names) {
        for (size_t i = 0; i < var_count; i++) {
            free(var_names[i]);
        }
        free(var_names);
    }
}