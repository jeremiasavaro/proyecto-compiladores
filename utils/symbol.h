#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>

typedef struct TABLE_STACK TABLE_STACK;
typedef struct ID_TABLE ID_TABLE;
typedef struct ARGS ARGS;
typedef struct ARGS_LIST ARGS_LIST;
typedef struct AST_NODE_LIST AST_NODE_LIST;
typedef struct AST_NODE AST_NODE;
typedef struct AST_ROOT AST_ROOT;

typedef enum {
	UNARY,
	BINARY
} OPERATOR_ARITY;

typedef enum {
	OP_ADDITION,
	OP_SUBTRACTION,
	OP_MULTIPLICATION,
	OP_DIVISION,
	OP_MOD,
	OP_LES,
	OP_GRT,
	OP_EQ,
	OP_NEQ,
	OP_LEQ,
	OP_GEQ,
	OP_MINUS,
	OP_AND,
	OP_OR,
	OP_NEG,
	OP_DECL,
	OP_ASSIGN,
	OP_RETURN,
} OPERATOR;

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
	I_SHIFT_RIGHT // Shift right operation for optimizations
} INSTR_TYPE;

typedef enum {
	AST_COMMON,
	AST_IF,
	AST_WHILE,
	AST_METHOD_DECL,
	AST_METHOD_CALL,
	AST_BLOCK,
	AST_LEAF,
	AST_NULL,
	TABLE_ID
} INFO_TYPE;

typedef enum {
	RETURN_INT,
	RETURN_BOOL,
	RETURN_VOID,
	RETURN_NULL
} RETURN_TYPE;

typedef enum {
	TYPE_INT,
	TYPE_BOOL,
	TYPE_ID
} TYPE;

struct ARGS {
	char* name;
	TYPE type;
};

struct ARGS_LIST {
	ARGS* arg;
	ARGS_LIST* next;
};

typedef union {
	int int_value;
	int bool_value; // 0 for false, 1 for true
	ID_TABLE* id_leaf; // Pointer to the symbol table entry for identifiers
} LEAF_UNION;

typedef struct INFO {
	INFO_TYPE type;
	union {

		struct {
			OPERATOR_ARITY arity;
			OPERATOR op;
			AST_NODE* left;
			AST_NODE* right;
		} common;

		struct {
			AST_NODE* condition;
			AST_NODE* then_block;
			AST_NODE* else_block;
		} if_stmt;

		struct {
			AST_NODE* condition;
			AST_NODE* block;
		} while_stmt;

		struct {
			char* name;
			int num_args; // Amount of arguments.
			RETURN_TYPE return_type;
			ARGS_LIST* args; // Arguments list.
			AST_NODE* block; // Method body.
			TABLE_STACK* scope; // Scope of the method.
			int is_extern; // Flag to check if the method is externally defined.
		} method_decl;

		struct {
			char* name;
			int num_args; // Amount of arguments.
			AST_NODE_LIST* args; // Arguments list.
		} method_call;

		struct {
			AST_NODE_LIST* stmts; // Statements list.
		} block;

		struct {
			TYPE type;
			LEAF_UNION* value;
		} leaf;

		struct {
			char* name;
			TYPE type;
			int temp;
		} id;

		struct {
			INSTR_TYPE type_instruct;
		} instruct;
	};
} INFO;

ARGS_LIST* allocate_args_list_mem();
ARGS* allocate_args_mem();
INFO* allocate_info_mem();

#endif
