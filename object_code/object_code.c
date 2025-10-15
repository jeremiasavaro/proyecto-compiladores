#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "object_code.h"
#include "intermediate_code.h"
#include "symbol.h"
#include "ast.h" // <<<--- AÑADIDO: Necesario para acceder a la lista de parámetros

// --- Declaraciones de funciones externas ---
extern Instr* get_intermediate_code();
extern int get_code_size();
extern AST_ROOT* head_ast; // <<<--- AÑADIDO: Acceso global al AST

#define MAX_VARS_PER_FUNCTION 200

typedef struct {
    char* name;
    int offset;
} VarLocation;

static VarLocation var_map[MAX_VARS_PER_FUNCTION];
static int var_count = 0;
static int current_stack_offset = 0;
const char* arg_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

/*
 * Busca o crea una ubicación en la pila para una variable.
 */
static int get_var_offset(const char* name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(var_map[i].name, name) == 0) {
            return var_map[i].offset;
        }
    }
    current_stack_offset -= 8;
    var_map[var_count].name = strdup(name);
    var_map[var_count].offset = current_stack_offset;
    var_count++;
    return current_stack_offset;
}

/*
 * Genera la cadena del operando en ensamblador.
 */
static void get_operand_str(INFO* var, char* buf, size_t buf_size) {
    if (var == NULL || var->id.name == NULL) {
        buf[0] = '\0';
        return;
    }
    char* name = var->id.name;

    if (name[0] == 'L') {
        snprintf(buf, buf_size, "%s", name);
    } else if (isdigit(name[0]) || (name[0] == '-' && isdigit(name[1]))) {
        snprintf(buf, buf_size, "$%s", name);
    } else {
        snprintf(buf, buf_size, "%d(%%rbp)", get_var_offset(name));
    }
}


/*
 * Genera el código objeto (ensamblador x86-64).
 */
