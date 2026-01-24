#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "../include/qbe-ir/ir.h"
#include "../include/vic-ir/mir.h"
typedef enum {
    VIC_OP_LOAD_NAME,
    VIC_OP_LOAD_DATA,
    VIC_OP_STORE_NAME,
    VIC_OP_ADD,
    VIC_OP_SUB,
    VIC_OP_MUL,
    VIC_OP_DIV,
    VIC_OP_CALL,
    VIC_OP_RET,
    VIC_OP_JMP,
    VIC_OP_JMP_IF_FALSE,
    VIC_OP_COPY,
    VIC_OP_LABEL,
    VIC_OP_EQ,
    VIC_OP_NE,
    VIC_OP_LT,
    VIC_OP_LE,
    VIC_OP_GT,
    VIC_OP_GE,
    VIC_OP_NEG,
    VIC_OP_CONCAT,
    VIC_OP_REPEAT
} VicOp;
typedef struct {
    VicOp op;
    char dest[64];
    char src1[64];
    char src2[64];
    char type[8];
    char func_name[64];
} VicInstruction;
typedef struct {
    VicInstruction *instructions;
    int count;
    int capacity;
    char **data_labels;
    char **data_values;
    int data_count;
    int data_capacity;
    char **float_labels;
    char **float_values;
    int float_count;
    int float_capacity;
} VicProgram;
typedef struct {
    int reg_counter;
    int label_counter;
    VicProgram *program;
    FILE* output;
} QbeConverter;


int is_number(const char* str) {// 检查字符串是否为数字
    if (!str || !*str) return 0;
    
    char* endptr;
    strtod(str, &endptr);
    return *endptr == '\0';
}


int is_register(const char* str) {//检查字符串是否为寄存器
    return str && strlen(str) > 0 && str[0] == 'r';
}
int is_label(const char* str) {// 检查字符串是否为标签
    return str && strlen(str) > 0 && str[0] == '@';
}
void convert_reg_name(char* qbe_reg, const char* vic_reg) {
    if (vic_reg[0] == 'r') {
        snprintf(qbe_reg, 64, "%%r%s", vic_reg+1);
    } else if (vic_reg[0] != '%'){
        snprintf(qbe_reg, 64, "%%%s", vic_reg);
    } else {
        strncpy(qbe_reg, vic_reg, 63);
        qbe_reg[63] = '\0';
    }
}


char* find_float_value_by_label(VicProgram* program, const char* label) {//查找浮点数标签对应的值
    for (int i = 0; i < program->float_count; i++) {
        if (strcmp(program->float_labels[i], label) == 0) {
            return program->float_values[i];
        }
    }
    return NULL;
}

