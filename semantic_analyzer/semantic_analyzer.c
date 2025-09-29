#include <stdio.h>
#include "semantic_analyzer.h"
#include "error_handling.h"

int line = 0;

/*
 * Recursively evaluates an AST_COMMON node and stores its type and value in ‘ret’.
 * Booleans are represented as 0 (false) or 1 (true).
 * Performs type checking on every operation and variable.
 */
static void eval_common(AST_NODE *tree, TYPE *ret) {
    line = tree->line;
    TYPE left_type;
    TYPE right_type;
    if (!tree) {printf("Tree doesnt exists \n");}
    switch (tree->common.op) {
        case OP_ADDITION:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_additional(line);
            }
            *ret = INT_TYPE;
            return;
        case OP_SUBTRACTION:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_substraction(line);
            }
            *ret = INT_TYPE;
            return;
        case OP_MULTIPLICATION:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_multiplication(line);
            }
            *ret = INT_TYPE;
            return;
        case OP_DIVISION:
        case OP_MOD:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_division(line);
            }
            *ret = INT_TYPE;
            return;
        case OP_MINUS:
            eval(tree->common.left, &left_type);
            if (left_type != INT_TYPE) {
               error_minus(line);
            }
            *ret = INT_TYPE;
            return;
        case OP_LES:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_lesser(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_GRT:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_greater(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_EQ:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_equal(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_NEQ:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_not_equal(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_LEQ:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_less_equal(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_GEQ:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != INT_TYPE || right_type != INT_TYPE) {
                error_greater_equal(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_AND:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != BOOL_TYPE || right_type != BOOL_TYPE) {
                error_and(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_OR:
            eval(tree->common.left, &left_type);
            eval(tree->common.right, &right_type);
            if (left_type != BOOL_TYPE || right_type != BOOL_TYPE) {
                error_or(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_NEG:
            eval(tree->common.left, &left_type);
            if (left_type != BOOL_TYPE) {
                error_neg(line);
            }
            *ret = BOOL_TYPE;
            return;
        case OP_ASSIGN: {
            // Left must be a TYPE_ID leaf
            if (!tree->common.left || tree->common.left->type != AST_LEAF || tree->common.left->leaf.leaf_type != TYPE_ID) {
                error_assign(line);
            }
            ID_TABLE *id = tree->common.left->leaf.value->id_leaf;

            eval(tree->common.right, &right_type);

            if ((id->id_type == CONST_INT && right_type != INT_TYPE) ||
                (id->id_type == CONST_BOOL && right_type != BOOL_TYPE)) {
                error_type_mismatch(line, id->id_name, id->id_type == CONST_INT ? "int" : "bool");
            }

            *ret = right_type;
            printf("%s \n", (char*) right_type);
            return;
        }
        case OP_RETURN: {
            if (tree->common.left) {
                eval(tree->common.left, &left_type);
                *ret = left_type;
            } else {
                *ret = VOID_TYPE;
            }
        }
    }
    error_unknown_operator(line);
}

static void eval_while(AST_NODE *tree){
    line = tree->line;
    TYPE retCond;
    TYPE retBlock;
    eval(tree->while_stmt.condition, &retCond);
    if(retCond != BOOL_TYPE) {
        error_conditional(line);
    }
    eval(tree->while_stmt.block, &retBlock);
}


static void eval_block(AST_NODE *tree, TYPE *ret){
    line = tree->line;
    AST_NODE_LIST *aux = tree->block.stmts;
    TYPE auxRet;
    while (aux != NULL){
        eval(aux->first, &auxRet);
        aux = aux->next;
    }
    memcpy(ret, &auxRet, sizeof(TYPE));
}

static void eval_leaf(AST_NODE *tree, TYPE *ret){
    line = tree->line;
    switch (tree->leaf.leaf_type) {
        case TYPE_INT:
            *ret = INT_TYPE;
            return;
        case TYPE_BOOL:
            *ret = BOOL_TYPE;
            return;
        case TYPE_ID: {
            ID_TABLE *id = tree->leaf.value->id_leaf;
            if (!id) {
                error_noexistent_id(line);
            }
            if (id->id_type == CONST_INT) {
                *ret = INT_TYPE;
            } else if (id->id_type == CONST_BOOL) {
                *ret = BOOL_TYPE;
            } else {
                error_id_unknown_type(line, id->id_name);
            }
            return;
        }
    }
    error_unknown_leaf_type(line);
}

/*
    * First, evaluates the condition. If it is not a boolean, returns an error.
    * Then, evaluates the then_block and else_block.
*/

static void eval_if(AST_NODE *tree) {
    line = tree->line;
    TYPE retCondition;
    TYPE retThen;
    TYPE retElse;
    AST_NODE* condition = tree->if_stmt.condition;
    AST_NODE* then_block = tree->if_stmt.then_block;
    AST_NODE* else_block = tree->if_stmt.else_block;
    eval(condition, &retCondition);
    if(retCondition != BOOL_TYPE) {
        error_conditional(line);
    }
    eval(then_block, &retThen);
    eval(else_block, &retElse);
}

/*
 * Evaluates a method call node.
 * First, look up the method in the symbol table.
 * If it is not there, we return an error. Otherwise, we obtain the arguments of the method.
 * Next, we go through the arguments of the method and the parameters we are passing in the call 
 * to see if they are of the same type and if the number of arguments is the same. 
 * If either of these two conditions is not met, we return an error.
 */

static void eval_method_call(AST_NODE *tree, TYPE *ret) {
    line = tree->line;
    // Buscamos el metodo en la tabla de simbolos, si no esta retornamos error.
    // Si esta, obtenemos los argumentos del metodo.
    // Luego recorremos los argumentos del metodo y los parametros que estamos pasando en la llamada
    // para ver si son del mismo tipo y si la cantidad de argumentos es la misma.
    // Si alguna de estas dos condiciones no se cumple, retornamos error.
    ID_TABLE* method = find_global(tree->method_call.name);
    if (!method) {
        error_method_not_found(tree->method_call.name);
    }
    if (method->id_type != METHOD) {
        error_type_mismatch(line, tree->method_call.name, (char*)method->id_type);
    }
    ARGS_LIST* method_args = method->method.arg_list;
    AST_NODE_LIST* call_args = tree->method_call.args;
    if (method->method.num_args != tree->method_call.num_args) {
        fprintf(stderr, "Amount of args should be the same \n");
    }

    while (method_args && call_args) {
        eval(call_args->first, ret);
        ID_TYPE auxType;
        switch (*ret) {
            case INT_TYPE:
                auxType = CONST_INT;
                break;
            case BOOL_TYPE:
                auxType = CONST_BOOL;
                break;
            default:
                error_type_mismatch(line, method_args->arg->name, "INT or BOOL \n");
        }
        if (method_args->arg->type != auxType) {
            error_type_mismatch(line, (char*) call_args->first->type, (char*) method_args->arg->type);
        }
        method_args = method_args->next;
        call_args = call_args->next;
    }
}

/*
 * Only eval_block is called.
 */

static void eval_method_decl(AST_NODE *tree, TYPE *ret) {
    line = tree->line;
    eval(tree->method_decl.block, ret);
    ID_TABLE* method = find_global(tree->method_decl.name);
    if (!method) {
        error_method_not_found(tree->method_decl.name);
    }
    RETURN_TYPE auxType;
    switch (*ret) {
        case INT_TYPE:
            auxType = RETURN_INT;
            break;
        case BOOL_TYPE:
            auxType = RETURN_BOOL;
            break;
        case VOID_TYPE:
            auxType = RETURN_VOID;
            break;
    }
    if (auxType != method->method.return_type) {
        error_type_mismatch(line, (char*) ret, tree->method_decl.name);
    }
}

void eval(AST_NODE *tree, TYPE *ret){
    if (!tree){
        error_null_node(-1);
    }
    switch (tree->type) {
        case AST_COMMON:
            eval_common(tree, ret);
            break;
        case AST_IF:
            eval_if(tree);
            break;
        case AST_WHILE:
            eval_while(tree);
            break;
        case AST_METHOD_DECL:
            eval_method_decl(tree, ret);
            break;
        case AST_METHOD_CALL:
            eval_method_call(tree, ret);
            break;
        case AST_BLOCK:
            eval_block(tree, ret);
            break;
        case AST_LEAF:
            eval_leaf(tree, ret);
            break;
    }
}

/* Public function: interprets (evaluates) a tree */
void semantic_analyzer(AST_ROOT *tree) {
    TYPE ret;
    for (AST_ROOT* cur = tree; cur != NULL; cur = cur->next) {
        eval(cur->sentence, &ret);
    }
}
