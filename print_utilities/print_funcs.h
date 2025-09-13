#ifndef PRINT_FUNCS_H
#define PRINT_FUNCS_H

#include "ast.h"

static const char *op_to_string(OPERATOR op);
static void print_leaf(AST_NODE *n);
void print_node(AST_NODE *node, const char *prefix, int is_left);
void print_program_horizontal(void);
void print_tree_ascii(AST_NODE *node, int depth, int is_left);
void print_program_vertical(void);
static const char *id_type_to_string(ID_TYPE t);
void print_id_table(void);

#endif