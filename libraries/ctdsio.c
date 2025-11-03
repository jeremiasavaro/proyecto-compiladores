#include "ctdsio.h"

void print_int(int s) {
	printf("%d\n", s);
}

void print_bool(int s) {
	if (s) printf("TRUE\n");
	else printf("FALSE\n");
}

int get_int() {
    int x;
	//printf("Ingrese un entero: ");
    scanf("%d", &x);
    return x;
}

int get_bool() {
    char x[5];
	//printf("Ingrese el valor booleano: ");
    scanf("%s", x);
	if (strcmp(x, "TRUE") == 0) {
		return 1;
	} else {
		if (strcmp(x, "FALSE") == 0) {
			return 0;
		} else {
			//error_get_bool();
			return -1; // This line will never be reached, but is added to avoid compiler warnings
		}
	}
}
