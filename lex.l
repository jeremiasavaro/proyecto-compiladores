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


{digit}+                 { yylval.ival = atoi(yytext); num_tokens++; return INTEGER_LITERAL; }

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
"false"                   { if (debug) printf("FALSE\n"); num_tokens++; return FALSE;}
"true"                    { if (debug) printf("TRUE\n");num_tokens++; return TRUE;}


{letter}({letter}|{digit}|_)*  { yylval.sval = my_strdup(yytext); if (debug) printf("ID: %s\n", yytext); num_tokens++; return ID; }

"//".*                 { /* ignore one line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* ignore multiline comments */ }

"&&"                   { if (debug) printf("AND\n"); return AND; }
"||"                   { if (debug) printf("OR\n");return OR; }
"!"                    { if (debug) printf("NEG\n"); return NEG; }
"=="                   { if (debug) printf("EQ\n"); return EQ; }
"!="                   { if (debug) printf("NEQ\n");return NEQ; }
"<="                   { if (debug) printf("LEQ\n"); return LEQ; }
">="                   { if (debug) printf("GEQ\n");return GEQ; }
"<"                    { if (debug) printf("LES\n"); return LES; }
">"                    { if (debug) printf("GRT\n"); return GRT; }
"+"                   { if (debug) printf("ADD\n"); return ADD; }
"-"                   { if (debug) printf("SUB\n"); return SUB; }
"*"                   { if (debug) printf("MUL\n"); return MUL; }
"/"                   { if (debug) printf("DIV\n"); return DIV; }
"%"                   { if (debug) printf("MOD\n"); return MOD; }
"="                   { if (debug) printf("ASSIGN\n"); return ASSIGN; }
[+\-*/%=]              { return *yytext; }


[,]                    { return *yytext; }
[;(){}]                { return *yytext; }
[ \t\r\n]+             { }
.                   { error_lexical(yylineno, yytext); }

%%
