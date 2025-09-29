#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

#define true 1
#define false 0

typedef struct AST_NODE_LIST AST_NODE_LIST;
typedef struct AST_NODE AST_NODE;
typedef struct AST_ROOT AST_ROOT;
typedef struct INT_LEAF INT_LEAF;
typedef struct BOOL_LEAF BOOL_LEAF;

// flag used for checking if the program should return an integer or void
// assuming there's no other types main can return
extern int returnInt;
extern int returnBool;
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

// enum for different AST node types
typedef enum {
    AST_COMMON, 
    AST_IF,
    AST_WHILE,
    AST_METHOD_DECL,
    AST_METHOD_CALL,
    AST_BLOCK,
    AST_LEAF,
    AST_NULL, // type created for inicialization
} AST_TYPE;

struct AST_NODE {
    AST_TYPE type;
    int line;
    struct AST_NODE* father;
    union {
        struct {
            OPERATOR_ARITY arity;
            OPERATOR op;
            struct AST_NODE* left;
            struct AST_NODE* right;
        } common;

        struct {
            struct AST_NODE* condition;
            struct AST_NODE* then_block;
            struct AST_NODE* else_block;
        } if_stmt;

        struct {
            struct AST_NODE* condition;
            struct AST_NODE* block;
        } while_stmt;

        struct {
            char* name;
            int num_args; // quantity of arguments
            struct AST_NODE_LIST* args;   // arguments list
            struct AST_NODE* block;      // method body
            TABLE_STACK* scope; // scope of the method
            int is_extern; 
        } method_decl;

        struct {
            char* name;
            int num_args; // quantity of arguments
            struct AST_NODE_LIST* args;   // arguments list
        } method_call;

        struct {
            struct AST_NODE_LIST* stmts; // statements list
        } block;

        // for leaf nodes
        struct {
            LEAF_TYPE leaf_type;
            union LEAF* value;
        } leaf;
    };
};

// struct for linked list of AST nodes (e.g., statements, arguments)
struct AST_NODE_LIST {
    struct AST_NODE* first;
    struct AST_NODE_LIST* next;
};

struct AST_ROOT {
    AST_NODE *sentence;
    AST_ROOT *next;
};

// methods to create different types of AST nodes
AST_NODE* new_unary_node(OPERATOR op, AST_NODE* left);
AST_NODE* new_binary_node(OPERATOR op, AST_NODE* left, AST_NODE* right);
AST_NODE* new_leaf_node(LEAF_TYPE type, void* value);
AST_NODE* new_if_node(AST_NODE* condition, AST_NODE* then_block, AST_NODE* else_block);
AST_NODE* new_while_node(AST_NODE* condition, AST_NODE* block);
AST_NODE* new_method_decl_node(char* name, AST_NODE_LIST* args, AST_NODE* block, TABLE_STACK* scope, int is_extern);
AST_NODE* new_block_node(AST_NODE_LIST* stmts);
AST_NODE* new_method_call_node(char* name, AST_NODE_LIST* args);
AST_NODE_LIST* append_expr(AST_NODE_LIST* list, AST_NODE* expr);
void create_root(AST_NODE* tree);
void add_sentence(AST_NODE* tree);

#endif