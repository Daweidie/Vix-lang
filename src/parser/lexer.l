%{
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "../include/ast.h"
#include "../include/compiler.h"
extern const char* current_input_filename;
extern YYSTYPE yylval;

int yycolumn = 1;

char* my_strndup(const char* str, size_t n) {
    size_t len = strlen(str);
    if (n < len) {
        len = n;
    }
    
    char* result = malloc(len + 1);
    if (result) {
        memcpy(result, str, len);
        result[len] = '\0';
    }
    return result;
}
char* process_escape_sequences(const char* input, size_t input_len) {
    char* result = malloc(input_len + 1);
    size_t result_idx = 0;
    for (size_t i = 0; i < input_len; i++) {//对于extern的printf
        if (input[i] == '\\' && i + 1 < input_len) {
            switch (input[i + 1]) {
                case 'n':
                    result[result_idx++] = '\n';
                    i++;
                    break;
                case 't':
                    result[result_idx++] = '\t';
                    i++;
                    break;
                case 'r':
                    result[result_idx++] = '\r';
                    i++;
                    break;
                case '\\':
                    result[result_idx++] = '\\';
                    i++;
                    break;
                case '"':
                    result[result_idx++] = '"';
                    i++;
                    break;
                case '\'':
                    result[result_idx++] = '\'';
                    i++;
                    break;
                case '0':
                    result[result_idx++] = '\0';
                    i++;
                    break;
                default:
                    result[result_idx++] = input[i];
                    break;
            }
        } else {
            result[result_idx++] = input[i];
        }
    }
    
    result[result_idx] = '\0';
    return result;
}

static int decode_char_literal(const char* text, size_t len, unsigned char* out) {
    if (!text || len < 3 || text[0] != '\'' || text[len - 1] != '\'') {
        return 0;
    }

    const char* body = text + 1;
    size_t body_len = len - 2;
    if (body_len == 0) return 0;

    if (body[0] != '\\') {
        if (body_len != 1) return 0;
        *out = (unsigned char)body[0];
        return 1;
    }

    if (body_len != 2) return 0;
    switch (body[1]) {
        case 'n': *out = (unsigned char)'\n'; return 1;
        case 't': *out = (unsigned char)'\t'; return 1;
        case 'r': *out = (unsigned char)'\r'; return 1;
        case '0': *out = (unsigned char)'\0'; return 1;
        case '\\': *out = (unsigned char)'\\'; return 1;
        case '\'': *out = (unsigned char)'\''; return 1;
        case '"': *out = (unsigned char)'"'; return 1;
        default: return 0;
    }
}
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput
%{
#define UPDATE_COLUMN() \
    do { \
        for (int i = 0; yytext[i] != '\0'; i++) { \
            if (yytext[i] == '\n') { \
                yycolumn = 1; \
            } else if (yytext[i] == '\t') { \
                yycolumn = ((yycolumn - 1) / 8 + 1) * 8 + 1; \
            } else { \
                yycolumn++; \
            } \
        } \
    } while(0)

#define GET_FIRST_COLUMN() (yycolumn - yyleng + 1)

#define SET_TOKEN_LOCATION() \
    do { \
        int col = GET_FIRST_COLUMN(); \
        yylloc.first_line = yylineno; \
        yylloc.first_column = col; \
        yylloc.last_line = yylineno; \
        yylloc.last_column = col + yyleng - 1; \
    } while(0)
%}

%%

[ \t\r\n]           { 
                      for (int i = 0; yytext[i] != '\0'; i++) {
                          if (yytext[i] == '\n') {
                              yycolumn = 1;
                          } else if (yytext[i] == '\t') {
                              yycolumn = ((yycolumn - 1) / 8 + 1) * 8 + 1;
                          } else {
                              yycolumn++;
                          }
                      }
                      } 

"#"[^\n]* { UPDATE_COLUMN(); }
"//"[^\n]*         { UPDATE_COLUMN(); }

"print"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return PRINT; }
"input"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return INPUT; }
"string"            { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_STR; }
"return"            { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return RETURN; }
"fn"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return FN; }
"u32"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I32; }
"i64"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I64; }
"u64"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I64; }
"i8"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I8; }
"u8"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I8; }
"f32"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_F32; }
"f64"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_F64; }
"usize"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I64; }
"extern"            { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return EXTERN; }
"const"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); /* const removed from user syntax */ }
"let"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return LET; }
"mut"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return MUT; }
"->"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return ARROW; }
"i32"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_I32; }
"void"              { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_VOID; }
"nil"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return NIL; }
"if"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return IF; }
"elif"              { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return ELIF; }
"else"              { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return ELSE; }
"while"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return WHILE; }
"break"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return BREAK; }
"continue"          { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return CONTINUE; }
"for"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return FOR; }
"in"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return IN; }
"global"           { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); /* global removed from user syntax */ }
"struct"           { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return STRUCT; }
"and"              { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return AND; }
"or"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return OR; }
"import"           { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return IMPORT; }
"pub"               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return PUB; }
"type"              { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return TYPE_KW; }
"match"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return MATCH; }
"true"              { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.num_int = 1; return NUMBER_INT; }
"false"             { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.num_int = 0; return NUMBER_INT; }

