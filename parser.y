%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handling.h"
#include "ast.h"
#include "print_funcs.h"
#include "symbol_table.h"

extern int yylex();
extern int yylineno;
void yyerror(const char *s);
extern FILE *yyin;
extern TABLE_STACK* global_level;
%}

%union {
    int ival;
    char *sval;
    AST_NODE* node;
    AST_NODE_LIST* nodelist;
}

%token PROGRAM IF ELSE THEN WHILE VOID RETURN EXTERN BOOL INTEGER FALSE TRUE
%token <ival> INTEGER_LITERAL
%token <sval> ID
%token AND OR NEG EQ NEQ LEQ GEQ

%left AND OR
%left EQ NEQ
%left LES GRT LEQ GEQ
%left '+' '-'
%left '*' '/' '%'

%right UMINUS
%right NEG

%type <node> program decls decl var_decl method_decl block statement expr literal method_call
%type <nodelist> method_args arg_list expr_list call_args statements var_decls
%type <ival> type

%%

program:
    PROGRAM '{' decls '}' { printf("No syntactic errors.\n"); }
    ;

decls:
      decls decl { if ($2) add_sentence($2); } /* garantize decl != NULL */
    | /* empty */ { $$ = NULL; }
    ;

decl:
      var_decl    { $$ = $1; }
    | method_decl { $$ = $1; }
    ;

var_decl:
      type ID '=' expr ';'
        {
          AST_NODE* id = new_leaf_node(TYPE_ID, $2);
          if ($1 == INTEGER) { $$ = new_binary_node(OP_DECL_INT, id, $4); add_id($2, CONST_INT); }
          else { $$ = new_binary_node(OP_DECL_BOOL, id, $4); add_id($2, CONST_BOOL); }
        }
    | type ID ';'
        {
          AST_NODE* id = new_leaf_node(TYPE_ID, $2);
          if ($1 == INTEGER) { $$ = new_unary_node(OP_DECL_INT, id); add_id($2, CONST_INT); }
          else { $$ = new_unary_node(OP_DECL_BOOL, id); add_id($2, CONST_BOOL); }
        }
    ;

var_decls:
            var_decls var_decl { $$ = append_expr($1, $2); }
        | /* empty */ { $$ = NULL; }     
        ;

method_decl:
      VOID ID '(' method_args ')' block {
        $$ = new_method_node($2, $4, $6, 0);
        add_method($2, RETURN_VOID);
      }
    | VOID ID '(' method_args ')' EXTERN ';'
        { $$ = new_method_node($2, $4, NULL, 1); }
    | type ID '(' method_args ')' block {
        $$ = new_method_node($2, $4, $6, 0);
        if ($1 == INTEGER) {add_method($2, RETURN_INT);}
        else if ($1 == BOOL) {add_method($2, RETURN_BOOL);}
    }
    | type ID '(' method_args ')' EXTERN ';'
        { $$ = new_method_node($2, $4, NULL, 1); }
    ;

method_args
    : arg_list         { $$ = $1; }
    | /* empty */      { $$ = NULL; }
    ;

arg_list
    : type ID  { $$ = append_expr(NULL, new_leaf_node(TYPE_ID, $2)); }
    | arg_list ',' type ID { $$ = append_expr($1, new_leaf_node(TYPE_ID, $4)); }
    ;

type:
      INTEGER { $$ = INTEGER; }
    | BOOL { $$ = BOOL; }
    ;

block:
    '{' var_decls statements '}' { /* to maintain the previous decls */
        AST_NODE_LIST *merged = $2;
        if (merged) {
            AST_NODE_LIST *last = merged;
            while (last->next) last = last->next;
            last->next = $3;
            $$ = new_block_node(merged);
        } else {
            $$ = new_block_node($3);
        }
    }
    ;

statements:
        statements statement { $$ = append_expr($1, $2); }
    | /* empty */ { $$ = NULL; }
    ;

statement:
      ID '=' expr ';' 
        {
          AST_NODE* id = new_leaf_node(TYPE_ID, $1);
          $$ = new_binary_node(OP_ASSIGN, id, $3);
        }
    | method_call ';' { $$ = $1; }
    | IF '(' expr ')' THEN block ELSE block { $$ = new_if_node($3, $6, $8); }
    | WHILE '(' expr ')' block { $$ = new_while_node($3, $5); }
    | RETURN expr ';' { $$ = new_unary_node(OP_RETURN, $2); }
    | RETURN ';'  { $$ = new_unary_node(OP_RETURN, NULL); }
    | ';' { $$ = NULL; }
    | block { $$ = $1; }
    ;

expr:
      ID { $$ = new_leaf_node(TYPE_ID, $1); }
    | method_call { $$ = $1; }
    | literal { $$ = $1; }
    | expr '+' expr { $$ = new_binary_node(OP_ADDITION, $1, $3); }
    | expr '-' expr { $$ = new_binary_node(OP_SUBTRACTION, $1, $3); }
    | expr '*' expr { $$ = new_binary_node(OP_MULTIPLICATION, $1, $3); }
    | expr '/' expr { $$ = new_binary_node(OP_DIVISION, $1, $3); }
    | expr '%' expr { $$ = new_binary_node(OP_MOD, $1, $3); }
    | expr LES expr { $$ = new_binary_node(OP_LES, $1, $3); }
    | expr GRT expr { $$ = new_binary_node(OP_GRT, $1, $3); }
    | expr EQ expr  { $$ = new_binary_node(OP_EQ, $1, $3); }
    | expr NEQ expr { $$ = new_binary_node(OP_NEQ, $1, $3); }
    | expr LEQ expr { $$ = new_binary_node(OP_LEQ, $1, $3); }
    | expr GEQ expr { $$ = new_binary_node(OP_GEQ, $1, $3); }
    | expr AND expr { $$ = new_binary_node(OP_AND, $1, $3); }
    | expr OR expr  { $$ = new_binary_node(OP_OR, $1, $3); }
    | '-' expr %prec UMINUS { $$ = new_unary_node(OP_MINUS, $2); }
    | NEG expr { $$ = new_unary_node(OP_NEG, $2); }
    | '(' expr ')' { $$ = $2; }
    ;

method_call:
      ID '(' call_args ')' { $$ = new_method_call_node($1, $3); }
      ;

call_args:
      expr_list { $$ = $1; }
    | /* empty */ { $$ = NULL; }
    ;

expr_list:
      expr { $$ = append_expr(NULL, $1); }
    | expr_list ',' expr { $$ = append_expr($1, $3); }
    ;

literal:
    INTEGER_LITERAL { $$ = new_leaf_node(TYPE_INT, &$1); }
  | TRUE           { int v = 1; $$ = new_leaf_node(TYPE_BOOL, &v); }
  | FALSE          { int v = 0; $$ = new_leaf_node(TYPE_BOOL, &v); }
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
    print_full_ast(head_ast);
    print_symbol_table(global_level);
    return 0;
}
