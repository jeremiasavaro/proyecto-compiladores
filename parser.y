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
    PROGRAM '{' main_block '}'
    ;

main_block:
      var_decl main_block
    | /* empty */
    ;

var_decl:
      BOOL ID ';' var_decl
    | INTEGER ID ';' var_decl
    | statement
    ;

statement:
      IF expr THEN block ELSE block END statement
    | WHILE expr block END statement
    | RETURN expr ';' statement
    | EXTERN ID ';' statement
    | expr ';' statement
    | COMMENT statement
    | INLINECOMMENT statement
    | /* empty */
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
    | method_call
    | ID
    | INTEGER_LITERAL
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