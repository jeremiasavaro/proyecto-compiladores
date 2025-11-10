#ifndef OBJECT_CODE_H
#define OBJECT_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "intermediate_code.h"
#include "symbol.h"
#include "ast.h"

#define MAX_VARS_PER_FUNCTION 200

// Structure for saving variable name and offset
typedef struct {
    char* name;
    int offset;
} VarLocation;

/* Main function to generate x86-64 assembly code from intermediate code
 * Outputs the assembly code to the provided file pointer
 * Handles function prologues/epilogues, arithmetic operations, control flow, and function calls
 * Uses a simple mapping of temporaries to stack offsets
 */
void generate_object_code(FILE* out_file, CANT_AP_TEMP* cant_ap_h);

#endif