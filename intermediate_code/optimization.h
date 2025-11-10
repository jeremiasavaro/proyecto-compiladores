#ifndef PROYECTO_COMPILADORES_OPTIMIZATION_H
#define PROYECTO_COMPILADORES_OPTIMIZATION_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"
#include "symbol.h"
#include <string.h>
#include "intermediate_code.h"

/* Functions that optimizes memory by reutilizing temporals
 */
void optimize_memory(CANT_AP_TEMP* tmp_list);

#endif