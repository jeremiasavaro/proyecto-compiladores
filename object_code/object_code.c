#include "object_code.h"
#include <string.h>
#include <stdlib.h>

/* Acceso al buffer de código intermedio (definido en intermediate_code.c) */
extern Instr code[];
extern int code_size;


/* Mapa simple de temporales a offsets en el stack */
typedef struct {
    char* temp_name;
    int stack_offset;
} TempMap;

static TempMap temp_map[100];
static int temp_count = 0;
static int current_stack_offset = -8;  // Empieza después de %rbp


/*
 * Obtiene o crea el offset en el stack para un temporal
 */
static int get_temp_offset(const char* temp_name) {
    // Buscar si ya existe
    for (int i = 0; i < temp_count; i++) {
        if (strcmp(temp_map[i].temp_name, temp_name) == 0) {
            return temp_map[i].stack_offset;
        }
    }

    // Crear nuevo mapeo
    temp_map[temp_count].temp_name = strdup(temp_name);
    temp_map[temp_count].stack_offset = current_stack_offset;
    temp_count++;
    current_stack_offset -= 8;  // Cada temporal ocupa 8 bytes (64 bits)

    return current_stack_offset + 8;
}

/*
 * Retorna la ubicación en assembly de una variable o temporal
 * Ejemplos:
 *   "5"     -> "$5"          (valor inmediato)
 *   "T0"    -> "-8(%rbp)"    (temporal en stack)
 *   "var"   -> "var(%rip)"   (variable global)
 */
static char* get_location(INFO* info) {
    static char buffer[64];

    if (!info) return NULL;

    // Si es un número (valor inmediato)
    if (info->id.name[0] >= '0' && info->id.name[0] <= '9') {
        snprintf(buffer, sizeof(buffer), "$%s", info->id.name);
        return buffer;
    }

    // Si es un temporal (empieza con 'T')
    if (info->id.name[0] == 'T') {
        int offset = get_temp_offset(info->id.name);
        snprintf(buffer, sizeof(buffer), "%d(%%rbp)", offset);
        return buffer;
    }

    // Si es una variable (asumir que está en memoria)
    snprintf(buffer, sizeof(buffer), "%s(%%rip)", info->id.name);
    return buffer;
}

/*
 * Traduce la instrucción I_LOADVAL a assembly x86-64
 * I_LOADVAL: reg = var1 (carga un valor inmediato en un temporal)
 * Ejemplo: T0 = 5  ->  mov $5, -8(%rbp)  # asumiendo T0 está en -8(%rbp)
 */
void translate_loadval(FILE* f, INFO* var1, INFO* reg) {
    if (!var1 || !reg) return;

    char* src = get_location(var1);
    char* dst = get_location(reg);

    fprintf(f, "    movq %s, %%rax\n", src);
    fprintf(f, "    movq %%rax, %s\n\n", dst);
}

void translate_add(FILE* f, INFO* var1, INFO* var2, INFO* reg) {
    if (!var1 || !var2 || !reg) return;

    char* src1 = get_location(var1);
    char* src2 = get_location(var2);
    char* dst = get_location(reg);

    // Cargar primer operando en %rax
    fprintf(f, "    movq %s, %%rax\n", src1);

    // Sumar segundo operando a %rax
    fprintf(f, "    addq %s, %%rax\n", src2);

    // Guardar resultado en destino
    fprintf(f, "    movq %%rax, %s\n\n", dst);
}

/*
 * Función principal que recorre el código intermedio y lo traduce a assembly
 */
void generate_assembly(const char* output_filename) {
    FILE* f = fopen(output_filename, "w");
    if (!f) {
        perror("Cant open output file");
        return;
    }

    // Reset temp mapping
    temp_count = 0;
    current_stack_offset = -8;

    // Escribir el preámbulo del archivo assembly
    fprintf(f, "# Assembly code x86-64 generated\n");
    fprintf(f, "# Generated automatically by the compiler\n\n");
    fprintf(f, ".section .text\n");
    fprintf(f, ".globl main\n\n");
    fprintf(f, "main:\n");
    fprintf(f, "    pushq %%rbp\n");
    fprintf(f, "    movq %%rsp, %%rbp\n");
    fprintf(f, "    subq $256, %%rsp    # Reserve space for locals and temps\n\n");

    for (int i = 0; i < code_size; i++) {
        Instr* instr = &code[i];

        if (!instr->instruct) continue;

        INSTR_TYPE type = instr->instruct->instruct.type_instruct;

        // Comment that indicates the intermediate instruction
        fprintf(f, "    # Instrucción %d: ", i);

        switch (type) {
            case I_LOADVAL:
                fprintf(f, "LOADVAL %s -> %s\n",
                        instr->var1 ? instr->var1->id.name : "?",
                        instr->reg ? instr->reg->id.name : "?");
                translate_loadval(f, instr->var1, instr->reg);
                break;

            case I_ADD:
                fprintf(f, "ADD %s + %s -> %s\n",
                        instr->var1 ? instr->var1->id.name : "?",
                        instr->var2 ? instr->var2->id.name : "?",
                        instr->reg ? instr->reg->id.name : "?");
                translate_add(f, instr->var1, instr->var2, instr->reg);
                break;

            case I_SUB:
                fprintf(f, "SUB\n");
                break;

            case I_MUL:
                fprintf(f, "MUL\n");
                break;

            case I_DIV:
                fprintf(f, "DIV\n");
                break;

            case I_STORE:
                fprintf(f, "STORE\n");
               break;

            case I_LOAD:
                fprintf(f, "LOAD\n");
                break;

            case I_LABEL:
                if (instr->var1) {
                    fprintf(f, "LABEL\n");
                    fprintf(f, "%s:\n\n", instr->var1->id.name);
                }
                break;

            case I_JMP:
                fprintf(f, "JMP\n");
                break;

            case I_JMPF:
                fprintf(f, "JMPF\n");
                break;

            case I_CALL:
                fprintf(f, "CALL\n");
                break;

            case I_RET:
                fprintf(f, "RET\n");
                break;

            default:
                fprintf(f, "UNKNOWN\n");
                break;
        }
    }

    // Epílogo de la función main
    fprintf(f, "\n    # Epilogue\n");
    fprintf(f, "    movq $0, %%rax\n");
    fprintf(f, "    leave\n");
    fprintf(f, "    ret\n");

    fclose(f);

    // Cleanup temp mapping
    for (int i = 0; i < temp_count; i++) {
        free(temp_map[i].temp_name);
    }

    printf("Assembly code generated in: %s\n", output_filename);
}