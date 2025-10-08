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
/* Function for save instructions in the buffer
 */
void emit(INSTR_TYPE t, INFO* var1, INFO* var2, INFO* reg) {
    // reserve space for a new instruction
    code[code_size].instruct = (INFO*)malloc(sizeof(INFO));
    code[code_size].instruct->type = TABLE_ID;
    code[code_size].instruct->instruct.type_instruct = t;

    if (var1) {
        code[code_size].var1 = (INFO*)malloc(sizeof(INFO));
        *(code[code_size].var1) = *var1;
    } else {
        code[code_size].var1 = NULL;
    }

    if (var2) {
        code[code_size].var2 = (INFO*)malloc(sizeof(INFO));
        *(code[code_size].var2) = *var2;
    } else {
        code[code_size].var2 = NULL;
    }

    if (reg) {
        code[code_size].reg = (INFO*)malloc(sizeof(INFO));
        *(code[code_size].reg) = *reg;
    } else {
        code[code_size].reg = NULL;
    }

    code_size++;
}

/* Function that generates code for leaf nodes
 */
static void gen_code_leaf(AST_NODE* node, INFO* result) {
    char buf[32];
    INFO temp_info;
    temp_info.type = TABLE_ID;

    switch (node->info->leaf.type) {
        case TYPE_INT: {
            sprintf(buf, "%d", node->info->leaf.value->int_value);
            temp_info.id.name = my_strdup(buf);
            temp_info.id.type = TYPE_INT;
            emit(I_LOADVAL, &temp_info, NULL, NULL);
            if (result) *result = temp_info;
            break;
        }
        case TYPE_BOOL: {
            sprintf(buf, "%d", node->info->leaf.value->bool_value);
            temp_info.id.name = my_strdup(buf);
            temp_info.id.type = TYPE_BOOL;
            emit(I_LOADVAL, &temp_info, NULL, NULL);
            if (result) *result = temp_info;
            break;
        }
        case TYPE_ID: {
            ID_TABLE* sym = node->info->leaf.value->id_leaf;
            if (sym) {
                temp_info.id.name = sym->info->id.name;
                temp_info.id.type = sym->info->id.type;
                emit(I_LOAD, &temp_info, NULL, NULL);
                if (result) *result = temp_info;
            }
            break;
        }
    }
}

/* Function that generates code for common expressions
 */
static void gen_code_common(AST_NODE* node, INFO* result) {
    INFO left_info, right_info, temp_info;
    INFO* left = &left_info;
    INFO* right = &right_info;
    INFO* temp = &temp_info;

    // Initialize all INFO structs as TABLE_ID type
    left->type = TABLE_ID;
    right->type = TABLE_ID;
    temp->type = TABLE_ID;

    switch (node->info->common.op) {
        case OP_ADDITION:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_ADD, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_SUBTRACTION:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_SUB, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_MULTIPLICATION:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_MUL, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_DIVISION:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_DIV, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_MOD:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_MOD, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_MINUS:
            gen_code(node->info->common.left, left);
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_MIN, left, NULL, temp);
            if (result) *result = *temp;
            break;

        case OP_RETURN:
            if (node->info->common.left) {
                gen_code(node->info->common.left, left);
                emit(I_RET, left, NULL, NULL);
            } else {
                emit(I_RET, NULL, NULL, NULL);
            }
            break;

        case OP_LES:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_LES, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_GRT:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_GRT, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_EQ:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_EQ, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_NEQ:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_NEQ, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_LEQ:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_LEQ, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_GEQ:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_GEQ, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_AND:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_AND, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_OR:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_OR, left, right, temp);
            if (result) *result = *temp;
            break;

        case OP_NEG:
            gen_code(node->info->common.left, left);
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_NEG, left, NULL, temp);
            if (result) *result = *temp;
            break;

        case OP_ASSIGN:
            gen_code(node->info->common.left, left);
            gen_code(node->info->common.right, right);
            emit(I_STORE, right, NULL, left);
            break;

        case OP_DECL:
            gen_code(node->info->common.left, left);
            if (node->info->common.right) {
                gen_code(node->info->common.right, right);
                emit(I_STORE, right, NULL, left);
            }
            break;

        default:
            break;
    }
}

