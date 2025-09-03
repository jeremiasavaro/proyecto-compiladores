%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();
void yyerror(const char *s);

%}

%token PROGRAM IF ELSE THEN WHILE END VOID RETURN EXTERN BOOL INTEGER FALSE TRUE
%token DIG ID INLINECOMMENT COMMENT
%token AND OR NEG

%left OR
%left AND
%left '+' '-'
%left '*' '/'

%%

program:
    PROGAM '{' var_decl method_decl '}'
    ;

var_decl:
      type ID '=' expr ';' var_decl
    | /* empty */
    ;

method_decl:
      method_type ID '(' method_args ')' block method_decl
    | method_type ID '(' method_args ')' extern ';' method_decl
    | /* empty */
    ;

method_type:
      type
    | VOID
    ;

method_args:
      type ID ',' method_args
    | type ID
    ;

type:
      INTEGER
    | BOOL
    ;

block:
      '{' var_decl statement '}'
      ;

statement:
    | ID '=' expr ';' statement
    | method_call ';' statement
    | IF '( expr ')' THEN block statement
    | IF '( expr ')' THEN block ELSE block statement
    | WHILE '(' expr ')' block statement
    | RETURN expr ';' statement
    | RETURN ';' statement
    | ';'
    | block
    |
    ;

method_call:
      ID '(' method_args ')'
      ;

expr:
      ID
    | method_call
    | literal
    | expr binary_op expr
    | '-' expr
    | NEG expr
    | '(' expr ')'
    ;

binary_op:
      arith_op
    | rel_op
    | cond_op
    ;

arith_op:
      '+'
    | '-'
    | '*'
    | '/'
    | '%'
    ;

rel_op:
      '<'
    | '>'
    | '=='
    ;

cond_op:
      AND
    | OR
    ;

literal:
      INTEGER_LITERAL
    | TRUE
    | FALSE
    ;



%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}

int main() {
    printf("=== SYNTAX ANALYSIS ===\n");
    yyparse();
    printf("Parsing finished.\n");
    return 0;
}