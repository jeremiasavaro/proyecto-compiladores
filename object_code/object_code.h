#ifndef OBJECT_CODE_H
#define OBJECT_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "intermediate_code.h"
#include "symbol.h"
#include "ast.h"

// Structure for saving variable name and offset
typedef struct {
    char* name;
    int offset;
} VarLocation;

void generate_object_code(FILE* out_file, cant_ap_temp* cant_ap_h);

#endif