#include "symbol_table.h"
#include "error_handling.h"

extern int yylineno;

TABLE_STACK* global_level = NULL;
static TABLE_STACK* stack_level = NULL;	//representa un scope

ID_TABLE* allocate_mem();
ARGS_LIST* allocate_args_list_mem();
ARGS* allocate_args_mem();

// creates a new node with id_name = name and returns its memory direction
ID_TABLE* add_id(char* name, ID_TYPE type) {
	// Check for redeclaration before adding to table
	if(find(name) != NULL) {
		error_variable_redeclaration(yylineno, name);
	}

	ID_TABLE* aux = allocate_mem();
	aux->id_name = strdup(name);
	aux->id_type = type;
	if (stack_level->head_block == NULL) {
		stack_level->head_block = aux;
	} else {
		stack_level->end_block->next = aux;
		stack_level->end_block = aux;
	}

	return stack_level->end_block;
}

// adds data to the variable name node
void add_data(char* name, ID_TYPE type, void* data) {
	ID_TABLE* aux = find(name);
	if (aux == NULL) {
		error_variable_not_declared(yylineno, name);
	}
	if (aux->id_type == METHOD) {
		fprintf(stderr, "Error: can't assign data to a method id, use add_method_return_data() instead\n");
		exit(EXIT_FAILURE);
	}
	if (aux->id_type != type) {
		error_type_mismatch(yylineno, name, (char*) aux->id_type);
	}

	// Only free if data was previously allocated
	if (aux->common.data != NULL) {
		free(aux->common.data);
	}

	switch(type) {
		// allocate memory and copy data into the id_table
        case CONST_BOOL:
        case CONST_INT:
            aux->common.data = malloc(sizeof(int));
            memcpy(aux->common.data, data, sizeof(int));
            return;
		default:
			error_type_mismatch(yylineno, name, (char*) aux->id_type);
    }
}

void add_method_return_data(char* name, RETURN_TYPE type, void* data) {
	ID_TABLE* aux = find(name);
	if (aux == NULL) {
		error_variable_not_declared(yylineno, name);
	}
	if (aux->id_type != METHOD) {
		fprintf(stderr, "Error: can't assign data to a non-method id, use add_data() instead\n");
		exit(EXIT_FAILURE);
	}
	if (type == VOID) {
		fprintf(stderr, "Error: can't assign return value of type VOID\n");
		exit(EXIT_FAILURE);
	}
	if (aux->method.return_type != type) {
		error_type_mismatch(yylineno, name, (char*) aux->id_type);
	}

	if (aux->method.data != NULL) {
		free(aux->common.data);
	}

	switch(type) {
		// allocate memory and copy data into the id_table
		case CONST_BOOL:
		case CONST_INT:
			aux->method.data = malloc(sizeof(int));
			memcpy(aux->method.data, data, sizeof(int));
			return;
		default:
			error_type_mismatch(yylineno, name, (char*) aux->id_type);
	}
}

/* returns the memory direction of the node with id_name = name
   if the node is not found, returns NULL
   first, it looks for the id in the current scope, if it doesn't find it,
   it goes up one scope level and keeps searching */
ID_TABLE* find(char* name) {
	for (TABLE_STACK* current_level = stack_level; current_level != NULL; current_level = current_level->up) {
		for (ID_TABLE* current_id = stack_level->head_block; current_id; current_id = current_id->next) {
			if (current_id->id_name && strcmp(current_id->id_name, name) == 0) {
				return current_id;
			}
		}
	}
    return NULL;
}

// allocate memory for a node in the id_table
ID_TABLE* allocate_mem() {
	ID_TABLE* aux = malloc(sizeof(ID_TABLE));
	if (!aux) {
        error_allocate_mem();
    }

	// initializes all data to NULL
	aux->id_name = NULL;
	aux->id_type = UNKNOWN;
	aux->next = NULL;

	return aux;
}

// creates a new scope
void create_scope() {
}

// retrieves data of id from table
void* get_data(char* name) {
	ID_TABLE* aux = find(name);
	if (aux == NULL) {
		fprintf(stderr, "Error: id not found\n");
		exit(EXIT_FAILURE);
	}

	switch (aux->id_type) {
		case CONST_INT:
		case CONST_BOOL:
			return aux->common.data;
		case METHOD:
			return aux->method.data;
		default:
			fprintf(stderr, "Error: id's type unknown\n");
			exit(EXIT_FAILURE);
	}
}

// add an argument to a given method
void add_arg(char* method_name, ID_TYPE arg_type, char* arg_name) {
	ID_TABLE* aux_table = find(method_name);
	if (aux_table->id_type != METHOD) {
		fprintf(stderr, "Error: trying to add argument to non-method %s\n", method_name);
		exit(EXIT_FAILURE);
	}

	ARGS_LIST* aux_arg = aux_table->method.arg_list;
	if (aux_arg == NULL) {
		create_args_list(aux_table, arg_type, arg_name);
	} else {
		while (aux_arg->next != NULL) {
			aux_arg = aux_arg->next;
		}
		ARGS* new_arg = allocate_args_mem();
		new_arg->type = arg_type;
		new_arg->name = arg_name;
		ARGS_LIST* new_arg_place = allocate_args_list_mem();
		aux_arg->next = new_arg_place;
		new_arg_place->arg = new_arg;
	}
}

// creates the argument list of a given method
ARGS_LIST* create_args_list(ID_TABLE* method, ID_TYPE arg_type, char* arg_name) {
	if (method == NULL) {
		fprintf(stderr, "Error: method is NULL in create_args_list \n");
		exit(EXIT_FAILURE);
	}

	method->method.arg_list = allocate_args_list_mem();
	method->method.arg_list->arg = allocate_args_mem();
	method->method.arg_list->arg->name = arg_name;
	method->method.arg_list->arg->type = arg_type;
	return method->method.arg_list;
}

ARGS_LIST* allocate_args_list_mem() {
	ARGS_LIST* aux = malloc(sizeof(ARGS_LIST));
	aux->arg = NULL;
	aux->next = NULL;

	return aux;
}

ARGS* allocate_args_mem() {
	ARGS* aux = malloc(sizeof(ARGS));
	aux->data = NULL;
	aux->name = NULL;
	aux->type = UNKNOWN;

	return aux;
}


//implementacion de metodos para el scope
static TABLE_STACK* allocate_scope(TABLE_STACK* up) {
    TABLE_STACK* s = malloc(sizeof(TABLE_STACK));
    if (!s) error_allocate_mem();
    s->head_block = NULL;
    s->end_block = NULL;
    s->up = up;
    return s;
}

void stack_init(void) {
    if (!global_level) {
        global_level = allocate_scope(NULL);
        stack_level = global_level;
    }
}

void scope_push(void) {
    if (!stack_level) st_init();
    stack_level = allocate_scope(stack_level);
}