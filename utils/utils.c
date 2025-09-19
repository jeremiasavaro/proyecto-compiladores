#include "utils.h"
#include <string.h>
#include <stdlib.h>

/* Portable strdup replacement to avoid implicit declaration issues. */
char *my_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *r = malloc(n);
    if (!r) return NULL;
    strcpy(r, s);
    return r;
}