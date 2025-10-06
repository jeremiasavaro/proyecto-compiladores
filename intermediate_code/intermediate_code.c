#include "intermediate_code.h"

// Buffer for save all the instructions (pseudo-assembly)
Instr code[1000];   // 1000 instructions for example
int code_size = 0;  // Number of instructions saved

static int temp_counter = 0;
static int label_counter = 0;

/* Function to generate new temporary variables
 */
char* new_temp() {
    char buf[32];
    sprintf(buf, "T%d", temp_counter++);
    return my_strdup(buf);
}

/* Function to generate new labels for jumps
 */
static char* new_label() {
    char buf[32];
    sprintf(buf, "L%d", label_counter++);
    return my_strdup(buf);
}

/* Function for save instructions in the buffer
 */
void emit(InstrType t, const char* var1, const char* var2, const char* reg) {
    code[code_size].type = t;
    if (var1) {
        strcpy(code[code_size].var1, var1);
    } else {
        code[code_size].var1[0] = '\0';     //If received NULL, save empty string
    }
    if (var2) {
        strcpy(code[code_size].var2, var2);
    } else {
        code[code_size].var2[0] = '\0';     //If received NULL, save empty string
    }
    if(reg) {
        strcpy(code[code_size].reg, reg);
    } else {
        code[code_size].reg[0] = '\0';     //If received NULL, save empty string
    }
    code_size++;
}

/* Function that generates code for leaf nodes
 */
static void gen_code_leaf(AST_NODE* node, char** result) {
    char buf[32];
    switch (node->leaf.leaf_type) {
        case TYPE_INT: {
            sprintf(buf, "%d", node->leaf.value->int_leaf.value);
            char* val = my_strdup(buf);
            emit(I_LOADVAL, val, NULL, NULL);
            if (result) *result = val;
            break;
        }
        case TYPE_BOOL: {
            sprintf(buf, "%d", node->leaf.value->bool_leaf.value);
            char* val = my_strdup(buf);
            emit(I_LOADVAL, val, NULL, NULL);
            if (result) *result = val;
            break;
        }
        case TYPE_ID: {
            ID_TABLE* sym = node->leaf.value->id_leaf; // Symbol
            if (sym) {
                emit(I_LOAD, sym->id_name, NULL, NULL);
                if (result) *result = sym->id_name; // Stable name
            }
            break;
        }
    }
}

/* Function that generates code for common expressions
 */
static void gen_code_common(AST_NODE* node, char** result) {
    char* left = NULL;
    char* right = NULL;
    char* temp = NULL;

    switch (node->common.op) {
        case OP_ADDITION:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_ADD, left, right, temp);
            *result = temp;
            break;

        case OP_SUBTRACTION:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_SUB, left, right, temp);
            *result = temp;
            break;

        case OP_MULTIPLICATION:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_MUL, left, right, temp);
            *result = temp;
            break;

        case OP_DIVISION:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_DIV, left, right, temp);
            *result = temp;
            break;

        case OP_MOD:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_MOD, left, right, temp);
            *result = temp;
            break;

        case OP_MINUS:
            gen_code(node->common.left, &left);
            temp = new_temp();
            emit(I_MIN, left, NULL, temp);
            *result = temp;
            break;

        case OP_RETURN:
            if (node->common.left) {
                gen_code(node->common.left, &left);
                emit(I_RET, left, NULL, NULL);
            } else {
                emit(I_RET, NULL, NULL, NULL);
            }
            break;

        case OP_LES:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_LES, left, right, temp);
            *result = temp;
            break;

        case OP_GRT:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_GRT, left, right, temp);
            *result = temp;
            break;

        case OP_EQ:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_EQ, left, right, temp);
            *result = temp;
            break;

        case OP_NEQ:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_NEQ, left, right, temp);
            *result = temp;
            break;

        case OP_LEQ:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_LEQ, left, right, temp);
            *result = temp;
            break;

        case OP_GEQ:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_GEQ, left, right, temp);
            *result = temp;
            break;

        case OP_AND:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_AND, left, right, temp);
            *result = temp;
            break;

        case OP_OR:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            temp = new_temp();
            emit(I_OR, left, right, temp);
            *result = temp;
            break;
        case OP_NEG:
            gen_code(node->common.left, &left);
            temp = new_temp();
            emit(I_NEG, left, NULL, temp);
            *result = temp;
            break;
        case OP_ASSIGN:
            gen_code(node->common.left, &left);
            gen_code(node->common.right, &right);
            emit(I_STORE, right, NULL, left);
            break;
        case OP_DECL:
            gen_code(node->common.left, &left);
            if (node->common.right) {
                gen_code(node->common.right, &right);
                emit(I_STORE, right, NULL, left);
            }
            break;
        default:
            break;
    }
}

/* Function that generates code for if expressions
 */
static void gen_code_if(AST_NODE* node, char** result) {
    char* cond_temp = NULL;
    gen_code(node->if_stmt.condition, &cond_temp);
    char* else_label = new_label();
    char* end_label = new_label();

    // Jump for false to else (or end if no else block)
    emit(I_JMPF, cond_temp, NULL, else_label);
    // Then block
    gen_code(node->if_stmt.then_block, NULL);
    // Jump to end after then
    emit(I_JMP, end_label, NULL, NULL);
    // Else label
    emit(I_LABEL, else_label, NULL, NULL);
    if (node->if_stmt.else_block) {
        gen_code(node->if_stmt.else_block, NULL);
    }
    // End label
    emit(I_LABEL, end_label, NULL, NULL);
}

/* Function that generates code for while expressions
 */