/* Function that generates code for if expressions
 */
static void gen_code_if(AST_NODE* node, INFO* result) {
    INFO cond_info, else_label_info, end_label_info;

    cond_info.type = TABLE_ID;
    else_label_info.type = TABLE_ID;
    end_label_info.type = TABLE_ID;

    gen_code(node->info->if_stmt.condition, &cond_info);

    else_label_info.id.name = new_label();
    end_label_info.id.name = new_label();

    emit(I_JMPF, &cond_info, NULL, &else_label_info);
    gen_code(node->info->if_stmt.then_block, NULL);
    emit(I_JMP, &end_label_info, NULL, NULL);
    emit(I_LABEL, &else_label_info, NULL, NULL);
    if (node->info->if_stmt.else_block) {
        gen_code(node->info->if_stmt.else_block, NULL);
    }
    emit(I_LABEL, &end_label_info, NULL, NULL);
}

/* Function that generates code for while expressions
 */
static void gen_code_while(AST_NODE* node, INFO* result) {
    INFO start_label_info, end_label_info, cond_info;

    start_label_info.type = TABLE_ID;
    end_label_info.type = TABLE_ID;
    cond_info.type = TABLE_ID;

    start_label_info.id.name = new_label();
    end_label_info.id.name = new_label();

    emit(I_LABEL, &start_label_info, NULL, NULL);
    gen_code(node->info->while_stmt.condition, &cond_info);
    emit(I_JMPF, &cond_info, NULL, &end_label_info);
    gen_code(node->info->while_stmt.block, NULL);
    emit(I_JMP, &start_label_info, NULL, NULL);
    emit(I_LABEL, &end_label_info, NULL, NULL);
}

/* Function that generates code for method declarations
 */
static void gen_code_method_decl(AST_NODE* node, INFO* result) {
    INFO name_info;
    name_info.type = TABLE_ID;
    name_info.id.name = node->info->method_decl.name;

    if (node->info->method_decl.is_extern) {
        emit(I_EXTERN, &name_info, NULL, NULL);
        return;
    }
    emit(I_ENTER, &name_info, NULL, NULL);
    if (!node->info->method_decl.is_extern && node->info->method_decl.block) {
        gen_code(node->info->method_decl.block, NULL);
    }
    emit(I_LEAVE, &name_info, NULL, NULL);
}

/* Function that generates code for method calls
 */
static void gen_code_method_call(AST_NODE* node, INFO* result) {
    AST_NODE_LIST* arg = node->info->method_call.args;
    while (arg) {
        INFO arg_info;
        arg_info.type = TABLE_ID;
        gen_code(arg->first, &arg_info);
        emit(I_PARAM, &arg_info, NULL, NULL);
        arg = arg->next;
    }

    INFO name_info, ret_info;
    name_info.type = TABLE_ID;
    ret_info.type = TABLE_ID;
    name_info.id.name = node->info->method_call.name;
    ret_info.id.name = new_temp();
    emit(I_CALL, &name_info, NULL, &ret_info);
    if (result) {
        *result = ret_info;
    }
}

/* Function that generates code for blocks
 */
static void gen_code_block(AST_NODE* node, INFO* result) {
    AST_NODE_LIST* cur = node->info->block.stmts;
    INFO last_info;
    last_info.type = TABLE_ID;
    int has_last = 0;

    while (cur) {
        INFO stmt_info;
        stmt_info.type = TABLE_ID;
        gen_code(cur->first, &stmt_info);
        last_info = stmt_info;
        has_last = 1;
        cur = cur->next;
    }

    if (result && has_last) {
        *result = last_info;
    }
}

