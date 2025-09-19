#include "symbol_table.h"
#include "error_handling.h"
#include "utils.h"

// scope on top and global scope
TABLE_STACK* global_level = NULL;
TABLE_STACK* stack_level = NULL;     // top scope

extern int yylineno;

ID_TABLE* allocate_mem();
ARGS_LIST* allocate_args_list_mem();
ARGS* allocate_args_mem();

// creates a new scope associated with its superior scope
static TABLE_STACK* allocate_scope(TABLE_STACK* up) {
    TABLE_STACK* s = calloc(1, sizeof(TABLE_STACK));
    if (!s) error_allocate_mem();
    s->up = up;
    return s;
}

// initializes symbols' table (singleton)
void st_init() {
    if (!global_level) {
        global_level = allocate_scope(NULL);
        stack_level = global_level;
    }
}

// pushes a new scope in the stack
void push_scope() {
    if (!stack_level) st_init();
    if (stack_level == global_level) {
        stack_level = allocate_scope(global_level);
    } else {
        TABLE_STACK* aux = stack_level;
        stack_level = allocate_scope(aux->up);
        aux->up = stack_level;
    }

}

// frees all memory of one level in the table stack (probably we won't use this)
static void free_id_list(ID_TABLE* head) {
    while (head) {
        ID_TABLE* nxt = head->next;
        free(head->id_name);
        if (head->id_type == METHOD) {	// if the symbol is a method free all args and return value
            // free args list
            ARGS_LIST* al = head->method.arg_list;
            while (al) {
                ARGS_LIST* an = al->next;
                if (al->arg) {
                    free(al->arg->name);
                    free(al->arg);
                }
                free(al);
                al = an;
            }
            free(head->method.data);
        } else {
            free(head->common.data);
        }
        free(head);
        head = nxt;
    }
}

// pop the actual scope
void pop_scope(void) {
    if (!stack_level) return;
    TABLE_STACK* doomed = stack_level;
    stack_level = stack_level->up;
    // free_id_list(doomed->head_block);
    free(doomed);
    if (!stack_level) {
        global_level = NULL;
    }
}

/* creates a new node with id_name = name and returns its memory direction
   and doesn't allow to create two symbols with the same id in the same scope level */
ID_TABLE* add_id(char* name, const ID_TYPE type) {
    if (!stack_level) st_init();
    if (find_in_current_scope(name) != NULL) {
        error_variable_redeclaration(yylineno, name);
    }

    ID_TABLE* aux = allocate_mem();
    aux->id_name = my_strdup(name);
    if (!aux->id_name) error_allocate_mem();
    aux->id_type = type;

    if (stack_level->head_block == NULL) {
        stack_level->head_block = aux;
        stack_level->end_block = aux;
    } else {
        stack_level->end_block->next = aux;
        stack_level->end_block = aux;
    }
    return stack_level->end_block;
}

// declare a method in the actual scope with its return value
ID_TABLE* add_method(char* name, const RETURN_TYPE ret_type) {
    ID_TABLE* id = add_id(name, METHOD);
    id->method.return_type = ret_type;
    id->method.num_args = 0;
    id->method.arg_list = NULL;
    id->method.data = NULL;
    return id;
}

// adds data to the variable name node
void add_data(char* name, const ID_TYPE type, const void* data) {
    ID_TABLE* aux = find(name);
    if (aux == NULL) {
        error_variable_not_declared(yylineno, name);
    }
    if (aux->id_type == METHOD) {
        error_method_return_data();
    }
    if (aux->id_type != type) {
        error_type_mismatch(yylineno, name, (char*) aux->id_type);
    }

    if (aux->common.data != NULL) {
        free(aux->common.data);
    }

    switch(type) {
        case CONST_BOOL:
        case CONST_INT:
            aux->common.data = malloc(sizeof(int));
            if (!aux->common.data) error_allocate_mem();
            memcpy(aux->common.data, data, sizeof(int));
            return;
        default:
            error_type_mismatch(yylineno, name, (char*) aux->id_type);
    }
}

