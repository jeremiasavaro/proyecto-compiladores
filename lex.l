%{
    #define _GNU_SOURCE
    #include "ast.h"
    #include "parser.tab.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "error_handling.h"
    #include "utils.h"

extern int yylineno;
extern int debug;

/* Stats counter */
int num_tokens = 0;

%}

%option noyywrap
%option yylineno

digit     [0-9]
letter    [a-zA-Z]

%%


{digit}+                 { if (debug) printf("VALUE: %s\n", yytext); yylval.ival = atoi(yytext); num_tokens++; return INTEGER_LITERAL; }

"Program"                 { if (debug) printf("PROGRAM\n"); num_tokens++; return PROGRAM;}
"if"                      { if (debug) printf("IF\n"); num_tokens++; return IF; }
"else"                    { if (debug) printf("ELSE\n"); num_tokens++; return ELSE; }
"then"                    { if (debug) printf("THEN\n"); num_tokens++; return THEN;}
"while"                   { if (debug) printf("WHILE\n"); num_tokens++; return WHILE; }
"void"                    { if (debug) printf("VOID\n"); num_tokens++; return VOID;}
"return"                  { if (debug) printf("RETURN\n");num_tokens++; return RETURN;}
"extern"                  { if (debug) printf("EXTERN\n"); num_tokens++; return EXTERN;}
"bool"                    { if (debug) printf("BOOL\n"); num_tokens++; return BOOL;}
"integer"                 { if (debug) printf("INTEGER\n"); num_tokens++; return INTEGER;}
"false"                   { if (debug) printf("VALUE: FALSE\n"); num_tokens++; return FALSE;}
"true"                    { if (debug) printf("VALUE: TRUE\n");num_tokens++; return TRUE;}


{letter}({letter}|{digit}|_)*  { yylval.sval = my_strdup(yytext); if (debug) printf("ID: %s\n", yytext); num_tokens++; return ID; }

"//".*                 { /* ignore one line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* ignore multiline comments */ }

"&&"                   { if (debug) printf("AND\n"); num_tokens++; return AND; }
"||"                   { if (debug) printf("OR\n"); num_tokens++; return OR; }
"!"                    { if (debug) printf("NEG\n"); num_tokens++; return NEG; }
"=="                   { if (debug) printf("EQ\n"); num_tokens++; return EQ; }
"!="                   { if (debug) printf("NEQ\n"); num_tokens++; return NEQ; }
"<="                   { if (debug) printf("LEQ\n"); num_tokens++; return LEQ; }
">="                   { if (debug) printf("GEQ\n"); num_tokens++; return GEQ; }
"<"                    { if (debug) printf("LES\n"); num_tokens++; return LES; }
">"                    { if (debug) printf("GRT\n"); num_tokens++; return GRT; }
"+"                    { if (debug) printf("ADD\n"); num_tokens++; return '+'; }
"-"                    { if (debug) printf("SUB\n"); num_tokens++; return '-'; }
"*"                    { if (debug) printf("MUL\n"); num_tokens++; return '*'; }
"/"                    { if (debug) printf("DIV\n"); num_tokens++; return '/'; }
"%"                    { if (debug) printf("MOD\n"); num_tokens++; return '%'; }
"="                    { if (debug) printf("ASSIGN\n"); num_tokens++; return '='; }

[,]                    { return *yytext; }
[;(){}]                { return *yytext; }
[ \t\r\n]+             { }
.                   { error_lexical(yylineno, yytext); }

%%
