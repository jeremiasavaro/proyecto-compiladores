#include "intermediate_code.h"

// Buffer for save all the instructions (pseudo-assembly)
Instr code[1000];   // 1000 instructions for example
int code_size = 0;  // Number of instructions saved

static cant_ap_temp* cant_ap_h;
static int temp_counter = 0;
static int label_counter = 0;

extern int optimizations;
extern int debug;

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

/* Function to update the count of appearances of a temporary variable
 */
void increase_temp_ap(char* temp) {
    cant_ap_temp* current = cant_ap_h;
    while (current) {
        if (strcmp(current->temp, temp) == 0) {
            current->cant_ap += 1;
            return;
        }
        current = current->next;
    }
    // If not found, create a new entry
    cant_ap_temp* new_entry = (cant_ap_temp*)malloc(sizeof(cant_ap_temp));
    new_entry->cant_ap = 1;
    new_entry->temp = my_strdup(temp);
    new_entry->next = cant_ap_h;
    cant_ap_h = new_entry;
}

/* Function for save instructions in the buffer
 */
void emit(INSTR_TYPE t, INFO* var1, INFO* var2, INFO* reg) {
    // reserve space for a new instruction
    code[code_size].instruct = (INFO*)malloc(sizeof(INFO));
    code[code_size].instruct->type = TABLE_ID;
    code[code_size].instruct->instruct.type_instruct = t;

    if (var1) {
        if (var1->id.name[0] == 'T'){
            increase_temp_ap(var1->id.name);
        }
        code[code_size].var1 = (INFO*)malloc(sizeof(INFO));
        *(code[code_size].var1) = *var1;
    } else {
        code[code_size].var1 = NULL;
    }

    if (var2) {
        if (var2->id.name[0] == 'T'){
            increase_temp_ap(var2->id.name);
        }
        code[code_size].var2 = (INFO*)malloc(sizeof(INFO));
        *(code[code_size].var2) = *var2;
    } else {
        code[code_size].var2 = NULL;
    }

    if (reg) {
        if (reg->id.name[0] == 'T'){
            increase_temp_ap(reg->id.name);
        }
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
    INFO aux, temp_info;
    INFO* temp = &temp_info;
    aux.type = TABLE_ID;
    temp->type = TABLE_ID;

    switch (node->info->leaf.type) {
        case TYPE_INT: {
            sprintf(buf, "%d", node->info->leaf.value->int_value);
            aux.id.name = my_strdup(buf);
            aux.id.type = TYPE_INT;
            temp->id.name = new_temp();
            temp->id.type = TYPE_INT;
            emit(I_LOADVAL, &aux, NULL, temp);
            if (result) *result = *temp;
            break;
        }
        case TYPE_BOOL: {
            sprintf(buf, "%d", node->info->leaf.value->bool_value);
            aux.id.name = my_strdup(buf);
            aux.id.type = TYPE_BOOL;
            temp->id.name = new_temp();
            temp->id.type = TYPE_BOOL;
            emit(I_LOADVAL, &aux, NULL, temp);
            if (result) *result = *temp;
            break;
        }
        case TYPE_ID: {
            ID_TABLE* sym = node->info->leaf.value->id_leaf;
            if (sym) {
                aux.id.name = sym->info->id.name;
                aux.id.type = sym->info->id.type;
                if (node->father->info->common.op != OP_ASSIGN && node->father->info->common.op != OP_DECL) {
                    emit(I_LOAD, &aux, NULL, NULL);
                }
                if (result) *result = aux;
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
            AST_NODE* right_child = node->info->common.right;
            if (right_child && right_child->info->type == AST_LEAF && optimizations) {
                int right_value = right_child->info->leaf.value->int_value;
                // Check if right_value is a power of 2 using bits operations
                if (right_value > 0 && (right_value & (right_value - 1)) == 0) {
                    right = (INFO*) malloc(sizeof(INFO));
                    char buf[32];
                    sprintf(buf, "%d", __builtin_ctz(right_value));
                    right->id.name = my_strdup(buf);
                    right->id.type = TYPE_INT;
                    temp->id.name = new_temp();
                    temp->id.type = TYPE_INT;
                    emit(I_SHIFT_RIGHT, left, right, temp);
                    if (result) *result = *temp;
                    break;
                }
            }
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
    INFO args_info_list[node->info->method_call.num_args];
    int i = 0;
    while (arg) {
        INFO arg_info;
        arg_info.type = TABLE_ID;
        gen_code(arg->first, &arg_info);
        args_info_list[i] = arg_info;
        arg = arg->next;
        i++;
    }
    i = 0;
    while (i < node->info->method_call.num_args) {
        emit(I_PARAM, &args_info_list[i], NULL, NULL);
        i++;
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
    INFO last_info; // store info of the last statement
    last_info.type = TABLE_ID;
    int has_last = 0; // flag to check if minimum one statement was processed
    //int returned = 0; // flag to check if a return statement was encountered
    while (cur) {
        INFO stmt_info;
        stmt_info.type = TABLE_ID; // initialize stmt_info
        gen_code(cur->first, &stmt_info);
        if (cur->first->info->type == AST_COMMON && cur->first->info->common.op == OP_RETURN && optimizations) {
            cur = NULL;
        } else {
            cur = cur->next;
        }
        last_info = stmt_info;
        has_last = 1;
    }

    if (result && has_last) {
        *result = last_info;
    }
}

cant_ap_temp* print_code_to_file(const char* filename) {

    if (debug) {
        FILE* f = fopen(filename, "w");
        if (!f) {
            perror("Can't open the file provided");
            return NULL;
        }

        for (int i = 0; i < code_size; i++) {
            INFO* v1 = code[i].var1;
            INFO* v2 = code[i].var2;
            INFO* reg = code[i].reg;

            switch (code[i].instruct->instruct.type_instruct) {
                case I_LOADVAL:
                    if (v1 && v1->id.name && reg && reg->id.name)
                        fprintf(f, "LOADVAL %s, %s\n", v1->id.name, reg->id.name);
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
                case I_SHIFT_RIGHT:
                    if (v1 && v1->id.name && v2 && v2->id.name && reg && reg->id.name)
                        fprintf(f, "SHIFT_RIGHT %s, %s, %s\n", v1->id.name, v2->id.name, reg->id.name);
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
        return cant_ap_h;
    }
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

Instr* get_intermediate_code() {
    return code;
}

int get_code_size() {
    return code_size;
}