#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symbol_table.h"
#include <memory.h>
#include "error_handling.h"
#include <stdio.h>

typedef enum {
	INT_TYPE,
	BOOL_TYPE,
	VOID_TYPE,
	NULL_TYPE,
} RET_TYPE;

/* Public function: checks the semantic of a tree */
void semantic_analyzer(AST_ROOT* tree);

#endif