#include "symbol.h"
#include "error_handling.h"

/* Allocates memory for ARGS_LIST and initializes all fields in NULL.
 */
ARGS_LIST* allocate_args_list_mem() {
	ARGS_LIST* aux = calloc(1, sizeof(ARGS_LIST));
	if (!aux) error_allocate_mem();
	return aux;
}

/* Allocates memory for ARGS and initializes all fields in NULL.
 */
ARGS* allocate_args_mem() {
	ARGS* aux = calloc(1, sizeof(ARGS));
	if (!aux) error_allocate_mem();
	return aux;
}

INFO* allocate_info_mem() {
	INFO* aux = calloc(1, sizeof(INFO));
	if (!aux) error_allocate_mem();
	return aux;
}