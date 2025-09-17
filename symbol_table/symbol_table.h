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
	CONST_INT,
	CONST_BOOL,
	METHOD,
	UNKNOWN
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


ID_TABLE* add_id(char* name, ID_TYPE type);
void add_data(char* name, ID_TYPE type, void* data);
ID_TABLE* find(char* name);
void* get_data(char* name);
ARGS_LIST* create_args_list(ID_TABLE* method, ID_TYPE arg_type, char* arg_name);

//metodos para el scope (estos son los que añadi tito maxi)

// crea el scope global si aún no existe.
void st_init(void);

// pushea un nuevo scope (para entrar a un bloque o al cuerpo de un método).
void scope_push(void);

// popea el scope actual y hace free de los símbolos locales.
void scope_pop(void);

// declara un método en el scope actual (permite la recursion).
ID_TABLE* add_method(char* name, RETURN_TYPE ret_type);

// busca dentro del scope que esta en el tope (para ver que no haya redeclaraciones).
ID_TABLE* find_in_current_scope(char* name);

#endif