void convert_instruction_to_qbe(QbeConverter* converter, VicInstruction* inst) {//单个指令转换
    FILE* fp = converter->output;
    
    switch(inst->op) {
        case VIC_OP_LOAD_NAME:
            {
                char dest_reg[64];
                convert_reg_name(dest_reg, inst->dest);
                
                fprintf(fp, "\t%s =%s load %s\n", dest_reg, inst->type, inst->src1);
            }
            break;
            
        case VIC_OP_LOAD_DATA:
            {
                char dest_reg[64];
                convert_reg_name(dest_reg, inst->dest);
                if (inst->src1[0] == '$') {// 通过标签名称判断数据类型
                    if (inst->src1[1] == 's') {  // 字符串类型
                        fprintf(fp, "\t%s =l copy %s\n", dest_reg, inst->src1);
                    } else if (inst->src1[1] == 'i') {  // 整数类型
                        fprintf(fp, "\t%s =w loadw %s\n", dest_reg, inst->src1);
                    } else if (inst->src1[1] == 'f') {
                        char* float_value = find_float_value_by_label(converter->program, inst->src1);
                        if (float_value != NULL) {
                            char* value_start = float_value;
                            if (strncmp(float_value, "f64 ", 4) == 0) {
                                value_start += 4;
                            } else if (strncmp(float_value, "f32 ", 4) == 0) {
                                value_start += 4;
                            }
                            fprintf(fp, "\t%s =d copy d_%s\n", dest_reg, value_start);
                        } else {
                            fprintf(fp, "\t%s =d copy d_%s\n", dest_reg, inst->src1);
                        }
                    }
                    else{
                        fprintf(fp, "\t%s =l copy %s\n", dest_reg, inst->src1);
                    }
                } else {
                    if (isdigit(inst->src1[0]) || inst->src1[0] == '-' || strchr(inst->src1, '.') != NULL) {
                        if (strchr(inst->src1, '.') != NULL) {
                            fprintf(fp, "\t%s =d copy %s\n", dest_reg, inst->src1);
                        } else {
                            fprintf(fp, "\t%s =w copy %s\n", dest_reg, inst->src1);
                        }
                    } else {
                        fprintf(fp, "\t%s =%s copy %s\n", dest_reg, inst->type, inst->src1);
                    }
                }
            }
            break;
            
        case VIC_OP_STORE_NAME:
            {
                char src_reg[64];
                if (is_register(inst->src1)) {
                    convert_reg_name(src_reg, inst->src1);
                } else {
                    strcpy(src_reg, inst->src1);
                }
                
                fprintf(fp, "\tstore %s, %s\n", src_reg, inst->src2);
            }
            break;
            
        case VIC_OP_ADD:
            {
                char dest_reg[64], src1_reg[64], src2_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src1_reg, inst->src1);
                } else {
                    strcpy(src1_reg, inst->src1);
                }
                
                if (is_register(inst->src2)) {
                    convert_reg_name(src2_reg, inst->src2);
                } else {
                    strcpy(src2_reg, inst->src2);
                }
                
                fprintf(fp, "\t%s =%s add %s, %s\n", dest_reg, inst->type, src1_reg, src2_reg);
            }
            break;
            
        case VIC_OP_SUB:
            {
                char dest_reg[64], src1_reg[64], src2_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src1_reg, inst->src1);
                } else {
                    strcpy(src1_reg, inst->src1);
                }
                
                if (is_register(inst->src2)) {
                    convert_reg_name(src2_reg, inst->src2);
                } else {
                    strcpy(src2_reg, inst->src2);
                }
                
                fprintf(fp, "\t%s =%s sub %s, %s\n", dest_reg, inst->type, src1_reg, src2_reg);
            }
            break;
            
        case VIC_OP_MUL:
            {
                char dest_reg[64], src1_reg[64], src2_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src1_reg, inst->src1);
                } else {
                    strcpy(src1_reg, inst->src1);
                }
                
                if (is_register(inst->src2)) {
                    convert_reg_name(src2_reg, inst->src2);
                } else {
                    strcpy(src2_reg, inst->src2);
                }
                
                fprintf(fp, "\t%s =%s mul %s, %s\n", dest_reg, inst->type, src1_reg, src2_reg);
            }
            break;
            
        case VIC_OP_DIV:
            {
                char dest_reg[64], src1_reg[64], src2_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src1_reg, inst->src1);
                } else {
                    strcpy(src1_reg, inst->src1);
                }
                
                if (is_register(inst->src2)) {
                    convert_reg_name(src2_reg, inst->src2);
                } else {
                    strcpy(src2_reg, inst->src2);
                }
                
                fprintf(fp, "\t%s =%s div %s, %s\n", dest_reg, inst->type, src1_reg, src2_reg);
            }
            break;
            
        case VIC_OP_NEG:
            {
                char dest_reg[64], src_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src_reg, inst->src1);
                } else {
                    strcpy(src_reg, inst->src1);
                }
                
                fprintf(fp, "\t%s =%s sub 0, %s\n", dest_reg, inst->type, src_reg);
            }
            break;
            
        case VIC_OP_EQ:
        case VIC_OP_NE:
        case VIC_OP_LT:
        case VIC_OP_LE:
        case VIC_OP_GT:
        case VIC_OP_GE:
            {
                char dest_reg[64], src1_reg[64], src2_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src1_reg, inst->src1);
                } else {
                    strcpy(src1_reg, inst->src1);
                }
                
                if (is_register(inst->src2)) {
                    convert_reg_name(src2_reg, inst->src2);
                } else {
                    strcpy(src2_reg, inst->src2);
                }
                char cmp_op[16];
                switch(inst->op) {
                    case VIC_OP_EQ: strcpy(cmp_op, "ceq"); break;
                    case VIC_OP_NE: strcpy(cmp_op, "cne"); break;
                    case VIC_OP_LT: strcpy(cmp_op, "clt"); break;
                    case VIC_OP_LE: strcpy(cmp_op, "cle"); break;
                    case VIC_OP_GT: strcpy(cmp_op, "cgt"); break;
                    case VIC_OP_GE: strcpy(cmp_op, "cge"); break;
                    default: strcpy(cmp_op, "ceq"); break;
                }
                char type_suffix = inst->type[0];
                if (type_suffix == 'w' || type_suffix == 'l') {
                    fprintf(fp, "\t%s =%s c%s%c %s, %s\n", dest_reg, inst->type, cmp_op, type_suffix, src1_reg, src2_reg);
                } else {
                    fprintf(fp, "\t%s =%s c%s%c %s, %s\n", dest_reg, inst->type, cmp_op, type_suffix, src1_reg, src2_reg);
                }
            }
            break;
            
        case VIC_OP_COPY:
            {
                char dest_reg[64], src_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                } else {
                    snprintf(dest_reg, 64, "%%%s", inst->dest);
                }
                
                if (is_register(inst->src1)) {
                    convert_reg_name(src_reg, inst->src1);
                } else {
                    strcpy(src_reg, inst->src1);
                }
                
                fprintf(fp, "\t%s =%s copy %s\n", dest_reg, inst->type, src_reg);
            }
            break;
            
        case VIC_OP_CALL: {
            char actual_func_name[64];
            if (strcmp(inst->src1, "print") == 0) {
                strcpy(actual_func_name, "vix_print");
            } else {
                strcpy(actual_func_name, inst->src1);
            }
            
            if (strlen(inst->dest) > 0) {
                char dest_reg[64];
                if (is_register(inst->dest)) {
                    convert_reg_name(dest_reg, inst->dest);
                    fprintf(fp, "\t%s = call $%s(", dest_reg, actual_func_name);
                } else {
                    fprintf(fp, "\tcall $%s(", actual_func_name);
                }
            } else {
                fprintf(fp, "\tcall $%s(", actual_func_name);
            }
            
            if (strlen(inst->src2) > 0) {
                char arg_reg[64];
                if (is_register(inst->src2)) {
                    convert_reg_name(arg_reg, inst->src2);
                } else {
                    strcpy(arg_reg, inst->src2);
                }
                fprintf(fp, "l %s", arg_reg);
            }
            fprintf(fp, ")\n");
            break;
        }
        
        case VIC_OP_RET:
            break;//默认不处理
            
        case VIC_OP_JMP:
            fprintf(fp, "\tjmp %s\n", inst->src1);
            break;
            
        case VIC_OP_JMP_IF_FALSE:
            {
                char src_reg[64];
                if (is_register(inst->src1)) {
                    convert_reg_name(src_reg, inst->src1);
                } else {
                    strcpy(src_reg, inst->src1);
                }
                
                fprintf(fp, "\tjnz %s, %s, %s\n", src_reg, inst->src2, inst->dest);
            }
            break;
            
        case VIC_OP_LABEL:
            fprintf(fp, "%s:\n", inst->dest);
            break;
            
        default:
            break;
    }
}
void convert_vic_to_qbe(VicProgram* vic_program, FILE* output) {
    if (!vic_program) return;
    
    QbeConverter converter = {0};
    converter.reg_counter = 0;
    converter.label_counter = 0;
    converter.program = vic_program;
    converter.output = output;
    
    for (int i = 0; i < vic_program->data_count; i++) {
        char* original_value = vic_program->data_values[i];
        char* label = vic_program->data_labels[i];
        if (label[1] == 'f') {
            continue;
        }
        
        if (strstr(original_value, "\"") != NULL) {
            char* start_quote = strchr(original_value, '"');
            char* end_quote = strrchr(original_value, '"');
            if (start_quote && end_quote && start_quote != end_quote) {
                size_t start_pos = start_quote - original_value;
                size_t end_pos = end_quote - original_value;
                char extracted_str[256];
                strncpy(extracted_str, original_value + start_pos, end_pos - start_pos + 1);
                extracted_str[end_pos - start_pos + 1] = '\0';
                fprintf(output, "data $s%d = { b %s, b 0 }\n", i, extracted_str);
            } else {
                fprintf(output, "data %s = { %s }\n", vic_program->data_labels[i], original_value);
            }
        } else {
            if (strstr(label, "i") != NULL) {//整数标签
                char type_part[64], value_part[64];
                if (sscanf(original_value, "%s %s", type_part, value_part) == 2) {
                    if (strcmp(type_part, "i64") == 0) {
                        fprintf(output, "data $i%d = { w %s }\n", i, value_part);
                    } else {
                        fprintf(output, "data %s = { %s }\n", label, original_value);
                    }
                } else {
                    fprintf(output, "data $i%d = { w %s }\n", i, original_value);
                }
            } else {
                fprintf(output, "data %s = { %s }\n", vic_program->data_labels[i], original_value);
            }
        }
    }
    
    if (vic_program->data_count > 0) fprintf(output, "\n");
    fprintf(output, "export function w $main() {\n");
    fprintf(output, "@start\n");
    for (int i = 0; i < vic_program->count; i++) { // 转换每个指令
        if (vic_program->instructions[i].op != VIC_OP_RET) {
            convert_instruction_to_qbe(&converter, &vic_program->instructions[i]);
        }
    }
    
    fprintf(output, "\tret 0\n");
    fprintf(output, "}\n");
    for (int i = 0; i < vic_program->data_count; i++) {
        free(vic_program->data_labels[i]);
        free(vic_program->data_values[i]);
    }
    for (int i = 0; i < vic_program->float_count; i++) {
        free(vic_program->float_labels[i]);
        free(vic_program->float_values[i]);
    }
    free(vic_program->data_labels);
    free(vic_program->data_values);
    free(vic_program->float_labels);
    free(vic_program->float_values);
    free(vic_program->instructions);
    free(vic_program);
}
int parse_call_params(char* line, char* func_name, char* params) {
    char* call_start = strstr(line, "call ");
    if (call_start) {
        call_start += 5;// 跳过call
        char* paren_open = strchr(call_start, '(');
        char* paren_close = strrchr(line, ')');
        if (paren_open && paren_close && paren_open < paren_close) {
            int name_len = paren_open - call_start;
            strncpy(func_name, call_start, name_len);
            func_name[name_len] = '\0';
            int param_len = paren_close - paren_open - 1;
            strncpy(params, paren_open + 1, param_len);
            params[param_len] = '\0';
            int start = 0, end = strlen(params) - 1;
            while(start <= end && isspace(params[start])) start++;
            while(end >= start && isspace(params[end])) end--;
            
            if(start <= end) {
                memmove(params, params+start, end-start+1);
                params[end-start+1] = '\0';
            } else {
                params[0] = '\0';
            }
            
            return 1;
        }
    }
    return 0;
}


