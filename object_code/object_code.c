#include "object_code.h"
#include <string.h>
#include <stdlib.h>

// Acces to the buffer of intermediate code
extern Instr code[];
extern int code_size;

static TempMap temp_map[100];
static int temp_count = 0;
static int current_stack_offset = -8;  // Start before of %rbp


// Obtains the stack offset for a given temporary variable.

static int get_temp_offset(const char* temp_name) {
    // Search if it already exists
    for (int i = 0; i < temp_count; i++) {
        if (strcmp(temp_map[i].temp_name, temp_name) == 0) {
            return temp_map[i].stack_offset;
        }
    }

    // Create new mapping
    temp_map[temp_count].temp_name = strdup(temp_name);
    temp_map[temp_count].stack_offset = current_stack_offset;
    temp_count++;
    current_stack_offset -= 8;  // Each temporary occupies 8 bytes (64 bits)

    return current_stack_offset + 8;
}

/*
 * Returns the assembly location of a variable or temporary.
 * Examples:
 *   "5"     -> "$5"          (immediate value)
 *   "T0"    -> "-8(%rbp)"    (stack temporary)
 *   "var"   -> "var(%rip)"   (global variable)
 */
static char* get_location(INFO* info) {
    static char buffer[64];

    if (!info) return NULL;

    // If it's an immediate value (number)
    if (info->id.name[0] >= '0' && info->id.name[0] <= '9') {
        snprintf(buffer, sizeof(buffer), "$%s", info->id.name);
        return buffer;
    }

    // If it's a temporary (starts with 'T')
    if (info->id.name[0] == 'T') {
        int offset = get_temp_offset(info->id.name);
        snprintf(buffer, sizeof(buffer), "%d(%%rbp)", offset);
        return buffer;
    }

    // If it's a variable (assume it's in memory)
    snprintf(buffer, sizeof(buffer), "%s(%%rip)", info->id.name);
    return buffer;
}

/*
 * Translates a LOADVAL instruction to assembly.
  * I_LOADVAL: reg = var1 (loads an immediate value into a temporary)
  * Example: T0 = 5  ->  mov $5, -8(%rbp)  # assuming T0 is at -8(%rbp)
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

    // Load first operand into %rax
    fprintf(f, "    movq %s, %%rax\n", src1);

    // Add second operand to %rax
    fprintf(f, "    addq %s, %%rax\n", src2);

    // Store result in destination
    fprintf(f, "    movq %%rax, %s\n\n", dst);
}

/*
 * Main function that generates x86-64 assembly code from intermediate code.
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

    // Write the assembly prologue
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

    // Epílogue of the main function
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