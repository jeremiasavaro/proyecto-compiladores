#include "symbol_table.h"

/* Scope on top and global scope. */
TABLE_STACK* global_level = NULL;
TABLE_STACK* stack_level = NULL;

extern int yylineno;

ID_TABLE* allocate_mem();

/* Creates a new scope associated with its superior scope.
 */
static TABLE_STACK* allocate_scope(TABLE_STACK* up) {
    TABLE_STACK* s = calloc(1, sizeof(TABLE_STACK));
    if (!s) error_allocate_mem();
    s->up = up;
    return s;
}

/* Initializes symbols' table (singleton).
 */
void st_init() {
    if (!global_level) {
        global_level = allocate_scope(NULL);
        stack_level = global_level;
    }
}

/* Pushes a new scope in the stack.
 */
void push_scope() {
    if (!stack_level) st_init();
    TABLE_STACK* aux = stack_level;
    stack_level = allocate_scope(aux);
}

/* Pop the actual scope.
 */
void pop_scope(void) {
    if (!stack_level) return;
    if (global_level != stack_level) {
        stack_level = stack_level->up;
    }
}

/* Creates a new node with id_name = name and returns its memory direction
 * and doesn't allow to create two symbols with the same id in the same scope level.
 */
ID_TABLE* add_id(char* name, const TYPE type) {
    if (!stack_level) st_init();
    if (find_in_current_scope(name) != NULL) {
        error_variable_redeclaration(yylineno, name);
    }

    ID_TABLE* aux = allocate_mem();
    aux->info = allocate_info_mem();
    aux->info->id.name = my_strdup(name);
    aux->info->type = TABLE_ID;
    if (!aux->info->id.name) error_allocate_mem();
    aux->info->id.type = type;

    if (stack_level->head_block == NULL) {
        stack_level->head_block = aux;
        stack_level->end_block = aux;
    } else {
        stack_level->end_block->next = aux;
        stack_level->end_block = aux;
    }

    return stack_level->end_block;
}

/* Adds an id to the global scope.
 */
ID_TABLE* add_global_id(char* name, TYPE id_type) {
    if (find(name) != NULL) {
        error_variable_redeclaration(yylineno, name);
    }

    ID_TABLE* aux = allocate_mem();
    aux->info = allocate_info_mem();
    aux->info->id.name = my_strdup(name);
    if (!aux->info->id.name) error_allocate_mem();
    aux->info->id.type = id_type;

    if (!global_level) st_init();

    if (global_level->head_block == NULL) {
        global_level->head_block = aux;
        global_level->end_block = aux;
    } else {
        global_level->end_block->next = aux;
        global_level->end_block = aux;
    }

    return global_level->end_block;
}

/* Declare a method in the global scope with its return value.
 */
ID_TABLE* add_method(char* name, const RETURN_TYPE ret_type, TABLE_STACK* method_scope, int is_extern) {
    if (find(name) != NULL) {
        error_variable_redeclaration(yylineno, name);
    }

    ID_TABLE* aux = allocate_mem();
    aux->info = allocate_info_mem();
    aux->info->type = AST_METHOD_DECL;
    aux->info->method_decl.name = my_strdup(name);
    aux->info->method_decl.return_type = ret_type;
    aux->info->method_decl.num_args = 0;
    aux->info->method_decl.args = NULL;
    aux->info->method_decl.scope = method_scope;
    aux->info->method_decl.is_extern = is_extern;

    if (!global_level) st_init();

    if (global_level->head_block == NULL) {
        global_level->head_block = aux;
        global_level->end_block = aux;
    } else {
        global_level->end_block->next = aux;
        global_level->end_block = aux;
    }

    return global_level->end_block;
}

/* Return the actual scope (TABLE_STACK).
 */
TABLE_STACK* get_this_scope() {
    return stack_level;
}

/* Returns the memory direction of the node with id_name = name.
 * If the node is not found, returns NULL.
 * First, it looks for the id in the current scope, if it doesn't find it,
 * it goes up one scope level and keeps searching.
 */
ID_TABLE* find(const char* name) {
    for (const TABLE_STACK* current_level = stack_level; current_level != NULL; current_level = current_level->up) {
        for (ID_TABLE* current_id = current_level->head_block; current_id; current_id = current_id->next) {
            if (current_id->info->type == AST_METHOD_DECL) {
                if (current_id->info->method_decl.name && strcmp(current_id->info->method_decl.name, name) == 0) {
                    return current_id;
                }
            } else {
                if (current_id->info->id.name && strcmp(current_id->info->id.name, name) == 0) {
                    return current_id;
                }
            }
        }
    }
    return NULL;
}

