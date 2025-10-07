%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handling.h"
#include "ast.h"
#include "print_funcs.h"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "intermediate_code.h"
#include "symbol.h"

extern int yylex();
extern int yylineno;
void yyerror(const char *s);
extern FILE *yyin;
ARGS_LIST* current_args_list = NULL;

/* flag so that the next block does NOT do push_scope() (used by methods) */
int suppress_next_block_push = 0;
/* indicates whether the most recent block was pushed (to decide pop) */
int last_block_pushed = 0;
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

%type <node> program decls decl var_decl method_decl block statement expr literal method_call else
%type <nodelist> method_args arg_list expr_list call_args statements var_decls 
%type <ival> type

%%

program:
    PROGRAM '{' decls '}' { printf("No syntactic errors.\n"); }
    ;

decls:
      decls decl {  if ($2) add_sentence($2); }
    | /* empty */ { $$ = NULL; }  
    ;

decl:
      var_decl    { $$ = $1; }
    | method_decl { $$ = $1; }
    ;

var_decl:
      type ID '=' expr ';'
        {
          ID_TABLE* dir;
          AST_NODE* id;
          if ($1 == INTEGER) {
            dir = add_id($2, TYPE_INT);
            id = new_leaf_node(TYPE_ID, dir);
          } else {
            dir = add_id($2, TYPE_BOOL);
            id = new_leaf_node(TYPE_ID, dir);
          }
          $$ = new_binary_node(OP_DECL, id, $4);
        }
    | type ID ';'
        {
          ID_TABLE* dir;
          AST_NODE* id;
          if ($1 == INTEGER) {
            dir = add_id($2, TYPE_INT);
            id = new_leaf_node(TYPE_ID, dir);
          } else {
            dir = add_id($2, TYPE_BOOL);
            id = new_leaf_node(TYPE_ID, dir);
          }
          $$ = new_unary_node(OP_DECL, id);
        }
    ;

var_decls:
            var_decls var_decl { $$ = append_expr($1, $2); }
        | /* empty */ { $$ = NULL; }
        ;

method_decl:
    VOID ID '(' method_args ')' block {
        add_method($2, RETURN_VOID, get_this_scope(), 0);
        add_current_list($2, current_args_list);
        $$ = new_method_decl_node($2, $6);
        current_args_list = NULL;
        pop_scope();
    }
  |
    VOID ID '(' method_args ')' EXTERN ';' {
        add_method($2, RETURN_VOID, get_this_scope(), 1);
        add_current_list($2, current_args_list);
        $$ = new_method_decl_node($2, NULL);
        current_args_list = NULL;
        pop_scope();
    }
  |
    type ID '(' method_args ')' block {
        if ($1 == INTEGER) add_method($2, RETURN_INT, get_this_scope(), 0);
        else if ($1 == BOOL) add_method($2, RETURN_BOOL, get_this_scope(), 0);
        add_current_list($2, current_args_list);
        $$ = new_method_decl_node($2, $6);
        current_args_list = NULL;
        pop_scope();
    }
  |
    type ID '(' method_args ')' EXTERN ';' {
        if ($1 == INTEGER) add_method($2, RETURN_INT, get_this_scope(), 1);
        else if ($1 == BOOL) add_method($2, RETURN_BOOL, get_this_scope(), 1);
        add_current_list($2, current_args_list);
        $$ = new_method_decl_node($2, NULL);
        current_args_list = NULL;
        pop_scope();
    }
;

method_args
    : { push_scope(); suppress_next_block_push = 1; } arg_list {
        $$ = $2;
      }
    | { push_scope(); suppress_next_block_push = 1; } /* empty */ {
        $$ = NULL;
      }
    ;

arg_list
    : type ID  {
        ID_TABLE *dir = add_id($2, ($1 == INTEGER) ? TYPE_INT : TYPE_BOOL);
        $$ = append_expr(NULL, new_leaf_node(TYPE_ID, dir));
        current_args_list = add_arg_current_list(current_args_list, $2, ($1 == INTEGER) ? TYPE_INT : TYPE_BOOL);
      }
    | arg_list ',' type ID {
        ID_TABLE *dir = add_id($4, ($3 == INTEGER) ? TYPE_INT : TYPE_BOOL);
        $$ = append_expr($1, new_leaf_node(TYPE_ID, dir));
        current_args_list = add_arg_current_list(current_args_list, $4, ($3 == INTEGER) ? TYPE_INT : TYPE_BOOL);
      }
    ;


type:
      INTEGER { $$ = INTEGER; }
    | BOOL { $$ = BOOL; }
    ;

block:
    '{' {
        if (suppress_next_block_push) {
            last_block_pushed = 0;
            suppress_next_block_push = 0;
        } else {
            push_scope();
            last_block_pushed = 1;
        }
  } var_decls statements '}' {
    /* Merge lists: statements first (which may contain assignments generated for
                   initializations), then the other statements. */
    AST_NODE_LIST* merged = $3; /* var_decls */
    AST_NODE_LIST* it = $4;     /* statements */
    while (it) {
      if (it->first) {
        merged = append_expr(merged, it->first);
      }
      it = it->next;
    }
    if (merged == NULL) {
        $$ = NULL;
    } else {
        $$ = new_block_node(merged);
    }
    if (last_block_pushed) {
        pop_scope();
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
          ID_TABLE* dir = find($1);
          if (!dir) {
            error_variable_not_declared(yylineno, $1);
          }
          AST_NODE* id = new_leaf_node(TYPE_ID, dir);
          $$ = new_binary_node(OP_ASSIGN, id, $3);
        }
    | method_call ';' { $$ = $1; }
    | IF '(' expr ')' THEN block else { $$ = new_if_node($3, $6, $7); }
    | WHILE '(' expr ')' block { $$ = new_while_node($3, $5); }
    | RETURN expr ';' { $$ = new_unary_node(OP_RETURN, $2); }
    | RETURN ';'  { $$ = new_unary_node(OP_RETURN, NULL); }
    | ';' { $$ = NULL; }
    | block { $$ = $1; }
    ;

else :
        ELSE block { $$ = $2; }
    |  /* empty */ { $$ = NULL; }
    ;

expr:
      ID {
        ID_TABLE* dir = find($1);
        if (!dir) {
          error_variable_not_declared(yylineno, $1);
        }
        $$ = new_leaf_node(TYPE_ID, dir);
      }
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
    semantic_analyzer(head_ast);
    // Generate intermediate code for each top-level method declaration
    reset_code();
    for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
        gen_code(cur->sentence, NULL);
    }
    print_code_to_file("intermediate_code/intermediate_code.out");
    return 0;
}
