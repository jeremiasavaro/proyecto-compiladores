#include "semantic_analyzer.h"

int line = 0;
int returned_global = 0; // Global flag set when a return statement has been encountered and propagated.
TYPE method_return_type; // Current method's expected return TYPE (used when checking return statements).

/*
 * Function that calls the correct evaluator depending on the AST node type.
 * Also resets global variable returned_global when needed.
 */
void eval(AST_NODE *tree, TYPE *ret);

/*
 * Recursively evaluates an AST_COMMON node and stores its type and value in ‘ret’.
 * Booleans are represented as 0 (false) or 1 (true).
 * Performs type checking on every operation and variable.
 */
static void eval_common(AST_NODE *tree, TYPE *ret) {
    line = tree->line;
    TYPE left_type;
    TYPE right_type;
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
                error_less(line);
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
                error_type_mismatch(line, id->id_name, id->id_type == CONST_INT ? "INT" : "BOOL");
            }

            *ret = right_type;
            return;
        }
        case OP_RETURN: {
            if (tree->common.left) {                
                eval(tree->common.left, &left_type);
                if (left_type != method_return_type) {
                    error_return_type(tree->line, left_type, method_return_type);
                }
                memcpy(ret, &left_type, sizeof(TYPE));
                returned_global = 1;
            } else {
                if (method_return_type != VOID_TYPE) {
                    error_return_type_void(tree->line, method_return_type);
                }
                *ret = VOID_TYPE;
                returned_global = 1;
            }
            return;
        }
    }
    error_unknown_operator(line);
}

/*
 * Evaluates a while-statement node: checks if the loop condition is boolean and
 * evaluates the loop body. Reports an error if condition is not boolean.
 */
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

/*
 * Evaluates a block of statements. Iterates statements, tracks and reports
 * ignored code after returns, and sets `ret` to the returned type if a return
 * is found; otherwise sets NULL_TYPE unless a return was globally flagged.
 */
static void eval_block(AST_NODE *tree, TYPE *ret){
    line = tree->line;
    AST_NODE_LIST *aux = tree->block.stmts;
    TYPE auxRet;
    int returned = 0;
    while (aux != NULL) {
        if (returned_global) {
            warning_ignored_line(aux->first->line);
        }
        eval(aux->first, &auxRet);
        if (returned && !returned_global) {
            warning_ignored_line(aux->first->line);
        }
        if (aux->first->type == AST_COMMON && aux->first->common.op == OP_RETURN) {
            returned = 1;
            memcpy(ret, &auxRet, sizeof(TYPE)); // When we find a return, we copy its type to ret, the other statements are ignored.
        }
        aux = aux->next;
    }

    // If no statement was a return inside this block.
    if (!returned && !returned_global) {
        *ret = NULL_TYPE;
    }
}

