#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error_handling.h"
#include "utils.h"
#include "symbol_table.h"

// Forward declarations to avoid circular dependencies.
typedef struct ID_TABLE ID_TABLE;
typedef struct TABLE_STACK TABLE_STACK;

typedef struct AST_NODE_LIST AST_NODE_LIST;
typedef struct AST_NODE AST_NODE;
typedef struct AST_ROOT AST_ROOT;
typedef struct INT_LEAF INT_LEAF;
typedef struct BOOL_LEAF BOOL_LEAF;

extern AST_ROOT *head_ast;
extern AST_ROOT *end_ast;

typedef enum {
    UNARY,
    BINARY
} OPERATOR_ARITY;

typedef enum {
    OP_ADDITION,
    OP_SUBTRACTION,
    OP_MULTIPLICATION,
    OP_DIVISION,
    OP_MOD,
    OP_LES,
    OP_GRT,
    OP_EQ,
    OP_NEQ,
    OP_LEQ,
    OP_GEQ,
    OP_MINUS,
    OP_AND,
    OP_OR,
    OP_NEG,
    OP_ASSIGN,
    OP_RETURN,
    OP_DECL
} OPERATOR;

typedef enum {
	TYPE_INT,
	TYPE_BOOL,
	TYPE_ID
} LEAF_TYPE;

struct INT_LEAF {
    LEAF_TYPE type;
    int value;
};

struct BOOL_LEAF {
    LEAF_TYPE type;
    int value;
};

union LEAF {
    INT_LEAF int_leaf;
    BOOL_LEAF bool_leaf;
    ID_TABLE* id_leaf;
};

// Enum for different AST node types.
typedef enum {
    AST_COMMON,
    AST_IF,
    AST_WHILE,
    AST_METHOD_DECL,
    AST_METHOD_CALL,
    AST_BLOCK,
    AST_LEAF,
    AST_NULL, // Type created for initialization.
} AST_TYPE;

struct AST_NODE {
    AST_TYPE type;
    int line;
    AST_NODE* father;
    union {
        struct {
            OPERATOR_ARITY arity;
            OPERATOR op;
            AST_NODE* left;
            AST_NODE* right;
        } common;

        struct {
            AST_NODE* condition;
            AST_NODE* then_block;
            AST_NODE* else_block;
        } if_stmt;

        struct {
            AST_NODE* condition;
            AST_NODE* block;
        } while_stmt;

        struct {
            char* name;
            int num_args; // Amount of arguments.
            AST_NODE_LIST* args; // Arguments list.
            AST_NODE* block; // Method body.
            TABLE_STACK* scope; // Scope of the method.
            int is_extern; // Flag to check if the method is externally defined.
        } method_decl;

        struct {
            char* name;
            int num_args; // Amount of arguments.
            AST_NODE_LIST* args; // Arguments list.
        } method_call;

        struct {
            AST_NODE_LIST* stmts; // Statements list.
        } block;

        // For leaf nodes.
        struct {
            LEAF_TYPE leaf_type;
            union LEAF* value;
        } leaf;
    };
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
AST_NODE* new_leaf_node(LEAF_TYPE type, void* value);
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
AST_NODE* new_method_decl_node(char* name, AST_NODE_LIST* args, AST_NODE* block, TABLE_STACK* scope, int is_extern);
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