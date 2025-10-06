#ifndef INTERMEDIATE_CODE_GENERATOR_H
#define INTERMEDIATE_CODE_GENERATOR_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"
#include "symbol.h"

// Instruction types for the pseudo-assembly
typedef enum {
    I_LOAD,
    I_LOADVAL,
    I_STORE,
    I_ADD,
    I_SUB,
    I_MUL,
    I_DIV,
    I_MOD,
    I_MIN,
    I_LES,
    I_GRT,
    I_EQ,
    I_NEQ,
    I_LEQ,
    I_GEQ,
    I_AND,
    I_OR,
    I_NEG,
    I_RET,
    I_LABEL,     // Label pseudo instruction
    I_JMP,       // Unconditional jump
    I_JMPF,      // Jump if false (0)
    I_PARAM,     // Pass parameter (argument) before a call
    I_CALL,      // Call a method (var1 = method name, reg = temp for return if any)
    I_ENTER,     // Method prologue (var1 = method name)
    I_LEAVE,      // Method epilogue (var1 = method name)
    I_EXTERN,     // Extern method prologue 
} InstrType;

// Instruction representation for the pseudo-assembly
typedef struct {
    InstrType type;
    char var1[32]; // Buffer size (variable name or value)
    char var2[32]; // Buffer size (variable name or value)
    char reg[32]; // Buffer size (variable name or value)
} Instr;

void emit(InstrType t, const char* var1, const char* var2, const char* reg);
void gen_code(AST_NODE* node, char** result);
void print_code_to_file(const char* filename);
void reset_code();

#endif