// adds data to the method's return value
void add_method_return_data(char* name, const RETURN_TYPE type, const void* data) {
    ID_TABLE* aux = find(name);
    if (aux == NULL) {
        error_variable_not_declared(yylineno, name);
    }
    if (aux->id_type != METHOD) {
        error_method_data();
    }
    if (type == RETURN_VOID) {
        error_return_void();
    }
    if (aux->method.return_type != type) {
        error_type_mismatch(yylineno, name, (char*) aux->id_type);
    }

    if (aux->method.data != NULL) {
        free(aux->method.data);
    }

    switch(type) {
        case CONST_INT:
        case CONST_BOOL:
            aux->method.data = malloc(sizeof(int));
            if (!aux->method.data) error_allocate_mem();
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
ID_TABLE* find(const char* name) {
    for (const TABLE_STACK* current_level = stack_level; current_level != NULL; current_level = current_level->up) {
        for (ID_TABLE* current_id = current_level->head_block; current_id; current_id = current_id->next) {
            if (current_id->id_name && strcmp(current_id->id_name, name) == 0) {
                return current_id;
            }
        }
    }
    return NULL;
}

/* return the memory direction of the node with id_name = name in the actual scope
   if the node is not found, returns NULL */
ID_TABLE* find_in_current_scope(const char* name) {
    if (!stack_level) return NULL;
    for (ID_TABLE* id = stack_level->head_block; id; id = id->next) {
        if (id->id_name && strcmp(id->id_name, name) == 0) return id;
    }
    return NULL;
}

// allocate memory for a node in the id_table
ID_TABLE* allocate_mem() {
    // calloc allocates memory and sets all its fields in 0 (NULL)
    ID_TABLE* aux = calloc(1, sizeof(ID_TABLE));
    if (!aux) error_allocate_mem();
    return aux;
}

// retrieves data of id from table
void* get_data(char* name) {
    const ID_TABLE* aux = find(name);
    if (aux == NULL) {
        error_variable_not_declared(yylineno, name);
    }

    switch (aux->id_type) {
        case CONST_INT:
        case CONST_BOOL:
            return aux->common.data;
        case METHOD:
            return aux->method.data;
        default:
            error_type_id_unknown();
    }
}

// add an argument to a given method
void add_arg(char* method_name, const ID_TYPE arg_type, const char* arg_name) {
    ID_TABLE* aux_table = find(method_name);
    if (!aux_table || aux_table->id_type != METHOD) {
        error_add_argument_method(method_name);
    }

    ARGS_LIST* aux_arg = aux_table->method.arg_list;
    if (aux_arg == NULL) {
        create_args_list(aux_table, arg_type, arg_name);
    } else {
        while (aux_arg->next != NULL) {
            aux_arg = aux_arg->next;
        }
        // allocates and sets the fields of ARGS and its place in ARGS_LIST
        ARGS* new_arg = allocate_args_mem();
        new_arg->type = arg_type;
        new_arg->name = my_strdup(arg_name);
        if (!new_arg->name) error_allocate_mem();
        ARGS_LIST* new_arg_place = allocate_args_list_mem();
        aux_arg->next = new_arg_place;
        new_arg_place->arg = new_arg;
        aux_table->method.num_args++;
    }
}

// creates the argument list of a given method
ARGS_LIST* create_args_list(ID_TABLE* method, const ID_TYPE arg_type, const char* arg_name) {
    if (method == NULL) {
        error_method_not_found((char*) method);
    }

    method->method.arg_list = allocate_args_list_mem();
    method->method.arg_list->arg = allocate_args_mem();
    method->method.arg_list->arg->name = my_strdup(arg_name);
    if (!method->method.arg_list->arg->name) error_allocate_mem();
    method->method.arg_list->arg->type = arg_type;
    method->method.num_args = 1;
    return method->method.arg_list;
}

// allocates memory for ARGS_LIST and initializes all fields in NULL
ARGS_LIST* allocate_args_list_mem() {
    ARGS_LIST* aux = calloc(1, sizeof(ARGS_LIST));
    if (!aux) error_allocate_mem();
    return aux;
}

// allocates memory for ARGS and initializes all fields in NULL
ARGS* allocate_args_mem() {
    ARGS* aux = calloc(1, sizeof(ARGS));
    if (!aux) error_allocate_mem();
    return aux;
}