void generate_object_code(FILE* out_file) {
    Instr* code = get_intermediate_code();
    int code_size = get_code_size();
    int param_count = 0;

    fprintf(out_file, ".text\n");

    for (int i = 0; i < code_size; ++i) {
        Instr* instr = &code[i];
        char op1[64], op2[64], dest[64];

        switch (instr->instruct->instruct.type_instruct) {
            case I_EXTERN:
                fprintf(out_file, ".globl %s\n", instr->var1->id.name);
                break;

            case I_ENTER: {
                const char* func_name = instr->var1->id.name;

                // 1. Encontrar el nodo AST de la función para obtener sus parámetros
                AST_NODE* func_node = NULL;
                for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
                    if (cur->sentence->info->type == AST_METHOD_DECL &&
                        strcmp(cur->sentence->info->method_decl.name, func_name) == 0) {
                        func_node = cur->sentence;
                        break;
                    }
                }

                // 2. Primer paso: Calcular el tamaño de la pila
                var_count = 0;
                current_stack_offset = 0;
                int end_func_idx = i;
                for (int j = i + 1; j < code_size; ++j) {
                    if (code[j].instruct->instruct.type_instruct == I_LEAVE &&
                        strcmp(code[j].var1->id.name, func_name) == 0) {
                        end_func_idx = j;
                        break;
                    }
                }
                for (int k = i; k <= end_func_idx; ++k) {
                    if (code[k].var1 && code[k].var1->id.name && !isdigit(code[k].var1->id.name[0]) && code[k].var1->id.name[0] != 'L') get_var_offset(code[k].var1->id.name);
                    if (code[k].var2 && code[k].var2->id.name && !isdigit(code[k].var2->id.name[0]) && code[k].var2->id.name[0] != 'L') get_var_offset(code[k].var2->id.name);
                    if (code[k].reg  && code[k].reg->id.name  && !isdigit(code[k].reg->id.name[0])  && code[k].reg->id.name[0] != 'L') get_var_offset(code[k].reg->id.name);
                }
                int total_stack_size = -current_stack_offset;
                if (total_stack_size % 16 != 0) {
                    total_stack_size += 16 - (total_stack_size % 16);
                }

                // 3. Resetear para la generación de código
                var_count = 0;
                current_stack_offset = 0;

                // 4. Generar el prólogo
                fprintf(out_file, "\n.globl %s\n", func_name);
                fprintf(out_file, "%s:\n", func_name);
                fprintf(out_file, "  pushq %%rbp\n");
                fprintf(out_file, "  movq %%rsp, %%rbp\n");
                if (total_stack_size > 0) {
                    fprintf(out_file, "  subq $%d, %%rsp\n", total_stack_size);
                }

                // 5. CORRECCIÓN: Guardar los parámetros de los registros a la pila
                if (func_node) {
                    ARGS_LIST* arg_list = func_node->info->method_decl.args;
                    int arg_idx = 0;
                    while (arg_list && arg_idx < 6) {
                        const char* arg_name = arg_list->arg->name;
                        int offset = get_var_offset(arg_name); // Asignar espacio en la pila
                        fprintf(out_file, "  movq %s, %d(%%rbp)\n", arg_regs[arg_idx], offset);
                        arg_list = arg_list->next;
                        arg_idx++;
                    }
                }
                break;
            }
            case I_LEAVE: {
                fprintf(out_file, ".L_leave_%s:\n", instr->var1->id.name);
                fprintf(out_file, "  movq %%rbp, %%rsp\n");
                fprintf(out_file, "  popq %%rbp\n");
                fprintf(out_file, "  ret\n");
                for (int v = 0; v < var_count; ++v) {
                    free(var_map[v].name);
                    var_map[v].name = NULL;
                }
                var_count = 0;
                break;
            }
            case I_RET: {
                char current_func_name[64] = "unknown";
                for(int j=i; j>=0; j--) {
                    if (code[j].instruct->instruct.type_instruct == I_ENTER) {
                         strncpy(current_func_name, code[j].var1->id.name, sizeof(current_func_name)-1);
                         current_func_name[sizeof(current_func_name)-1] = '\0';
                         break;
                    }
                }
                if(instr->var1) {
                    get_operand_str(instr->var1, op1, sizeof(op1));
                    fprintf(out_file, "  movq %s, %%rax\n", op1);
                }
                fprintf(out_file, "  jmp .L_leave_%s\n", current_func_name);
                break;
            }
            case I_LOADVAL:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %s\n", op1, dest);
                break;
            case I_STORE:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;
            case I_ADD: case I_SUB: case I_MUL: case I_AND: case I_OR:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->var2, op2, sizeof(op2));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                const char* op_str = (instr->instruct->instruct.type_instruct == I_ADD) ? "addq" :
                                     (instr->instruct->instruct.type_instruct == I_SUB) ? "subq" :
                                     (instr->instruct->instruct.type_instruct == I_MUL) ? "imulq" :
                                     (instr->instruct->instruct.type_instruct == I_AND) ? "andq" : "orq";
                fprintf(out_file, "  %s %s, %%rax\n", op_str, op2);
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;
            case I_DIV: case I_MOD:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->var2, op2, sizeof(op2));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  cqto\n");
                fprintf(out_file, "  idivq %s\n", op2);
                char* result_reg = (instr->instruct->instruct.type_instruct == I_DIV) ? "%rax" : "%rdx";
                fprintf(out_file, "  movq %s, %s\n", result_reg, dest);
                break;
            case I_MIN:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  negq %%rax\n");
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;
            case I_NEG:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  testq %%rax, %%rax\n");
                fprintf(out_file, "  sete %%al\n");
                fprintf(out_file, "  movzbq %%al, %%rax\n");
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;
            case I_LES: case I_GRT: case I_EQ: case I_NEQ: case I_LEQ: case I_GEQ: {
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->var2, op2, sizeof(op2));
                get_operand_str(instr->reg, dest, sizeof(dest));
                const char* set_op;
                switch (instr->instruct->instruct.type_instruct) {
                    case I_LES: set_op = "setl"; break; case I_GRT: set_op = "setg"; break;
                    case I_EQ:  set_op = "sete"; break; case I_NEQ: set_op = "setne"; break;
                    case I_LEQ: set_op = "setle"; break; case I_GEQ: set_op = "setge"; break;
                    default: set_op = ""; break;
                }
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  cmpq %s, %%rax\n", op2);
                fprintf(out_file, "  %s %%al\n", set_op);
                fprintf(out_file, "  movzbq %%al, %%rax\n");
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;
            }
            case I_LABEL:
                get_operand_str(instr->var1, op1, sizeof(op1));
                fprintf(out_file, "%s:\n", op1);
                break;
            case I_JMP:
                get_operand_str(instr->var1, op1, sizeof(op1));
                fprintf(out_file, "  jmp %s\n", op1);
                break;
            case I_JMPF:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  testq %%rax, %%rax\n");
                fprintf(out_file, "  jz %s\n", dest);
                break;
            case I_PARAM:
                if (param_count < 6) {
                    get_operand_str(instr->var1, op1, sizeof(op1));
                    fprintf(out_file, "  movq %s, %s\n", op1, arg_regs[param_count]);
                } else {
                    // Manejo de parámetros en pila no implementado
                }
                param_count++;
                break;
            case I_CALL:
                // CORRECCIÓN: Usar el nombre de la función directamente como una etiqueta.
                fprintf(out_file, "  call %s\n", instr->var1->id.name);
                if (instr->reg) {
                    get_operand_str(instr->reg, dest, sizeof(dest));
                    fprintf(out_file, "  movq %%rax, %s\n", dest);
                }
                param_count = 0;
                break;

            case I_LOAD: // Intencionalmente vacío
                break;
            default:
                fprintf(out_file, "  # Instrucción desconocida\n");
                break;
        }
    }
}