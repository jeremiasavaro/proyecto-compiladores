#ifndef PRINT_FUNCS_H
#define PRINT_FUNCS_H

#include "ast.h"
#include "symbol_table.h"

void print_ast_node(AST_NODE *node, int indent);
void print_ast_list(AST_NODE_LIST *list, int indent);
void print_full_ast(AST_ROOT *root);
void print_symbol_table(TABLE_STACK* top);

#endif