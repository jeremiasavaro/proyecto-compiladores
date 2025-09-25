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
                error_additional(line);
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
                error_substraction(line);
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
                error_multiplication(line);
            }
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) * (*(int*)right.value);
            free(left.value);
            free(right.value);
            return;
        case OP_DIVISION:
        case OP_MOD:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_division(line);
            }
            ret->type = INT_TYPE;
            int denom = (*(int*)right.value);
            if (denom == 0) {
                error_division_zero(line);
            }
            ret->value = malloc(sizeof(int));
            if(tree->common.op == OP_DIVISION) {
                *(int*)ret->value = (*(int*)left.value) / (*(int*)right.value);
            } else {
                *(int*)ret->value = (*(int*)left.value) % (*(int*)right.value);
            }
            free(left.value);
            free(right.value);
            return;
        case OP_MINUS:
            eval(tree->common.left, &left);
            if (left.type != INT_TYPE) {
               error_minus(line);
            }
            ret->type = INT_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = - (*(int*)left.value);
            free(left.value);
            return;
        case OP_LES:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_lesser(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) < (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_GRT:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_greater(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) > (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_EQ:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_equal(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) == (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_NEQ:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_not_equal(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) != (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_LEQ:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_less_equal(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) <= (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_GEQ:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != INT_TYPE || right.type != INT_TYPE) {
                error_greater_equal(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = (*(int*)left.value) <= (*(int*)right.value) ? 1 : 0;
            free(left.value);
            free(right.value);
            return;
        case OP_AND:
            eval(tree->common.left, &left);
            eval(tree->common.right, &right);
            if (left.type != BOOL_TYPE || right.type != BOOL_TYPE) {
                error_and(line);
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
                error_or(line);
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
                error_neg(line);
            }
            ret->type = BOOL_TYPE;
            ret->value = malloc(sizeof(int));
            *(int*)ret->value = !(*(int*)left.value) ? 1 : 0;
            free(left.value);
            return;
        case OP_ASSIGN: {
            // Left must be a TYPE_ID leaf
            if (!tree->common.left || tree->common.left->type != AST_LEAF || tree->common.left->leaf.leaf_type != TYPE_ID) {
                error_assign(line);
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
            // Deberiamos corroborar el return dependiendo el tipo de retorno del metodo
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

static void eval_while(AST_NODE *tree, ReturnValueNode *ret){
    line = tree->line;
    AST_NODE* condition = tree->if_stmt.condition;
    AST_NODE* then_block = tree->if_stmt.then_block;
    AST_NODE* else_block = tree->if_stmt.else_block;
    ReturnValueNode retCond;
    ReturnValueNode retBlock;
    eval(tree->while_stmt.condition, &retCond);
    if(condition->type != BOOL_TYPE) {
        error_conditional(line);
    }
    eval(tree->while_stmt.block, &retBlock);
    free(retCond.value);
    free(retBlock.value);
    return;
}


static void eval_block(AST_NODE *tree, ReturnValueNode *ret){
    line = tree->line;
    AST_NODE_LIST *aux = tree->block.stmts;
    ReturnValueNode auxRet;
    while (aux != NULL){
        eval(aux->first, &auxRet);
        if (aux->next != NULL) {
            free(auxRet.value); // only free if not last
        } else {
            *ret = auxRet; // save the last evaluated statement
        }
        aux = aux->next;
    }
    return;
}

static void eval_leaf(AST_NODE *tree, ReturnValueNode *ret){
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
            // Deberiamos corroborar que la variable este en el scope actual
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

/*
    * First, evaluates the condition. If it is not a boolean, returns an error.
    * Then, evaluates the then_block and else_block.
*/

static void eval_if(AST_NODE *tree, ReturnValueNode *ret) {
    line = tree->line;
    ReturnValueNode retCondition;
    ReturnValueNode retThen;
    ReturnValueNode retElse;
    AST_NODE* condition = tree->if_stmt.condition;
    AST_NODE* then_block = tree->if_stmt.then_block;
    AST_NODE* else_block = tree->if_stmt.else_block;
    eval(condition, &retCondition);
    if(retCondition.type != BOOL_TYPE) {
        error_conditional(line);
    }
    eval(then_block, &retThen);
    eval(else_block, &retElse);

    if (*(int*)retCondition.value) {
        ret->type = retThen.type;
        ret->value = malloc(sizeof(int));
        *(int*)ret->value = *(int*)retThen.value;
    } else {
        ret->type = retElse.type;
        ret->value = malloc(sizeof(int));
        *(int*)ret->value = *(int*)retElse.value;
    }

    free(retCondition.value);
    free(retThen.value);
    free(retElse.value);

    return;
}

/*
 * Evaluates a method call node.
 * First, look up the method in the symbol table.
 * If it is not there, we return an error. Otherwise, we obtain the arguments of the method.
 * Next, we go through the arguments of the method and the parameters we are passing in the call 
 * to see if they are of the same type and if the number of arguments is the same. 
 * If either of these two conditions is not met, we return an error.
 */

static void eval_method_call(AST_NODE *tree, ReturnValueNode *ret) {
    line = tree->line;
    // Buscamos el metodo en la tabla de simbolos, si no esta retornamos error.
    // Si esta, obtenemos los argumentos del metodo.
    // Luego recorremos los argumentos del metodo y los parametros que estamos pasando en la llamada
    // para ver si son del mismo tipo y si la cantidad de argumentos es la misma.
    // Si alguna de estas dos condiciones no se cumple, retornamos error.
    return;
}

/*
 * Only eval_block is called.
 */

static void eval_method_decl(AST_NODE *tree, ReturnValueNode *ret) {
    line = tree->line;
    eval(tree->method_decl.block, &ret);
    free(ret->value);
    return;
}

void eval(AST_NODE *tree, ReturnValueNode *ret){
    if (alreadyReturned){   
        warning_already_returned(line);
        return;
    }
    if (!tree){
        error_null_node(-1);
    }
    switch (tree->type) {
        case AST_COMMON:
            eval_common(tree, ret);
        case AST_IF:
            eval_if(tree, ret);
        case AST_WHILE:
            eval_while(tree, ret);
        case AST_METHOD_DECL:
            eval_method_decl(tree, ret);
        case AST_METHOD_CALL:
            eval_method_call(tree, ret);
        case AST_BLOCK:
            eval_block(tree, ret);
        case AST_LEAF:
            eval_leaf(tree, ret);
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
