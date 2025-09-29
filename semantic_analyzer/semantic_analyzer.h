#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symbol_table.h"
#include <memory.h>

typedef enum {
	INT_TYPE,
	BOOL_TYPE,
	VOID_TYPE
} TYPE;

void semantic_analyzer(AST_ROOT* tree);
void eval(AST_NODE *tree, TYPE *ret);

#endif