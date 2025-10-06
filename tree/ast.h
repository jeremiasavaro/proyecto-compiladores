#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error_handling.h"
#include "utils.h"
#include "symbol_table.h"
#include "symbol.h"

// Forward declarations to avoid circular dependencies.
typedef struct ID_TABLE ID_TABLE;
typedef struct TABLE_STACK TABLE_STACK;
typedef struct AST_NODE_LIST AST_NODE_LIST;
typedef struct AST_NODE AST_NODE;
typedef struct AST_ROOT AST_ROOT;

extern AST_ROOT *head_ast;
extern AST_ROOT *end_ast;

struct AST_NODE {
    int line;
    AST_NODE* father;
    INFO* info;
};

// Struct for linked list of AST nodes (statements, arguments).
struct AST_NODE_LIST {
    AST_NODE* first;
    AST_NODE_LIST* next;
};

struct AST_ROOT {
    AST_NODE *sentence;
    AST_ROOT *next;
};

// Methods to create different types of AST nodes.

/* Function that creates a new unary node, assigning its type and the child.
 * Always assign the child to the left child of the node.
 */
AST_NODE* new_unary_node(OPERATOR op, AST_NODE* left);
/* Function that creates a new binary node, assigning its type and its children.
 */
AST_NODE* new_binary_node(OPERATOR op, AST_NODE* left, AST_NODE* right);
/* Function that creates a new node of type leaf, assigning its type and value.
 */
AST_NODE* new_leaf_node(TYPE type, void* value);
/* Function that creates a new node of type if, assigning its condition and the then block and
 * else block (if it is present).
 */
AST_NODE* new_if_node(AST_NODE* condition, AST_NODE* then_block, AST_NODE* else_block);
/* Function that creates a new node of type while, assigning its condition and the body block
 */
AST_NODE* new_while_node(AST_NODE* condition, AST_NODE* block);
/* Function that creates a new node of type method_decl, assigning its name, arguments, body block, scope and
 * if it is externally defined.
 */
AST_NODE* new_method_decl_node(const char* name, AST_NODE* block);
/* Function that creates a new node of type block, assigning its statements.
 */
AST_NODE* new_block_node(AST_NODE_LIST* stmts);
/* Function that creates a new node of type method_call, assigning its name and arguments.
 */
AST_NODE* new_method_call_node(char* name, AST_NODE_LIST* args);
/* Function utilized for build lists of expressions (statements, args, etc)
 */
AST_NODE_LIST* append_expr(AST_NODE_LIST* list, AST_NODE* expr);
/* Function that creates the root of the ast.
 */
void create_root(AST_NODE* tree);
/* Function that adds a sentence to the ast.
 */
void add_sentence(AST_NODE* tree);

#endif