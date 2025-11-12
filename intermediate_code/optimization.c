#include "optimization.h"

CANT_AP_TEMP* get_free(CANT_AP_TEMP* tmp_list, char* temp);
CANT_AP_TEMP* get_temp(CANT_AP_TEMP* tmp_list, char* temp);
void swap_temps(CANT_AP_TEMP* free_temp, CANT_AP_TEMP* actual_temp);
void check_optimize(CANT_AP_TEMP* tmp_list, char* name);

/* Functions that optimizes memory by reutilizing temporals
 */
void optimize_memory(CANT_AP_TEMP* tmp_list) {
	Instr *int_code = get_intermediate_code();
	for (int i = 0; i < get_code_size(); i++) {
		if (int_code[i].var1 && int_code[i].var1->id.temp == 1) {
			check_optimize(tmp_list, int_code[i].var1->id.name);
		}
		if (int_code[i].var2 && int_code[i].var2->id.temp == 1) {
			check_optimize(tmp_list, int_code[i].var2->id.name);
		}
		if (int_code[i].reg && int_code[i].reg->id.temp == 1) {
			check_optimize(tmp_list, int_code[i].reg->id.name);
		}
	}
}

/* Returns first temporal that have 0 uses left
 * If temporal with 0 uses left is not found, returns NULL
 * Searches until it finds the same temporal of its arguments (because what you want to do is
 * try to "compress" temporals towards T0).
 * It wouldn't make sense to change T4 for T6 because then it would change T5 for T4 and then T6 for T5 (meaningless swap)
 */
CANT_AP_TEMP* get_free(CANT_AP_TEMP* tmp_list, char* temp) {
	CANT_AP_TEMP* aux = tmp_list;
	while (aux) {
		if (strcmp(aux->temp, temp) == 0) {
			return NULL;
		}
		if (aux->cant_ap == 0) {
			return aux;
		}
		aux = aux->next;
	}
	return NULL;
}

/* Returns node of tmp_list with temporal name -> temp
 * If not found returns NULL
 */
CANT_AP_TEMP* get_temp(CANT_AP_TEMP* tmp_list, char* temp) {
	CANT_AP_TEMP* aux = tmp_list;
	while (aux) {
		if (strcmp(aux->temp, temp) == 0) {
			return aux;
		}
		aux = aux->next;
	}
	return NULL;
}

/* Function that swap temps directly on intermediate code when an optimization can be done
 */
void swap_temps(CANT_AP_TEMP* free_temp, CANT_AP_TEMP* actual_temp) {
	free_temp->cant_ap = actual_temp->cant_ap - 1; // When this line is reached it is because actual_temp has already been found once.
	free_temp->locked = 1; // Mark new temp as locked because when this is reached it has already been used
	actual_temp->locked = 0; // Mark "old" temp as unlocked
	actual_temp->cant_ap = 0; // Old temp has 0 remaining uses (so another temp can be replaced by this one)
	TEMP_LIST* aux = actual_temp->list;
	while (aux) {
		// IMPORTANT: don't duplicate string, because if you do that the optimization won't impact on intermediate code
		strcpy(aux->location, free_temp->temp);
		aux = aux->next;
	}
	free_temp->list = actual_temp->list; // Assign old temp instances list to new temp
	actual_temp->list = NULL;
}

/* Helper for optimize_memory that checks if a name is a temporal and the checks if it can be replaced with a
 * temp that doesn't have remaining uses
 */
void check_optimize(CANT_AP_TEMP* tmp_list, char* name) {
	if (name[0] == 'T') {
		CANT_AP_TEMP* free_temp = get_free(tmp_list, name);
		CANT_AP_TEMP* actual_temp = get_temp(tmp_list, name);
		// Only optimize if there's a temp that is free and if the temp to be replaced is unlocked
		if (free_temp && !actual_temp->locked) {
			swap_temps(free_temp, actual_temp);
		} else {
			// Decrease the number of remaining uses and locks the temp
			actual_temp->cant_ap--;
			actual_temp->locked = 1;
		}
	}
}