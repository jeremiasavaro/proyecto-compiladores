#ifndef ID_TABLE_H
#define ID_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "error_handling.h"
#include "utils.h"

typedef struct ID_TABLE ID_TABLE;
typedef struct TABLE_STACK TABLE_STACK;
typedef struct ARGS ARGS;
typedef struct ARGS_LIST ARGS_LIST;

// Pointer to the global level of table_stack.
extern TABLE_STACK* global_level;

typedef enum {
	UNKNOWN,
	CONST_INT,
	CONST_BOOL,
	METHOD
} ID_TYPE;

typedef enum {
	RETURN_INT,
	RETURN_BOOL,
	RETURN_VOID
} RETURN_TYPE;

struct TABLE_STACK {
	ID_TABLE* head_block;
	ID_TABLE* end_block;
	TABLE_STACK* up;
};

// Node type for ARGS_LIST.
struct ARGS {
	char* name;
	ID_TYPE type;
	void* data;
};

// List to save arguments of methods.
struct ARGS_LIST {
	ARGS* arg;
	ARGS_LIST* next;
};

// Node type for ID_TABLE (variable, constant or method).
struct ID_TABLE {
	char* id_name;
	ID_TYPE id_type;
	union {
		struct {
			void* data;
		} common;

		struct {
			RETURN_TYPE return_type;
			int num_args;
			ARGS_LIST* arg_list;
			TABLE_STACK* method_scope;
			void* data; // Store the return value of the method, if return_type is void this remains void as well.
		} method;
	};
	ID_TABLE* next;
};

/* Pushes a new scope in the stack.
 */
void push_scope();
/* Pop the actual scope.
 */
void pop_scope(void);
/* Creates a new node with id_name = name and returns its memory direction
 * and doesn't allow to create two symbols with the same id in the same scope level.
 */
ID_TABLE* add_id(char* name, ID_TYPE type);
/* Declare a method in the actual scope with its return value.
 */
ID_TABLE* add_method(char* name, RETURN_TYPE ret_type, TABLE_STACK* method_scope);
/* Adds data to the variable name node.
 */
void add_data(char* name, ID_TYPE type, const void* data);
/* Adds data to the method's return value.
 */
void add_method_return_data(char* name, RETURN_TYPE type, const void* data);
/* Returns the memory direction of the node with id_name = name.
 * If the node is not found, returns NULL.
 * First, it looks for the id in the current scope, if it doesn't find it,
 * it goes up one scope level and keeps searching.
 */
ID_TABLE* find(const char* name);
/* Returns the memory direction of the node with id_name = name in the actual scope
 * if the node is not found, returns NULL.
 */
ID_TABLE* find_in_current_scope(const char* name);
/* Returns the memory direction of the node with id_name = name in the global scope
 * if the node is not found, returns NULL.
 */
ID_TABLE* find_global(const char* name);
/* Retrieves data of id from table.
 */
void* get_data(char* name);
/* Adds an argument to a given method.
 */
void add_arg(char* method_name, ID_TYPE arg_type, const char* arg_name);
/* Creates the argument list of a given method.
 */
ARGS_LIST* create_args_list(ID_TABLE* method, ID_TYPE arg_type, const char* arg_name);
/* Adds an argument node into a temporary ARGS_LIST being built during parsing.
 */
ARGS_LIST* add_arg_current_list(ARGS_LIST* list, const char* name, ID_TYPE type);
/* Assigns a prepared list to a method symbol.
 */
void add_current_list(char* name, ARGS_LIST* list);
/* Return the actual scope (TABLE_STACK).
 */
TABLE_STACK* get_this_scope();
/* Adds an id to the global scope.
 */
ID_TABLE* add_global_id(char* name, ID_TYPE type);
/* Returns the argument list of a method.
 */
ARGS_LIST* get_method_args(const char* name);

#endif