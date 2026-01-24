#ifndef VIC_IR_H
#define VIC_IR_H

#include <stdio.h>
#include "../../include/ast.h"

void vic_gen(ASTNode* ast, FILE* fp);

#endif // VIC_IR_H