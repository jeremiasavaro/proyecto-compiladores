#ifndef INTERMEDIATE_CODE_GENERATOR_H
#define INTERMEDIATE_CODE_GENERATOR_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"
#include "symbol.h"

typedef struct cant_ap_temp cant_ap_temp;

// Instruction representation for the pseudo-assembly
typedef struct {
    INFO* instruct; // Instruction type
    INFO* var1; // Buffer size (variable name or value)
    INFO* var2; // Buffer size (variable name or value)
    INFO* reg; // Buffer size (variable name or value)
} Instr;

// Structure to keep track of temporary variable appearances
typedef struct cant_ap_temp {
    int cant_ap;
    char* temp;
    cant_ap_temp* next;
};

void emit(INSTR_TYPE t, INFO* var1, INFO* var2, INFO* reg);
void gen_code(AST_NODE* node, INFO* result);
cant_ap_temp* print_code_to_file(const char* filename);
void reset_code();
Instr* get_intermediate_code();
int get_code_size();

#endif