"="                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return ASSIGN; }
"+="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return PLUS_ASSIGN; }
"-="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); yylval.str = strdup(yytext); return MINUS_ASSIGN; }
"*="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return MULTIPLY_ASSIGN; }
"/="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return DIVIDE_ASSIGN; }
"%="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return MODULO_ASSIGN; }
"+"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return PLUS; }
"-"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return MINUS; }
"*"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return MULTIPLY; }
"/"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return DIVIDE; }
"%"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return MODULO; }
"**"                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return POWER; }
"=="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return EQ; }
"!="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return NE; }
"!"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return BANG; }
"<"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return LT; }
"<="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return LE; }
">"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return GT; }
">="                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return GE; }

"("                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return LPAREN; }
")"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return RPAREN; }
"{"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return LBRACE; }
"}"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return RBRACE; }
"["                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return LBRACKET; }
"]"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return RBRACKET; }
"@"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return AT; }
"&"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return AMPERSAND; }

";"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return SEMICOLON; }
","                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return COMMA; }
"..."               { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return DOTDOTDOT; }
".."                { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return DOTDOT; }
"."                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return DOT; }
":"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return COLON; }
"|"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return PIPE; }
"?"                 { UPDATE_COLUMN(); SET_TOKEN_LOCATION(); return QUESTION; }

[a-zA-Z_][a-zA-Z0-9_]*!?  {
                        int col = GET_FIRST_COLUMN();
                        yylval.str = strdup(yytext);
                        yylloc.first_line = yylineno;
                        yylloc.first_column = col;
                        yylloc.last_line = yylineno;
                        yylloc.last_column = col + yyleng - 1;
                        return IDENTIFIER;
                      }

0[xX][0-9a-fA-F]+   {
                                                int col = GET_FIRST_COLUMN();
                                                yylval.num_int = strtoll(yytext, NULL, 16);
                                                yylloc.first_line = yylineno;
                                                yylloc.first_column = col;
                                                yylloc.last_line = yylineno;
                                                yylloc.last_column = col + yyleng - 1;
                                                return NUMBER_INT;
                                            }

[0-9]+              {
                        int col = GET_FIRST_COLUMN();
                        yylval.num_int = atoll(yytext);
                        yylloc.first_line = yylineno;
                        yylloc.first_column = col;
                        yylloc.last_line = yylineno;
                        yylloc.last_column = col + yyleng - 1;
                        return NUMBER_INT;
                      }

[0-9]+"."[0-9]*      {
                        int col = GET_FIRST_COLUMN();
                        yylval.num_float = atof(yytext);
                        yylloc.first_line = yylineno;
                        yylloc.first_column = col;
                        yylloc.last_line = yylineno;
                        yylloc.last_column = col + yyleng - 1;
                        return NUMBER_FLOAT;
                      }

\"([^\"\\]|\\[\"\\nrt0\\'])*\"        {
                        int col = GET_FIRST_COLUMN();
                        char* raw_str = my_strndup(yytext + 1, yyleng - 2);
                        yylval.str = process_escape_sequences(raw_str, strlen(raw_str));
                        free(raw_str);
                        yylloc.first_line = yylineno;
                        yylloc.first_column = col;
                        yylloc.last_line = yylineno;
                        yylloc.last_column = col + yyleng - 1;
                        return STRING;
                      }

\'([^\\\'\n]|\\[nrt0\\\'\"])\'        {
                        int col = GET_FIRST_COLUMN();
                        unsigned char c = 0;
                        if (!decode_char_literal(yytext, (size_t)yyleng, &c)) {
                            report_lexical_error_with_location("Invalid char literal", current_input_filename ? current_input_filename : "unknown", yylineno);
                            return ERROR;
                        }
                        yylval.num_int = (long long)c;
                        yylloc.first_line = yylineno;
                        yylloc.first_column = col;
                        yylloc.last_line = yylineno;
                        yylloc.last_column = col + yyleng - 1;
                        return CHAR_LITERAL;
                      }

"/"[*]([^*]|[*]+[^*/])*[*]+"/"  { UPDATE_COLUMN(); } 

.                   {
                        char error_msg[256];
                        snprintf(error_msg, sizeof(error_msg), "Unknown: %c", yytext[0]);
                        report_lexical_error_with_location(error_msg, current_input_filename ? current_input_filename : "unknown", yylineno);
                        return ERROR;
                    }

%%

/*
 *wocao tmd 这个lexer也是真tm nb啊，我tm都不知道我是怎么写出来的
 */