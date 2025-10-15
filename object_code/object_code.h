#ifndef OBJECT_CODE_H
#define OBJECT_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "intermediate_code.h"
#include "symbol.h"

void generate_object_code(FILE* out_file);

#endif