/* Returns the memory direction of the node with id_name = name in the actual scope
 * if the node is not found, returns NULL.
 */
ID_TABLE* find_in_current_scope(const char* name) {
    if (!stack_level) return NULL;
    for (ID_TABLE* id = stack_level->head_block; id; id = id->next) {
        if (id->info->type == AST_METHOD_DECL) {
            if (id->info->method_decl.name && strcmp(id->info->method_decl.name, name) == 0) return id;
        } else {
            if (id->info->id.name && strcmp(id->info->id.name, name) == 0) return id;
        }
    }
    return NULL;
}

/* Returns the memory direction of the node with id_name = name in the global scope
 * if the node is not found, returns NULL.
 */
ID_TABLE* find_global(const char* name) {
    if (!global_level) return NULL;
    for (ID_TABLE* id = global_level->head_block; id; id = id->next) {
        if (id->info->type == AST_METHOD_DECL) {
            if (id->info->method_decl.name && strcmp(id->info->method_decl.name, name) == 0) return id;
        } else {
            if (id->info->id.name && strcmp(id->info->id.name, name) == 0) return id;
        }
    }
    return NULL;
}

/* Allocate memory for a node in the id_table.
 */
ID_TABLE* allocate_mem() {
    // calloc allocates memory and sets all its fields in 0 (NULL)
    ID_TABLE* aux = calloc(1, sizeof(ID_TABLE));
    if (!aux) error_allocate_mem();
    return aux;
}

/* Adds an argument to a given method.
 */
void add_arg(char* method_name, const TYPE arg_type, const char* arg_name) {
    ID_TABLE* aux_table = find_global(method_name);
    if (!aux_table || aux_table->info->type != AST_METHOD_DECL) {
        error_add_argument_method(method_name);
    }

    ARGS_LIST* aux_arg = aux_table->info->method_decl.args;
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
        aux_table->info->method_decl.num_args++;
    }
}

/* Creates the argument list of a given method.
 */
ARGS_LIST* create_args_list(ID_TABLE* method, const TYPE arg_type, const char* arg_name) {
    if (method == NULL) {
        error_method_not_found((char*) method);
    }

    method->info->method_decl.args = allocate_args_list_mem();
    method->info->method_decl.args->arg = allocate_args_mem();
    method->info->method_decl.args->arg->name = my_strdup(arg_name);
    if (!method->info->method_decl.args->arg->name) error_allocate_mem();
    method->info->method_decl.args->arg->type = arg_type;
    method->info->method_decl.num_args = 1;
    return method->info->method_decl.args;
}

/* Adds an argument node into a temporary ARGS_LIST being built during parsing.
 */
ARGS_LIST* add_arg_current_list(ARGS_LIST* list, const char* name, TYPE type) {
    if (list) {
        ARGS_LIST* tail = list;
        while (tail->next) tail = tail->next;
        ARGS* new_arg = allocate_args_mem();
        new_arg->name = my_strdup(name);
        if (!new_arg->name) error_allocate_mem();
        new_arg->type = type;
        ARGS_LIST* node = allocate_args_list_mem();
        node->arg = new_arg;
        tail->next = node;
        return list;
    }
    ARGS_LIST* head = allocate_args_list_mem();
    head->arg = allocate_args_mem();
    head->arg->name = my_strdup(name);
    if (!head->arg->name) error_allocate_mem();
    head->arg->type = type;
    head->next = NULL;
    return head;
}

/* Assigns a prepared list to a method symbol.
 */
void add_current_list(char* name, ARGS_LIST* list) {
    ID_TABLE* meth = find_global(name);
    if (!meth) {
        error_add_argument_method(name);
    }
    meth->info->method_decl.args = list;
    int count = 0;
    for (ARGS_LIST* it = list; it; it = it->next) count++;
    meth->info->method_decl.num_args = count;
}

/* Returns the argument list of a method.
 */
ARGS_LIST* get_method_args(const char* name) {
    ID_TABLE* meth = find_global(name);
    if (!meth) {
        error_method_not_found(name);
    }
    return meth->info->method_decl.args;
}