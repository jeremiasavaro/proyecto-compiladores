#include "intermediate_code.h"

// Buffer for save all the instructions (pseudo-assembly)
Instr code[1000];   // 1000 instructions for example
int code_size = 0;  // Number of instructions saved

static int temp_counter = 0;
static int label_counter = 0;

// Function to generate new temporary variables
char* new_temp() {
    char buf[32];
    sprintf(buf, "T%d", temp_counter++);
    return my_strdup(buf);
}

// Function to generate new labels for jumps
static char* new_label() {
    char buf[32];
    sprintf(buf, "L%d", label_counter++);
    return my_strdup(buf);
}

// Function for save instructions in the buffer
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

static void genCode_leaf(AST_NODE* node, char** result) {
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

static void genCode_common(AST_NODE* node, char** result) {
    char* left = NULL;
    char* right = NULL;
    char* temp = NULL;

    switch (node->common.op) {
        case OP_ADDITION:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_ADD, left, right, temp);
            *result = temp;
            break;

        case OP_SUBTRACTION:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_SUB, left, right, temp);
            *result = temp;
            break;

        case OP_MULTIPLICATION:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_MUL, left, right, temp);
            *result = temp;
            break;

        case OP_DIVISION:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_DIV, left, right, temp);
            *result = temp;
            break;

        case OP_MOD:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_MOD, left, right, temp);
            *result = temp;
            break;

        case OP_MINUS:
            genCode(node->common.left, &left);
            temp = new_temp();
            emit(I_MIN, left, NULL, temp);
            *result = temp;
            break;

        case OP_RETURN:
            if (node->common.left) {
                genCode(node->common.left, &left);
                emit(I_RET, left, NULL, NULL);
            } else {
                emit(I_RET, NULL, NULL, NULL);
            }
            break;

        case OP_LES:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_LES, left, right, temp);
            *result = temp;
            break;

        case OP_GRT:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_GRT, left, right, temp);
            *result = temp;
            break;

        case OP_EQ:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_EQ, left, right, temp);
            *result = temp;
            break;

        case OP_NEQ:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_NEQ, left, right, temp);
            *result = temp;
            break;

        case OP_LEQ:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_LEQ, left, right, temp);
            *result = temp;
            break;

        case OP_GEQ:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_GEQ, left, right, temp);
            *result = temp;
            break;

        case OP_AND:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_AND, left, right, temp);
            *result = temp;
            break;

        case OP_OR:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            temp = new_temp();
            emit(I_OR, left, right, temp);
            *result = temp;
            break;

        case OP_NEG:
            genCode(node->common.left, &left);
            temp = new_temp();
            emit(I_NEG, left, NULL, temp);
            *result = temp;
            break;

        case OP_ASSIGN:
            genCode(node->common.left, &left);
            genCode(node->common.right, &right);
            emit(I_STORE, right, NULL, left);
            break;

        default:
            break;
    }
}

static void genCode_if(AST_NODE* node, char** result) {
    char* cond_temp = NULL;
    genCode(node->if_stmt.condition, &cond_temp);
    char* else_label = new_label();
    char* end_label = new_label();

    // Jump for false to else (or end if no else block)
    emit(I_JMPF, cond_temp, NULL, else_label);
    // Then block
    genCode(node->if_stmt.then_block, NULL);
    // Jump to end after then
    emit(I_JMP, end_label, NULL, NULL);
    // Else label
    emit(I_LABEL, else_label, NULL, NULL);
    if (node->if_stmt.else_block) {
        genCode(node->if_stmt.else_block, NULL);
    }
    // End label
    emit(I_LABEL, end_label, NULL, NULL);
}

static void genCode_while(AST_NODE* node, char** result) {
    char* start_label = new_label();
    char* end_label = new_label();

    emit(I_LABEL, start_label, NULL, NULL);
    char* cond_temp = NULL;
    genCode(node->while_stmt.condition, &cond_temp);
    emit(I_JMPF, cond_temp, NULL, end_label);
    genCode(node->while_stmt.block, NULL);
    emit(I_JMP, start_label, NULL, NULL);
    emit(I_LABEL, end_label, NULL, NULL);
}

static void genCode_method_decl(AST_NODE* node, char** result) {
    return;
}

static void genCode_method_call(AST_NODE* node, char** result) {
    return;
}

static void genCode_block(AST_NODE* node, char** result) {
    return;
}

void printCodeToFile(const char* filename) {
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
            case I_DECL:
                fprintf(f, "DECL %s\n", code[i].var1);
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
            default:
                fprintf(f, "UNKNOWN\n");
                break;
        }
    }
    fclose(f);
}

// Generate the pseudo-assembly
void genCode(AST_NODE* node, char** result) {
    if (!node) return;
    switch (node->type) {
        case AST_COMMON:
            genCode_common(node, result);
            break;
        case AST_IF:
            genCode_if(node, result);
            break;
        case AST_WHILE:
            genCode_while(node, result);
            break;
        case AST_METHOD_DECL:
            genCode_method_decl(node, result);
            break;
        case AST_METHOD_CALL:
            genCode_method_call(node, result);
            break;
        case AST_BLOCK:
            genCode_block(node, result);
            break;
        case AST_LEAF:
            genCode_leaf(node, result);
            break;
        default:
            break;
    }
}