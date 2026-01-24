#ifndef OPT_H
#define OPT_H
typedef struct {
    char* line;
    int def_reg;
    int *uses;
    int uses_count;
    int alive;
} Instr;
char* optimize_code(const char* input_code);

#endif//OPT.H