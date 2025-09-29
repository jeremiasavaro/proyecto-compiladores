#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symbol_table.h"

typedef enum {
	INT_TYPE,
	BOOL_TYPE,
} TYPE;

extern int alreadyReturned;

void semantic_analyzer(AST_ROOT* tree);
void eval(AST_NODE *tree, TYPE *ret);

#endif