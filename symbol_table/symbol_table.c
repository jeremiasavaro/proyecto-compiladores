#include "symbol_table.h"
#include "error_handling.h"

// Scope top and global root
TABLE_STACK* global_level = NULL;		//scope global
static TABLE_STACK* stack_level = NULL;		//scope en el tope

extern int yylineno;

ID_TABLE* allocate_mem();
ARGS_LIST* allocate_args_list_mem();
ARGS* allocate_args_mem();

// crea un nuevo scope asociado a su scope superior.
static TABLE_STACK* allocate_scope(TABLE_STACK* up) {
    TABLE_STACK* s = malloc(sizeof(TABLE_STACK));
    if (!s) error_allocate_mem();
    s->head_block = NULL;
    s->end_block = NULL;
    s->up = up;
    return s;
}

// inicializa la TS.
void st_init(void) {
    if (!global_level) {
        global_level = allocate_scope(NULL);
        stack_level = global_level;
    }
}

// pushea un nuevo scope a la pila.
void scope_push(void) {
    if (!stack_level) st_init();
    stack_level = allocate_scope(stack_level);
}

// libera todos los símbolos perteneciente a un scope.
static void free_id_list(ID_TABLE* head) {
    while (head) {
        ID_TABLE* nxt = head->next;
        free(head->id_name);
        if (head->id_type == METHOD) {	// Si el símbolo es un metodo liberamos la lista de args y el retorno
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

// popea el scope actual
void scope_pop(void) {
    if (!stack_level) return;
    TABLE_STACK* doomed = stack_level;
    stack_level = stack_level->up;
    free_id_list(doomed->head_block);
    free(doomed);
    if (!stack_level) {
        global_level = NULL;
    }
}

// creates a new node with id_name = name and returns its memory direction
// ahora tambien prohibe redeclaracion en el mismo scope.
ID_TABLE* add_id(char* name, ID_TYPE type) {
    if (!stack_level) st_init();
    if (find_in_current_scope(name) != NULL) {
        error_variable_redeclaration(yylineno, name);
    }

    ID_TABLE* aux = allocate_mem();
    aux->id_name = strdup(name);
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

// declara un metodo en el scope actual con su tipo de valor de retorno.
// la declaro antes de procesar todo el cuerpo de la funcion, asi permitimos llamadas recursivas.
ID_TABLE* add_method(char* name, RETURN_TYPE ret_type) {

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
        error_type_mismatch(yylineno, name, "type mismatch");
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
            error_type_mismatch(yylineno, name, "unknown");
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
        error_type_mismatch(yylineno, name, "return type mismatch");
    }

    if (aux->method.data != NULL) {
        free(aux->method.data);
    }

    switch(type) {
        case INT:
        case BOOL:
            aux->method.data = malloc(sizeof(int));
            if (!aux->method.data) error_allocate_mem();
            memcpy(aux->method.data, data, sizeof(int));
            return;
        default:
            error_type_mismatch(yylineno, name, "unknown");
    }
}

/* returns the memory direction of the node with id_name = name
   if the node is not found, returns NULL
   first, it looks for the id in the current scope, if it doesn't find it,
   it goes up one scope level and keeps searching */
ID_TABLE* find(char* name) {
    for (TABLE_STACK* current_level = stack_level; current_level != NULL; current_level = current_level->up) {
        for (ID_TABLE* current_id = current_level->head_block; current_id; current_id = current_id->next) {
            if (current_id->id_name && strcmp(current_id->id_name, name) == 0) {
                return current_id;
            }
        }
    }
    return NULL;
}

// busca un id en el scope actual.
ID_TABLE* find_in_current_scope(char* name) {

}

// allocate memory for a node in the id_table
ID_TABLE* allocate_mem() {
    ID_TABLE* aux = malloc(sizeof(ID_TABLE));
    if (!aux) {
        error_allocate_mem();
    }
    aux->id_name = NULL;
    aux->id_type = UNKNOWN;
    aux->next = NULL;
    aux->common.data = NULL; // safe for both unions
    return aux;
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
    if (!aux_table || aux_table->id_type != METHOD) {
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
        new_arg->name = strdup(arg_name);
        if (!new_arg->name) error_allocate_mem();
        ARGS_LIST* new_arg_place = allocate_args_list_mem();
        aux_arg->next = new_arg_place;
        new_arg_place->arg = new_arg;
    }
    aux_table->method.num_args++;
}

// creates the argument list of a given method
ARGS_LIST* create_args_list(ID_TABLE* method, ID_TYPE arg_type, char* arg_name) {
    if (method == NULL) {
        fprintf(stderr, "Error: method is NULL in create_args_list \n");
        exit(EXIT_FAILURE);
    }

    method->method.arg_list = allocate_args_list_mem();
    method->method.arg_list->arg = allocate_args_mem();
    method->method.arg_list->arg->name = strdup(arg_name);
    if (!method->method.arg_list->arg->name) error_allocate_mem();
    method->method.arg_list->arg->type = arg_type;
    method->method.num_args = 1;
    return method->method.arg_list;
}

ARGS_LIST* allocate_args_list_mem() {
    ARGS_LIST* aux = malloc(sizeof(ARGS_LIST));
    if (!aux) error_allocate_mem();
    aux->arg = NULL;
    aux->next = NULL;
    return aux;
}

ARGS* allocate_args_mem() {
    ARGS* aux = malloc(sizeof(ARGS));
    if (!aux) error_allocate_mem();
    aux->data = NULL;
    aux->name = NULL;
    aux->type = UNKNOWN;
    return aux;
}
