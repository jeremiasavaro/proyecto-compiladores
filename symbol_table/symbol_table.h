#ifndef ID_TABLE_H
#define ID_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct ID_TABLE ID_TABLE;
typedef struct TABLE_STACK TABLE_STACK;
typedef struct ARGS ARGS;
typedef struct ARGS_LIST ARGS_LIST;

// pointer to the global level of table_stack
extern TABLE_STACK* global_level;

typedef enum {
	UNKNOWN,
	CONST_INT,
	CONST_BOOL,
	METHOD
} ID_TYPE;

typedef enum {
	INT,
	BOOL,
	VOID
} RETURN_TYPE;

struct TABLE_STACK {
	ID_TABLE* head_block;
	ID_TABLE* end_block;
	TABLE_STACK* up;
};

// node type for ARGS_LIST
struct ARGS {
	char* name;
	ID_TYPE type;
	void* data;
};

// list to save arguments of methods
struct ARGS_LIST {
	ARGS* arg;
	ARGS_LIST* next;
};

// node type for ID_TABLE (variable, constant or method)
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
			void* data; // store the return value of the method, if return_type is void this remains void as well
		} method;
	};
	ID_TABLE* next;
};

// pushes a new scope in the stack
void scope_push(void);
// frees all memory of one level in the table stack (probably we won't use this)
static void free_id_list(ID_TABLE* head);
// pop the actual scope
void scope_pop(void);
/* creates a new node with id_name = name and returns its memory direction
   and doesn't allow to create two symbols with the same id in the same scope level */
ID_TABLE* add_id(char* name, ID_TYPE type);
// declare a method in the actual scope with its return value
ID_TABLE* add_method(char* name, RETURN_TYPE ret_type);
// adds data to the variable name node
void add_data(char* name, ID_TYPE type, void* data);
// adds data to the method's return value
void add_method_return_data(char* name, RETURN_TYPE type, void* data);
/* returns the memory direction of the node with id_name = name
   if the node is not found, returns NULL
   first, it looks for the id in the current scope, if it doesn't find it,
   it goes up one scope level and keeps searching */
ID_TABLE* find(char* name);
/* return the memory direction of the node with id_name = name in the actual scope
   if the node is not found, returns NULL */
ID_TABLE* find_in_current_scope(char* name);
// retrieves data of id from table
void* get_data(char* name);
// add an argument to a given method
void add_arg(char* method_name, ID_TYPE arg_type, char* arg_name);
// creates the argument list of a given method
ARGS_LIST* create_args_list(ID_TABLE* method, ID_TYPE arg_type, char* arg_name);


#endif