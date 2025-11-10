#ifndef INTERMEDIATE_CODE_GENERATOR_H
#define INTERMEDIATE_CODE_GENERATOR_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"
#include "symbol.h"

#define MAX_CODE_SIZE 1000

typedef struct TEMP_LIST TEMP_LIST;
typedef struct CANT_AP_TEMP CANT_AP_TEMP;

extern CANT_AP_TEMP* cant_ap_h;

// Instruction representation for the pseudo-assembly
typedef struct {
    INFO* instruct; // Instruction type
    INFO* var1; // Buffer size (variable name or value)
    INFO* var2; // Buffer size (variable name or value)
    INFO* reg; // Buffer size (variable name or value)
} Instr;

// Structure for saving location of all instances of a temporal
typedef struct TEMP_LIST {
    char* location;
    TEMP_LIST* next;
} TEMP_LIST;

// Structure to keep track of temporary variable appearances
typedef struct CANT_AP_TEMP {
    int cant_ap;
    char* temp;
    int locked;
    TEMP_LIST* list;
    CANT_AP_TEMP* next;
} CANT_AP_TEMP;

/* Function for save instructions in the buffer
 */
void emit(INSTR_TYPE t, INFO* var1, INFO* var2, INFO* reg);
/* Function that generates the pseudo-assembly
 */
void gen_code(AST_NODE* node, INFO* result);
/* Function that dumps intermediate code into file -> filename
 */
CANT_AP_TEMP* print_code_to_file(const char* filename);
/* Function that resets intermediate code structure
 */
void reset_code();
/* Function that returns intermediate code generated
 */
Instr* get_intermediate_code();
/* Function that returns code size
 */
int get_code_size();
/* Function that prints list of temporals used before optimization
 */
void print_temp_list(CANT_AP_TEMP* head);

#endif