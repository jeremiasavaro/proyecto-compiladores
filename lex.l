%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "error_handling.h"

char *strdup(const char *s);

extern int yylineno;

/* Stats counter */
int num_tokens = 0;
int num_lines = 1;

%}

%option noyywrap
%option yylineno

digit     [0-9]
letter    [a-zA-Z]

%%


{digit}+                 { yylval.ival = atoi(yytext); num_tokens++; return INTEGER_LITERAL; }

"Program"                 { num_tokens++; return PROGRAM;}
"if"                      { num_tokens++; return IF; }
"else"                    { num_tokens++; return ELSE; }
"then"                    { num_tokens++; return THEN;}
"while"                   { num_tokens++; return WHILE; }
"void"                    { num_tokens++; return VOID;}
"return"                  { num_tokens++; return RETURN;}
"extern"                  { num_tokens++; return EXTERN;}
"bool"                    { num_tokens++; return BOOL;}
"integer"                 { num_tokens++; return INTEGER;}
"false"                   { num_tokens++; return FALSE;}
"true"                    { num_tokens++; return TRUE;}


{letter}({letter}|{digit}|_)*  { yylval.sval = strdup(yytext); num_tokens++; return ID; }


"//"({letter}|{digit}|_)*       { yylval.sval = strdup(yytext); num_tokens++; return INLINECOMMENT; }
"/*"({letter}|{digit}|_)*"*/"   { yylval.sval = strdup(yytext); num_tokens++; return COMMENT; }

"&&"                   { return AND; }
"||"                   { return OR; }
"!"                    { return NEG; }
"=="                   { return EQ; }
"!="                   { return NEQ; }
"<="                   { return LEQ; }
">="                   { return GEQ; }
[+\-*/%=]              { return *yytext; }
[;(){}]                { return *yytext; }
[ \t\r\n]+             { }
.                      { error_lexical(yylineno, yytext); }

%%