/*
 * Evaluates a leaf AST node (literal or identifier). Sets to ret the corresponding
 * TYPE (INT_TYPE, BOOL_TYPE) or reports errors for unknown ID's/types.
 */
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
                error_non_existent_id(line);
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
static void eval_if(AST_NODE *tree, TYPE *ret) {
    line = tree->line;
    TYPE retCondition;
    TYPE retThen = NULL_TYPE;
    TYPE retElse = NULL_TYPE;
    AST_NODE* condition = tree->if_stmt.condition;
    AST_NODE* then_block = tree->if_stmt.then_block;
    AST_NODE* else_block = tree->if_stmt.else_block;
    // Ensure condition is boolean.
    eval(condition, &retCondition);
    if(retCondition != BOOL_TYPE) {
        error_conditional(line);
    }
    eval(then_block, &retThen);
    if (else_block) {
        eval(else_block, &retElse);
    }
    // Checks if return statements were encountered inside then and (optional) else block.
    if (retThen != NULL_TYPE) {
        if (retElse != NULL_TYPE) { // If both blocks return something.
            returned_global = 1;
        }
        *ret = retThen;
    } else {
        if (retElse != NULL_TYPE) {
            *ret = retElse;
        } else { // If no block returns anything.
            *ret = NULL_TYPE;
        }
    }
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
    ID_TABLE* method = find_global(tree->method_call.name);
    if (!method) {
        error_method_not_found(tree->method_call.name);
    }
    if (method->id_type != METHOD) {
        error_type_mismatch(line, tree->method_call.name, "METHOD");
    }
    ARGS_LIST* method_args = method->method.arg_list;
    AST_NODE_LIST* call_args = tree->method_call.args;
    if (method->method.num_args != tree->method_call.num_args) {
        error_args_number(line, method->id_name, method->method.num_args);
    }

    while (method_args && call_args) {
        eval(call_args->first, ret);
        ID_TYPE auxType = UNKNOWN;
        switch (*ret) {
            case INT_TYPE:
                auxType = CONST_INT;
                break;
            case BOOL_TYPE:
                auxType = CONST_BOOL;
                break;
            default:
                error_type_mismatch(line,  method_args->arg->name, "INT or BOOL \n");
        }
        if (method_args->arg->type != auxType) {
            error_type_parameter(line,  method_args->arg->name, method_args->arg->type == CONST_INT ? "INT" : "BOOL");
        }
        method_args = method_args->next;
        call_args = call_args->next;
    }

    switch (method->method.return_type) {
        case RETURN_INT:
            *ret = INT_TYPE;
            break;
        case RETURN_BOOL:
            *ret = BOOL_TYPE;
            break;
        case RETURN_VOID:
            *ret = VOID_TYPE;
            break;
        default:
            error_type_mismatch_method(line, method->id_name, method->method.return_type);
    }
}

/*
 * Evaluates a method declaration node: sets the expected method return type,
 * evaluates the method body (unless extern), and checks for missing return when needed.
 */
static void eval_method_decl(AST_NODE *tree, TYPE *ret) {
    line = tree->line;
    ID_TABLE* method = find_global(tree->method_decl.name);
    if (!method) {
        error_method_not_found(tree->method_decl.name);
    }
    switch (method->method.return_type) {
        case RETURN_INT:
            method_return_type = INT_TYPE;
            break;
        case RETURN_BOOL:
            method_return_type = BOOL_TYPE;
            break;
        case RETURN_VOID:
            method_return_type = VOID_TYPE;
            break;
    }
    if (!tree->method_decl.is_extern) {
        eval(tree->method_decl.block, ret);
    }
    if (*ret == NULL_TYPE && method_return_type != VOID_TYPE) { // If no return was found and method should return something.
        error_missing_return(tree->method_decl.name, method_return_type);
    }
}

/*
 * Function that calls the correct evaluator depending on the AST node type.
 * Also resets global variable returned_global when needed.
 */
void eval(AST_NODE *tree, TYPE *ret){
    if (!tree){
        error_null_node(-1);
    }
    switch (tree->type) {
        case AST_COMMON:
            eval_common(tree, ret);
            return;
        case AST_IF:
            eval_if(tree, ret);
            return;
        case AST_WHILE:
            eval_while(tree);
            return;
        case AST_METHOD_DECL:
            eval_method_decl(tree, ret);
            returned_global = 0;
            return;
        case AST_METHOD_CALL:
            eval_method_call(tree, ret);
            return;
        case AST_BLOCK:
            eval_block(tree, ret);
            if (tree->father->type != AST_BLOCK && returned_global) {
                returned_global = 0;
            }
            return;
        case AST_LEAF:
            eval_leaf(tree, ret);
            return;
        default:
            error_null_node(tree->line);
    }
}

/* Public function: checks the semantic of a tree */
void semantic_analyzer(AST_ROOT *tree) {
    TYPE ret;
    for (AST_ROOT* cur = tree; cur != NULL; cur = cur->next) {
        eval(cur->sentence, &ret);
    }
}
