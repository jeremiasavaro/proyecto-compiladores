%{
#include <stdio.h>
#include <stdlib.h>
#include "error_handling.h"

extern int yylex();
extern int yylineno;
void yyerror(const char *s);
extern FILE *yyin;

%}

%union {
    int ival;
    char *sval;
}

%token PROGRAM IF ELSE THEN WHILE VOID RETURN EXTERN BOOL INTEGER FALSE TRUE
%token <ival> INTEGER_LITERAL
%token <sval> ID
%token AND OR NEG EQ NEQ LEQ GEQ

%nonassoc ELSE

%left AND OR
%left EQ NEQ
%left '<' '>' LEQ GEQ
%left '+' '-'
%left '*' '/' '%'

%right UMINUS
%right NEG

%%

program:
    PROGRAM '{' decls '}' { printf("No syntactic errors.\n"); }
    ;

decls:
      decl decls
    | /* empty */
    ;

decl:
      var_decl
    | method_decl
    ;

var_decl:
      type ID '=' expr ';'
    | type ID ';' /* variable declaration without initialization */
    ;

method_decl:
      VOID ID '(' method_args ')' block
    | VOID ID '(' method_args ')' EXTERN ';'
    | type ID '(' method_args ')' block
    | type ID '(' method_args ')' EXTERN ';'
    ;

method_args:
      arg_list
    | /* empty */
    ;

arg_list:
      type ID
    | arg_list ',' type ID
    ;

type:
      INTEGER
    | BOOL
    ;

block:
      '{' var_decls statements '}'
    ;

var_decls:
      var_decl var_decls
    | /* empty */
    ;

statements:
      statement statements
    | /* empty */
    ;

statement:
      ID '=' expr ';'
    | method_call ';'
    | IF '(' expr ')' THEN block else
    | WHILE '(' expr ')' block
    | RETURN expr ';'
    | RETURN ';'
    | ';'
    | block
    ;

else:
      ELSE block
    ;

expr:
      ID
    | method_call
    | literal
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | expr '<' expr
    | expr '>' expr
    | expr EQ expr
    | expr NEQ expr
    | expr LEQ expr
    | expr GEQ expr
    | expr AND expr
    | expr OR expr
    | '-' expr %prec UMINUS
    | NEG expr
    | '(' expr ')'
    ;

method_call:
      ID '(' call_args ')'
      ;

call_args:
      expr_list
    | /* empty */
    ;

expr_list:
      expr
    | expr_list ',' expr
    ;

literal:
      INTEGER_LITERAL
    | TRUE
    | FALSE
    ;

%%

void yyerror(const char *s) {
    error_parse(yylineno, (char *)s);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            error_open_file(argv[1]);
        }
        yyin = file;
    }

    printf("=== SYNTAX ANALYSIS ===\n");
    yyparse();
    return 0;
}
