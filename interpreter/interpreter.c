#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "error_handling.h"

int alreadyReturned = 0;
int line = 0;

/*
 * Recursively evaluates an AST_COMMON node and stores its type and value in ‘ret’.
 * Booleans are represented as 0 (false) or 1 (true).
 * Performs type checking on every operation and variable.
 */
static void eval_common(AST_NODE *tree, ReturnValueNode *ret) {
    line = tree->line;
    ReturnValueNode left;
    ReturnValueNode right;
    switch (tree->common.op) {
        case OP_ADDITION:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                additional_error(line);
            }
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) + (*(int*)right.value); 
            free(left.value);
            free(right.value);
            return;
        case OP_SUBTRACTION:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                substraction_error(line);
            }
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) - (*(int*)right.value);
            free(left.value);
            free(right.value);
            return;
        case OP_MULTIPLICATION:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                multiplication_error(line);
            }
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) * (*(int*)right.value);
            free(left.value);
            free(right.value);
            return;
        case OP_DIVISION: {
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                division_error(line);
            }
            ret->type = INT_TYPE;
            int denom = (*(int*)right.value);
            if (denom == 0) {
                division_zero_error(line);
            }
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) / (*(int*)right.value);
            free(left.value);
            free(right.value);
            return;
        }
        case OP_MINUS:
            eval(tree->common.left, &left);
            if (left.type != INT_TYPE) {
               minus_error(line); 
            }
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = - (*(int*)left.value);
            free(left.value);
            return;
        case OP_AND:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != BOOL_TYPE || right.type != BOOL_TYPE) {
                and_error(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) && (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_OR:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != BOOL_TYPE || right.type != BOOL_TYPE) {
                or_error(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) || (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;        
        case OP_NEG:
            eval(tree->common.left, &left);
            if (left.type != BOOL_TYPE) {
                neg_error(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = !(*(int*)left.value) ? 1 : 0;
            free(left.value);
            return;
        case OP_ASSIGN: {
            // Left must be a TYPE_ID leaf
            if (!tree->common.left || tree->common.left->type != AST_LEAF || tree->common.left->leaf.leaf_type != TYPE_ID) {
                assign_error(line);
            }
            ID_TABLE *id = tree->common.left->leaf.value->id_leaf;

            eval(tree->common.right, &right);

            if ((id->id_type == CONST_INT && right.type != INT_TYPE) ||
                (id->id_type == CONST_BOOL && right.type != BOOL_TYPE)) {
                error_type_mismatch(line, id->id_name, (id->id_type == CONST_INT) ? "int" : "bool");
            }

            ret->type = right.type;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)right.value);
            switch (id->id_type) {
                case CONST_INT: {
                    add_data(id->id_name, CONST_INT, ret->value);
                    break;
                }
                case CONST_BOOL: {
                    int b = (*(int*)ret->value) ? 1 : 0;
                    add_data(id->id_name, CONST_BOOL, &b);
                    break;
                }
                case UNKNOWN:
                    error_id_unknown_type(line, id->id_name);
                    break;
            }
            free(right.value);
            return;
        }
        case OP_DECL_INT:
        case OP_DECL_BOOL:
            // Declaration already recorded in symbol table by parser; do not evaluate identifier
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = 0;
            ret->type = (tree->common.op == OP_DECL_INT) ? INT_TYPE : BOOL_TYPE;
            return;
        case OP_RETURN: {
            if (tree->common.left) {
                if (returnInt) {
                    eval(tree->common.left, &left);
                    if (returnInt && left.type != INT_TYPE) {
                        error_return_int(line);
                    }
                    ret->type = left.type;
                    ret->value = malloc(sizeof(int));
                    *(int*)ret->value = (*(int*)left.value);
                    int return_value = *(int*)ret->value;
                    alreadyReturned = 1;
                    free(left.value);
                    return;
                } else if (returnBool) {
                    eval(tree->common.left, &left);
                    if (left.type != BOOL_TYPE) {
                        error_return_bool(line);
                    }
                    ret->type = left.type;
                    ret->value = malloc(sizeof(int));
                    *(int*)ret->value = (*(int*)left.value);
                    int return_value = *(int*)ret->value;
                    alreadyReturned = 1;
                    free(left.value);
                    return;
                } else {
                    error_return_void(line);
                }
            } else {
                if (returnInt) {
                    error_unespected_return_int(line);
                } else if (returnBool) {
                    error_unespected_return_bool(line);
                } else {
                    alreadyReturned = 1;
                    return;
                }
            }
        }
    }
    error_unknown_operator(line);
}

static void eval_leaf(AST_NODE *tree, ReturnValueNode *ret){
    if (!tree) {
        error_null_node(-1);
    }
    line = tree->line;
    switch (tree->leaf.leaf_type) {
        case TYPE_INT:
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = tree->leaf.value->int_leaf.value;
            return;
        case TYPE_BOOL:
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = tree->leaf.value->bool_leaf.value;
            return;
        case TYPE_ID: {
            ID_TABLE *id = tree->leaf.value->id_leaf;
            if (!id) {
                error_noexistent_id(line);
            }
            if (id->common.data == NULL) {
                error_variable_used_before_init(line, id->id_name);
            }
            if (id->id_type == CONST_INT) {
                ret->type = INT_TYPE;
            } else if (id->id_type == CONST_BOOL) {
                ret->type = BOOL_TYPE;
            } else {
                error_id_unknown_type(line, id->id_name);
            }
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = *(int*)id->common.data;
            return;
        }
    }
    error_unknown_leaf_type(line);
}

static void eval_if(AST_NODE *tree, ReturnValueNode *ret) {
    line = tree->line;
    AST_NODE* condition = tree->if_stmt.condition;
    AST_NODE* then_block = tree->if_stmt.then_block;
    AST_NODE* else_block = tree->if_stmt.else_block;
    eval(condition, &ret);
    if(ret->type != BOOL_TYPE) {
        error_conditional(line);
    }
    if (*(int*)ret->value) { // true
        eval(then_block, &ret);
    } else { // false
        eval(else_block, &ret);
    }
    free(ret->value);
    return;
}

void eval(AST_NODE *tree, ReturnValueNode *ret){
    if (alreadyReturned){
        warning_already_returned(line);
        return;
    }
    switch (tree->type) {
        case AST_COMMON:
            eval_common(tree, &ret);
        case AST_IF:
            eval_if(tree, &ret);
        case AST_WHILE:
        case AST_METHOD:
        case AST_BLOCK:
        case AST_LEAF:
            eval_leaf(tree, &ret);   
    }
    return;
}

/* Public function: interprets (evaluates) a tree */
int interpreter(AST_ROOT *tree) {
    ReturnValueNode ret;
    for (AST_ROOT* cur = tree; cur != NULL; cur = cur->next) {
        eval(cur->sentence, &ret);
    }
    if (alreadyReturned == 0 && (returnInt || returnBool)) {
        error_missing_return(-1);
    }
    return *(int*)ret.value;
}
