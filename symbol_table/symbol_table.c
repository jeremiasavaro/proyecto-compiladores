#define _GNU_SOURCE
#include "symbol_table.h"
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "error_handling.h"

extern int yylineno;

ROOT_TABLE* head_table = NULL;
ROOT_TABLE* curr_table = NULL;

// creates a new node with id_name = name and returns its memory direction
ID_TABLE* add_id(char* name, ID_TYPE type) {
	// Check for redeclaration before adding to table
	if(find(name) != NULL) {
		redeclaration_variable(yylineno, name);
	}

	ID_TABLE* aux = allocate_mem();
	if (curr_table->block == NULL) {
		curr_table->block = head_table = aux;
	} else {
		curr_table->block->next = aux;
		curr_table->block = aux;
	}

	curr_table->block->id_name = strdup((char*)name);
	curr_table->block->id_type = type;
	return curr_table->block;
}

// adds data to the variable name node
void add_data(char* name, ID_TYPE type, void* data) {
	ID_TABLE* aux = find(name);
	if (aux == NULL) {
		variable_not_declared(yylineno, name);
	}
	if (aux->id_type != type) {
		type_mismatch(yylineno, name);
	}

	// Only free if data was previously allocated
	if (aux->data != NULL) {
		free(aux->data);
	}

	switch(type) {
		// allocate memory and copy data into the node
        case CONST_BOOL:
            aux->data = malloc(sizeof(int));
            memcpy(aux->data, data, sizeof(int));
            return;
        case CONST_INT:
            aux->data = malloc(sizeof(int));
            memcpy(aux->data, data, sizeof(int));
            return;
        case UNKNOWN:
            variable_not_declared(yylineno, name);
    }

	return;
}

/* returns the memory direction of the node with id_name = name
   if the node is not found, returns NULL */
ID_TABLE* find(char* name) {
	for (ID_TABLE* cur = curr_table->block; cur; cur = cur->next) {
        if (cur->id_name && strcmp(cur->id_name, name) == 0) {
            return cur;
        }
    }
    return NULL;
}

// allocate memory for a node in the simbols' table
ID_TABLE* allocate_mem() {
	ID_TABLE* aux = malloc(sizeof(ID_TABLE));
	if (!aux) {
        allocate_mem_error();
    }

	// initializes all data to NULL
	aux->id_name = NULL;
	aux->id_type = UNKNOWN;
	aux->data = NULL;
	aux->next = NULL;

	return aux;
}

// creates the root list
void create_root(ID_TABLE* tree) {
    head_table = (ROOT_TABLE*) malloc(sizeof(ROOT_TABLE));
    head_table->block = tree;
    head_table->next = NULL;
    curr_table = head_table;
}

// add new node in root list
void add_block(ID_TABLE* tree) {
    /* if there is no root list yet, create one
       this works like a singleton pattern, only create one root list */
    if (head_table == NULL) {
        create_root(tree);
        return;
    } else {
        ROOT_TABLE *aux = malloc(sizeof(ROOT_TABLE));
        aux->block = tree;
        aux->next = NULL;
        curr_table->next = aux;
        curr_table = aux;
        return;
    }
}

// retrieves data of id from table
void* get_data(char* name) {
	ID_TABLE* aux = find(name);
	return aux->data;
}
