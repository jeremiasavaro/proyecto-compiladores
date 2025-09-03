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
    PROGRAM block END
    ;

block:
    /* empty */
    | block statement
    ;

statement:
      IF expr THEN block ELSE block END
    | WHILE expr block END
    | RETURN expr ';'
    | VOID ID '(' ')' block END
    | EXTERN ID ';'
    | BOOL ID ';'
    | INTEGER ID ';'
    | expr ';'
    | COMMENT
    | INLINECOMMENT
    ;

expr:
      expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr AND expr
    | expr OR expr
    | NEG expr
    | '(' expr ')'
    | ID
    | DIG
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