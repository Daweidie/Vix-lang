#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../../include/opt.h"


static int parse_reg_at(const char *s, int *pos) {
    int i = *pos;
    if (s[i] != 'r') return -1;
    i++;
    if (!isdigit((unsigned char)s[i])) return -1;
    int val = 0;
    while (isdigit((unsigned char)s[i])) {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    *pos = i;
    return val;
}

static int find_def_reg(const char *line) {
    const char *p = line;
    while (*p) {
        if (*p == 'r' && isdigit((unsigned char)p[1])) {
            const char *q = p + 1;
            while (isdigit((unsigned char)*q)) q++;
            const char *r = q;
            while (*r && isspace((unsigned char)*r)) r++;
            if (*r == '=') {
                int num = 0;
                const char *d = p + 1;
                while (isdigit((unsigned char)*d)) { num = num*10 + (*d - '0'); d++; }
                return num;
            }
        }
        p++;
    }
    return -1;
}

static void collect_uses(Instr *ins) {
    const char *s = ins->line;
    int n = strlen(s);
    ins->uses = NULL;
    ins->uses_count = 0;
    for (int i = 0; i < n; i++) {
        if (s[i] == 'r' && isdigit((unsigned char)s[i+1])) {
            int j = i;
            int reg = parse_reg_at(s, &j);
            if (reg >= 0) {
                int def = ins->def_reg;
                const char *p = s + i;
                if (def == reg) {
                    const char *q = p + 1;
                    while (*q && isdigit((unsigned char)*q)) q++;
                    const char *r = q;
                    while (*r && isspace((unsigned char)*r)) r++;
                    if (*r == '=') {
                        i = j-1;
                        continue;
                    }
                }
                int found = 0;
                for (int k = 0; k < ins->uses_count; k++) if (ins->uses[k] == reg) { found = 1; break; }
                if (!found) {
                    ins->uses = realloc(ins->uses, sizeof(int) * (ins->uses_count + 1));
                    ins->uses[ins->uses_count++] = reg;
                }
                i = j-1;
            }
        }
    }
}

char* optimize_code(const char* input_code) {
    if (!input_code) return NULL;
    char* result = NULL;
    char* code_copy = strdup(input_code);
    if (!code_copy) return NULL;
    int line_count = 0;
    char* temp = code_copy;
    while ((temp = strchr(temp, '\n')) != NULL) {
        line_count++;
        temp++;
    }
    temp = code_copy;//设置tmp指向代码开头
    char** lines = malloc((line_count + 1) * sizeof(char*));// 创建一个行指针数组
    if (!lines) {
        free(code_copy);
        return NULL;
    }
    int idx = 0;
    char* saveptr;
    char* token = strtok_r(code_copy, "\n", &saveptr);
    while (token != NULL) {
        lines[idx++] = strdup(token);
        token = strtok_r(NULL, "\n", &saveptr);
    }
    line_count = idx;
    Instr *ins = NULL;
    int ins_count = 0;
    for (int i = 0; i < line_count; i++) {
        if (strlen(lines[i]) == 0) continue; // 跳过空行
        
        Instr it;
        it.line = strdup(lines[i]);
        it.def_reg = find_def_reg(it.line);
        it.uses = NULL;
        it.uses_count = 0;
        it.alive = 1;
        collect_uses(&it);
        ins = realloc(ins, sizeof(Instr) * (ins_count + 1));
        ins[ins_count++] = it;
    }
    for (int i = 0; i < idx; i++) {
        free(lines[i]);
    }
    free(lines);
    int max_reg = -1;
    for (int i = 0; i < ins_count; i++) {
        if (ins[i].def_reg > max_reg) max_reg = ins[i].def_reg;
        for (int j = 0; j < ins[i].uses_count; j++) if (ins[i].uses[j] > max_reg) max_reg = ins[i].uses[j];
    }
    if (max_reg < 0) max_reg = 0;
    int changed = 1;
    while (changed) {
        changed = 0;
        char *used = calloc(max_reg + 1, 1);
        for (int i = ins_count - 1; i >= 0; i--) {
            Instr *it = &ins[i];
            if (!it->line) continue;
            int has_def = it->def_reg >= 0;
            int side_effect = 0;
            if (strstr(it->line, "store_name") || strstr(it->line, "jmp") || strstr(it->line, "ret") ||
                (strstr(it->line, "call") && strchr(it->line, '=') == NULL) || strstr(it->line, "break") || strstr(it->line, "continue")) {
                side_effect = 1;
            }

            int keep = 1;
            if (has_def) {
                int d = it->def_reg;
                if (!used[d] && !side_effect) {
                    keep = 0;
                }
            }
            if (keep) {
                it->alive = 1;
            } else {
                if (it->alive) changed = 1;
                it->alive = 0;
            }
            for (int u = 0; u < it->uses_count; u++) {
                int r = it->uses[u];
                if (r >= 0 && r <= max_reg) used[r] = 1;
            }
            if (has_def) used[it->def_reg] = 0;
        }
        free(used);
    }
    int result_len = 0;
    for (int i = 0; i < ins_count; i++) {
        if (ins[i].alive) {
            result_len += strlen(ins[i].line) + 1; // +1 for newline
        }
    }
    
    if (result_len > 0) {
        result = malloc(result_len + 1);
        if (result) {
            result[0] = '\0';
            for (int i = 0; i < ins_count; i++) {
                if (ins[i].alive) {
                    strcat(result, ins[i].line);
                    strcat(result, "\n");
                }
            }
        }
    } else {
        result = strdup("");
    }
    for (int i = 0; i < ins_count; i++) {
        free(ins[i].line);
        if (ins[i].uses) free(ins[i].uses);
    }
    free(ins);
    
    free(code_copy);
    
    return result;
}

