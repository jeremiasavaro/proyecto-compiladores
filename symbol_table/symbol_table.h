#ifndef ID_TABLE_H
#define ID_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "error_handling.h"
#include "utils.h"
#include "symbol.h"

typedef struct ID_TABLE ID_TABLE;
typedef struct TABLE_STACK TABLE_STACK;

// Pointer to the global level of table_stack.
extern TABLE_STACK* global_level;

struct TABLE_STACK {
	ID_TABLE* head_block;
	ID_TABLE* end_block;
	TABLE_STACK* up;
};

// Node type for ID_TABLE (variable, constant or method).
struct ID_TABLE {
	INFO* info;
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
ID_TABLE* add_id(char* name, TYPE type);
/* Declare a method in the global scope with its return value.
 */
ID_TABLE* add_method(char* name, const RETURN_TYPE ret_type, TABLE_STACK* method_scope, int is_extern);
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
/* Adds an argument to a given method.
 */
void add_arg(char* method_name, TYPE arg_type, const char* arg_name);
/* Creates the argument list of a given method.
 */
ARGS_LIST* create_args_list(ID_TABLE* method, TYPE arg_type, const char* arg_name);
/* Adds an argument node into a temporary ARGS_LIST being built during parsing.
 */
ARGS_LIST* add_arg_current_list(ARGS_LIST* list, const char* name, TYPE type);
/* Assigns a prepared list to a method symbol.
 */
void add_current_list(char* name, ARGS_LIST* list);
/* Return the actual scope (TABLE_STACK).
 */
TABLE_STACK* get_this_scope();
/* Adds an id to the global scope.
 */
ID_TABLE* add_global_id(char* name, TYPE id_type);
/* Returns the argument list of a method.
 */
ARGS_LIST* get_method_args(const char* name);

#endif