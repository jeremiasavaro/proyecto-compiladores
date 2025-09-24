#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "symbol_table.h"

typedef enum {
	INT_TYPE,
	BOOL_TYPE,
} TYPE;

typedef struct {
    void *value;
    TYPE type;
} ReturnValueNode;

extern int alreadyReturned;

int interpreter(AST_ROOT* tree);

#endif