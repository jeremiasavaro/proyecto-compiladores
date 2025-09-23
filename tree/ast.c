#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error_handling.h"
#include "ast.h"
#include "utils.h"

int returnInt;
int returnBool;
AST_ROOT *head_ast = NULL;
AST_ROOT *end_ast = NULL;

extern int yylineno;

static AST_NODE* alloc_node(void) {
    AST_NODE* node = (AST_NODE*) malloc(sizeof(AST_NODE));
    node->father = NULL;
    node->type = AST_NULL;
    node->line = -1;
    return node;
}

AST_NODE* new_leaf_node(LEAF_TYPE type, void* value) {
    AST_NODE* node = alloc_node();
    node->type = AST_LEAF;
    node->leaf.leaf_type = type;
    if (type == TYPE_INT) {
        node->leaf.value = malloc(sizeof(union LEAF));
        node->leaf.value->int_leaf.type = TYPE_INT;
        node->leaf.value->int_leaf.value = *((int*) value);
    } else if (type == TYPE_BOOL) {
        node->leaf.value = malloc(sizeof(union LEAF));
        node->leaf.value->bool_leaf.type = TYPE_BOOL;
        node->leaf.value->bool_leaf.value = *((int*) value);
    } else if (type == TYPE_ID) {
        /* Create ID leaf: store a copy of the identifier string */
        node->leaf.value = malloc(sizeof(union LEAF));
        if (!node->leaf.value) {
            free(node);
            return NULL;
        }
        node->leaf.value->id_leaf = (ID_TABLE*) value;
        if (!node->leaf.value->id_leaf) {
            free(node->leaf.value);
            free(node);
            return NULL;
        }
    }
    return node;
}

AST_NODE* new_binary_node(OPERATOR op, AST_NODE* left, AST_NODE* right) {
    AST_NODE* node = alloc_node();
    node->type = AST_COMMON;
    node->common.arity = BINARY;
    node->common.op = op;
    node->common.left = left;
    node->common.right = right;
    if (left) left->father = node;
    if (right) right->father = node;
    return node;
}

AST_NODE* new_unary_node(OPERATOR op, AST_NODE* left) {
    AST_NODE* node = alloc_node();
    node->type = AST_COMMON;
    node->common.arity = UNARY;
    node->common.op = op;
    node->common.left = left;
    node->common.right = NULL;
    if (left) left->father = node;
    return node;
}

AST_NODE* new_if_node(AST_NODE* condition, AST_NODE* then_block, AST_NODE* else_block) {
    AST_NODE* node = alloc_node();
    node->type = AST_IF;
    node->if_stmt.condition = condition;
    node->if_stmt.then_block = then_block;
    node->if_stmt.else_block = else_block;
    if (condition) condition->father = node;
    if (then_block) then_block->father = node;
    if (else_block) else_block->father = node;
    return node;
}

AST_NODE* new_while_node(AST_NODE* condition, AST_NODE* block) {
    AST_NODE* node = alloc_node();
    node->type = AST_WHILE;
    node->while_stmt.condition = condition;
    node->while_stmt.block = block;
    if (condition) condition->father = node;
    if (block) block->father = node;
    return node;
}

AST_NODE* new_method_node(char* name, AST_NODE_LIST* args, AST_NODE* block, int is_extern) {
    AST_NODE* node = alloc_node();
    node->type = AST_METHOD;
    node->method.name = my_strdup(name); // save a copy of the name 
    node->method.args = args; 
    node->method.block = block;
    node->method.is_extern = is_extern;
    if (args) {
        AST_NODE_LIST* it = args;
        while (it) {
            if (it->first) it->first->father = node; // asign father to each arg
            it = it->next;
        }
    }
    if (block) block->father = node;
    return node;
}

AST_NODE* new_block_node(AST_NODE_LIST* stmts) {
    AST_NODE* node = alloc_node();
    node->type = AST_BLOCK;
    node->block.stmts = stmts;
    if (stmts) {
        AST_NODE_LIST* it = stmts;
        while (it) {
            if (it->first) it->first->father = node; // asign father to each stmt
            it = it->next;
        }
    }
    return node;
}

void create_root(AST_NODE* tree) {
    head_ast = (AST_ROOT*) malloc(sizeof(AST_ROOT));
    head_ast->sentence = tree;
    head_ast->next = NULL;
    end_ast = head_ast;
}

void add_sentence(AST_NODE* tree) {
    if (head_ast == NULL) {
        create_root(tree);
    } else {
        AST_ROOT *aux = malloc(sizeof(AST_ROOT));
        aux->sentence = tree;
        aux->next = NULL;
        end_ast->next = aux;
        end_ast = aux;
    }
}

void free_mem(AST_NODE* node) {
    if (!node) return;
    switch (node->type) {
        case AST_IF:
            free_mem(node->if_stmt.condition);
            free_mem(node->if_stmt.then_block);
            free_mem(node->if_stmt.else_block);
            break;
        case AST_WHILE:
            free_mem(node->while_stmt.condition);
            free_mem(node->while_stmt.block);
            break;
        case AST_BLOCK: {
            AST_NODE_LIST* list_stmts = node->block.stmts;
            while (list_stmts) {
                if (list_stmts->first) free_mem(list_stmts->first);
                AST_NODE_LIST* next = list_stmts->next;
                free(list_stmts);
                list_stmts = next;
            }
            break;
        }
        case AST_METHOD: {
            free(node->method.name);
            AST_NODE_LIST* args_list = node->method.args;
            while (args_list) {
                if (args_list->first) free_mem(args_list->first);
                AST_NODE_LIST* next = args_list->next;
                free(args_list);
                args_list = next;
            }
            free_mem(node->method.block);
            break;
        }
        case AST_COMMON:
            free_mem(node->common.left);
            free_mem(node->common.right);
            break;

        case AST_LEAF:
            if (node->leaf.leaf_type == TYPE_INT || node->leaf.leaf_type == TYPE_BOOL) {
                free(node->leaf.value);
            }
            break;
        default:
            break;
    }
    free(node);
}

AST_NODE* new_method_call_node(char* name, AST_NODE_LIST* args) {
    AST_NODE* node = alloc_node();
    node->type = AST_METHOD;
    node->method.name = my_strdup(name);
    node->method.args = args;
    node->method.block = NULL;
    node->method.is_extern = 0;
    node->father = NULL;
    node->line = yylineno;
    if (args) {
        AST_NODE_LIST* args_list = args;
        while (args_list) {
            if (args_list->first) args_list->first->father = node;
            args_list = args_list->next;
        }
    }
    return node;
}
// method utilized for build lists of expressions (statements, args, etc)
AST_NODE_LIST* append_expr(AST_NODE_LIST* list, AST_NODE* expr) {
    if (!expr) return list;
    AST_NODE_LIST* new_node = malloc(sizeof(AST_NODE_LIST));
    new_node->first = expr;
    new_node->next = NULL;
    if (!list) {
        return new_node;
    } else {
        AST_NODE_LIST* aux = list;
        while (aux->next) aux = aux->next;
        aux->next = new_node;
        return list;
    }
}