void print_code_to_file(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Can't open the file provided");
        return;
    }

    for (int i = 0; i < code_size; i++) {
        INFO* v1 = code[i].var1;
        INFO* v2 = code[i].var2;
        INFO* reg = code[i].reg;

        switch (code[i].instruct->instruct.type_instruct) {
            case I_LOADVAL:
                if (v1 && v1->id.name)
                    fprintf(f, "LOADVAL %s\n", v1->id.name);
                break;
            case I_LOAD:
                if (v1 && v1->id.name)
                    fprintf(f, "LOAD %s\n", v1->id.name);
                break;
            case I_STORE:
                if (v1 && v1->id.name && reg && reg->id.name)
                    fprintf(f, "STORE %s, %s\n", v1->id.name, reg->id.name);
                break;
            case I_ADD:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "ADD %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_SUB:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "SUB %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_MUL:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "MUL %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_DIV:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "DIV %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_MOD:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "MOD %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_MIN:
                if (v1 && v1->id.name && reg && reg->id.name)
                    fprintf(f, "MIN %s, %s\n", v1->id.name, reg->id.name);
                break;
            case I_RET:
                if (v1 && v1->id.name) {
                    fprintf(f, "RET %s\n", v1->id.name);
                } else {
                    fprintf(f, "RET\n");
                }
                break;
            case I_LES:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "LES %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_GRT:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "GRT %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_EQ:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "EQ %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_NEQ:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "NEQ %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_LEQ:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "LEQ %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_GEQ:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "GEQ %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_AND:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "AND %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_OR:
                if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                    fprintf(f, "OR %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
                break;
            case I_NEG:
                if (v1 && v1->id.name && reg && reg->id.name)
                    fprintf(f, "NEG %s, %s\n", v1->id.name, reg->id.name);
                break;
            case I_LABEL:
                if (v1 && v1->id.name)
                    fprintf(f, "%s:\n", v1->id.name);
                break;
            case I_JMP:
                if (v1 && v1->id.name)
                    fprintf(f, "JMP %s\n", v1->id.name);
                break;
            case I_JMPF:
                if (v1 && v1->id.name && reg && reg->id.name)
                    fprintf(f, "JMPF %s, %s\n", v1->id.name, reg->id.name);
                break;
            case I_PARAM:
                if (v1 && v1->id.name)
                    fprintf(f, "PARAM %s\n", v1->id.name);
                break;
            case I_CALL:
                if (v1 && v1->id.name && reg && reg->id.name)
                    fprintf(f, "CALL %s, %s\n", v1->id.name, reg->id.name);
                break;
            case I_ENTER:
                if (v1 && v1->id.name)
                    fprintf(f, "ENTER %s\n", v1->id.name);
                break;
            case I_LEAVE:
                if (v1 && v1->id.name)
                    fprintf(f, "LEAVE %s\n", v1->id.name);
                break;
            case I_EXTERN:
                if (v1 && v1->id.name)
                    fprintf(f, "EXTERN %s\n", v1->id.name);
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
void gen_code(AST_NODE* node, INFO* result) {
    if (!node) return;
    switch (node->info->type) {
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
    // Free allocated memory before resetting
    for (int i = 0; i < code_size; i++) {
        if (code[i].var1) free(code[i].var1);
        if (code[i].var2) free(code[i].var2);
        if (code[i].reg) free(code[i].reg);
    }

    code_size = 0;
    temp_counter = 0;
    label_counter = 0;
}

void cleanup_code() {
    for (int i = 0; i < code_size; i++) {
        if (code[i].var1) {
            free(code[i].var1);
            code[i].var1 = NULL;
        }
        if (code[i].var2) {
            free(code[i].var2);
            code[i].var2 = NULL;
        }
        if (code[i].reg) {
            free(code[i].reg);
            code[i].reg = NULL;
        }
        if (code[i].instruct) {
            free(code[i].instruct);
            code[i].instruct = NULL;
        }
    }
}