static void gen_code_while(AST_NODE* node, char** result) {
    char* start_label = new_label();
    char* end_label = new_label();

    emit(I_LABEL, start_label, NULL, NULL);
    char* cond_temp = NULL;
    gen_code(node->while_stmt.condition, &cond_temp);
    emit(I_JMPF, cond_temp, NULL, end_label);
    gen_code(node->while_stmt.block, NULL);
    emit(I_JMP, start_label, NULL, NULL);
    emit(I_LABEL, end_label, NULL, NULL);
}

/* Function that generates code for method declarations
 */
static void gen_code_method_decl(AST_NODE* node, char** result) {
    if (node->method_decl.is_extern) {
        emit(I_EXTERN, node->method_decl.name, NULL, NULL);
        return;
    }
    emit(I_ENTER, node->method_decl.name, NULL, NULL);
    if (!node->method_decl.is_extern && node->method_decl.block) {
        gen_code(node->method_decl.block, NULL);
    }
    emit(I_LEAVE, node->method_decl.name, NULL, NULL);
}

/* Function that generates code for method calls
 */
static void gen_code_method_call(AST_NODE* node, char** result) {
    AST_NODE_LIST* arg = node->method_call.args;
    while (arg) {
        char* arg_temp = NULL;
        gen_code(arg->first, &arg_temp);
        emit(I_PARAM, arg_temp, NULL, NULL);
        arg = arg->next;
    }
    // Allocate a temp for return value always.
    char* ret_temp = new_temp();
    emit(I_CALL, node->method_call.name, NULL, ret_temp);
    if (result) {
        *result = ret_temp;
    }
}

/* Function that generates code for blocks
 */
static void gen_code_block(AST_NODE* node, char** result) {
    // Iterate statements generating code; last expression result not propagated unless explicitly requested.
    AST_NODE_LIST* cur = node->block.stmts;
    char* last_temp = NULL;
    while (cur) {
        char* stmt_res = NULL;
        gen_code(cur->first, &stmt_res);
        if (stmt_res) {
            last_temp = stmt_res; // Track last result
        }
        cur = cur->next;
    }
    if (result && last_temp) {
        *result = last_temp;
    }
}

void print_code_to_file(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Can't open the file provided");
        return;
    }

    for (int i = 0; i < code_size; i++) {
        switch (code[i].type) {
            case I_LOADVAL:
                fprintf(f, "LOADVAL %s\n", code[i].var1);
                break;
            case I_LOAD:
                fprintf(f, "LOAD %s\n", code[i].var1);
                break;
            case I_STORE:
                fprintf(f, "STORE %s, %s\n", code[i].var1, code[i].reg);
                break;
            case I_ADD:
                fprintf(f, "ADD %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_SUB:
                fprintf(f, "SUB %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_MUL:
                fprintf(f, "MUL %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_DIV:
                fprintf(f, "DIV %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_MOD:
                fprintf(f, "MOD %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_MIN:
                fprintf(f, "MIN %s, %s\n", code[i].var1, code[i].reg);
                break;
            case I_RET:
                if (code[i].var1[0] != '\0') {
                    fprintf(f, "RET %s\n", code[i].var1);
                } else {
                    fprintf(f, "RET\n");
                }
                break;
            case I_LES:
                fprintf(f, "LES %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_GRT:
                fprintf(f, "GRT %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_EQ:
                fprintf(f, "EQ %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_NEQ:
                fprintf(f, "NEQ %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_LEQ:
                fprintf(f, "LEQ %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_GEQ:
                fprintf(f, "GEQ %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_AND:
                fprintf(f, "AND %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_OR:
                fprintf(f, "OR %s, %s, %s\n", code[i].var1, code[i].var2, code[i].reg);
                break;
            case I_NEG:
                fprintf(f, "NEG %s, %s\n", code[i].var1, code[i].reg);
                break;
            case I_LABEL:
                fprintf(f, "%s:\n", code[i].var1);
                break;
            case I_JMP:
                fprintf(f, "JMP %s\n", code[i].var1);
                break;
            case I_JMPF:
                fprintf(f, "JMPF %s, %s\n", code[i].var1, code[i].reg);
                break;
            case I_PARAM:
                fprintf(f, "PARAM %s\n", code[i].var1);
                break;
            case I_CALL:
                fprintf(f, "CALL %s, %s\n", code[i].var1, code[i].reg);
                break;
            case I_ENTER:
                fprintf(f, "ENTER %s\n", code[i].var1);
                break;
            case I_LEAVE:
                fprintf(f, "LEAVE %s\n", code[i].var1);
                break;
            case I_EXTERN: 
                fprintf(f, "EXTERN %s\n", code[i].var1);
                break;
            default:
                fprintf(f, "UNKNOWN\n");
                break;
        }
    }
    fclose(f);
}

/* Function that generates the pseudo-assembly
 */
void gen_code(AST_NODE* node, char** result) {
    if (!node) return;
    switch (node->type) {
        case AST_COMMON:
            gen_code_common(node, result);
            break;
        case AST_IF:
            gen_code_if(node, result);
            break;
        case AST_WHILE:
            gen_code_while(node, result);
            break;
        case AST_METHOD_DECL:
            gen_code_method_decl(node, result);
            break;
        case AST_METHOD_CALL:
            gen_code_method_call(node, result);
            break;
        case AST_BLOCK:
            gen_code_block(node, result);
            break;
        case AST_LEAF:
            gen_code_leaf(node, result);
            break;
        default:
            break;
    }
}

void reset_code() {
    code_size = 0;
    temp_counter = 0;
    label_counter = 0;
}