int parse_assignment(char* line, char* dest, char* op, char* src1, char* src2) {
    // 解析赋值语句: dest = op src1 或 dest = op src1, src2
    char* eq_pos = strchr(line, '='); // 查找等号
    if (!eq_pos) return 0;
    *eq_pos = '\0';
    if (sscanf(line, "%63s", dest) != 1) {
        *eq_pos = '=';//恢复
        return 0;
    }
    *eq_pos = '=';//恢复
    char* after_eq = eq_pos + 1;
    while (*after_eq == ' ' || *after_eq == '\t') after_eq++;
    char* comma = strchr(after_eq, ',');
    if (comma) {
        char temp[256];
        strcpy(temp, after_eq);
        char* comma_in_temp = strchr(temp, ',');
        if (comma_in_temp) {
            *comma_in_temp = '\0';
            comma_in_temp++;
            while (*comma_in_temp == ' ' || *comma_in_temp == '\t') comma_in_temp++;
            
            if (sscanf(temp, "%63s %63s", op, src1) == 2) {
                strcpy(src2, comma_in_temp);
                int len = strlen(src2);
                while (len > 0 && (src2[len-1] == ' ' || src2[len-1] == '\t' || src2[len-1] == '\n' || src2[len-1] == '\r')) {
                    src2[--len] = '\0';
                }
                return 1;
            }
        }
    } else {
        if (sscanf(after_eq, "%63s %63s", op, src1) == 2) {
            src2[0] = '\0';
            return 1;
        }
    }
    
    return 0;
}
VicProgram* parse_vic_ir_from_text(const char* text) {
    VicProgram* prog = malloc(sizeof(VicProgram));// 创建VicProgram结构
    prog->capacity = 64;
    prog->count = 0;
    prog->instructions = malloc(prog->capacity * sizeof(VicInstruction));
    prog->data_labels = malloc(16 * sizeof(char*));
    prog->data_values = malloc(16 * sizeof(char*));
    prog->data_count = 0;
    prog->data_capacity = 16;
    prog->float_labels = malloc(16 * sizeof(char*));
    prog->float_values = malloc(16 * sizeof(char*));
    prog->float_count = 0;
    prog->float_capacity = 16;
    
    if (text) {
        char* text_copy = strdup(text);
        char* line = strtok(text_copy, "\n\r");
        
        while (line != NULL) {
           
            while (*line == ' ' || *line == '\t') {
                line++;
            } // 跳过空行和注释
            if (strlen(line) == 0 || line[0] == ';') {
                line = strtok(NULL, "\n\r");
                continue;
            }
            
            
            if (strncmp(line, "data ", 5) == 0) {// 解析数据段
                char label[64], value[256];
                if (sscanf(line, "data %63s = { %[^\n\r}]}", label, value) == 2) {
                    if (prog->data_count >= prog->data_capacity) {
                        prog->data_capacity *= 2;
                        prog->data_labels = realloc(prog->data_labels, prog->data_capacity * sizeof(char*));
                        prog->data_values = realloc(prog->data_values, prog->data_capacity * sizeof(char*));
                    }
                    if (label[1] == 'f') {  // 浮点数标签以f开头
                        if (prog->float_count >= prog->float_capacity) {
                            prog->float_capacity *= 2;
                            prog->float_labels = realloc(prog->float_labels, prog->float_capacity * sizeof(char*));
                            prog->float_values = realloc(prog->float_values, prog->float_capacity * sizeof(char*));
                        }
                        
                        prog->float_labels[prog->float_count] = strdup(label);
                        prog->float_values[prog->float_count] = strdup(value);
                        prog->float_count++;
                    } else {
                        prog->data_labels[prog->data_count] = strdup(label);
                        prog->data_values[prog->data_count] = strdup(value);
                        prog->data_count++;
                    }
                }
            }
            else if (strstr(line, "function main() {")) {//解析函数定义
            }
            else if (line[0] == '}') {
                // 函数结束
            }
            else {
                char dest[64], op[64], src1[64], src2[64];
                
                if (parse_assignment((char*)line, dest, op, src1, src2)) {
                    if (prog->count >= prog->capacity) {
                        prog->capacity *= 2;
                        prog->instructions = realloc(prog->instructions, prog->capacity * sizeof(VicInstruction));
                    }
                    
                    VicInstruction* inst = &prog->instructions[prog->count++];
                    if (strcmp(op, "load_data") == 0) {
                        inst->op = VIC_OP_LOAD_DATA;
                        strcpy(inst->dest, dest);
                        strcpy(inst->src1, src1);
                        strcpy(inst->type, "l");
                    } else if (strcmp(op, "add") == 0) {
                        inst->op = VIC_OP_ADD;
                        strcpy(inst->dest, dest);
                        strcpy(inst->src1, src1);
                        strcpy(inst->src2, src2);
                        strcpy(inst->type, "l");
                    } else if (strcmp(op, "sub") == 0) {
                        inst->op = VIC_OP_SUB;
                        strcpy(inst->dest, dest);
                        strcpy(inst->src1, src1);
                        strcpy(inst->src2, src2);
                        strcpy(inst->type, "l");
                    } else {
                        prog->count--;//回退技术
                    }
                }
                else if (strncmp(line, "call ", 5) == 0) {
                    if (prog->count >= prog->capacity) {
                        prog->capacity *= 2;
                        prog->instructions = realloc(prog->instructions, prog->capacity * sizeof(VicInstruction));
                    }
                    
                    VicInstruction* inst = &prog->instructions[prog->count++];
                    inst->op = VIC_OP_CALL;
                    inst->dest[0] = '\0';
                    char func_name[64], params[64];
                    if (parse_call_params((char*)line, func_name, params)) {
                        strcpy(inst->src1, func_name);
                        strcpy(inst->src2, params);
                    } else {
                        if (sscanf(line, "call %63s(%63[^)])", inst->src1, inst->src2) != 2) {//对解析失败的情况进行处理
                            sscanf(line, "call %63s", inst->src1);// 如果还是失败，只提取函数名
                            inst->src2[0] = '\0';
                        }
                    }
                }
                else if (strncmp(line, "ret", 3) == 0) {
                    if (prog->count >= prog->capacity) {
                        prog->capacity *= 2;
                        prog->instructions = realloc(prog->instructions, prog->capacity * sizeof(VicInstruction));
                    }
                    
                    VicInstruction* inst = &prog->instructions[prog->count++];
                    inst->op = VIC_OP_RET;
                    char temp[64];
                    if (sscanf(line, "ret %63s", temp) == 1) {
                        strcpy(inst->src1, temp);
                    } else {
                        inst->src1[0] = '\0';
                    }
                }
            }
            
            line = strtok(NULL, "\n\r");
        }
        
        free(text_copy);
    }
    
    return prog;
}


