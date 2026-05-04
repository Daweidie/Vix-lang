#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
int yylex(void);
int yyparse(void);
void yyerror(const char *s);
extern ASTNode* root;
#ifdef __cplusplus
extern "C" {
#endif
int vix_is_adt_definition(const char* name);
int vix_adt_generic_arity(const char* name);
int vix_adt_ctor_payload_count(const char* ctor_name);
const char* vix_adt_ctor_base_name(const char* ctor_name);
#ifdef __cplusplus
}
#endif
#endif