void ir_gen(FILE* vic_ir_input, FILE* output) {// 主要的转换函数 从vic ir文件转换为qbe ir
    if (!vic_ir_input || !output) {
        fprintf(stderr, "Error: Invalid input/output file pointers\n");
        return;
    }
    fseek(vic_ir_input, 0, SEEK_END);
    long size = ftell(vic_ir_input);
    fseek(vic_ir_input, 0, SEEK_SET);
    
    if (size <= 0) {
        fprintf(output, "export function w $main() {\n");
        fprintf(output, "@start\n");
        fprintf(output, "\tret 0\n");
        fprintf(output, "}\n");
        return;
    }
    
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }
    fread(buffer, 1, size, vic_ir_input);
    buffer[size] = '\0';
    VicProgram* program = parse_vic_ir_from_text(buffer);
    if (!program) {
        fprintf(stderr, "Error: Could not parse VIC IR\n");
        free(buffer);
        return;
    }
    convert_vic_to_qbe(program, output);
    
    free(buffer);
}
void qbe_gen_from_ast(ASTNode* ast, FILE* fp) {
    if (!fp || !ast) return;
    fprintf(fp, "export function w $main() {\n");
    fprintf(fp, "@start\n");
    fprintf(fp, "\tret 0\n");
    fprintf(fp, "}\n");
}