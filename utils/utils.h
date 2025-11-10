#ifndef UTILS
#define UTILS

#include <string.h>
#include <stdlib.h>

/* Portable strdup replacement to avoid implicit declaration issues. */
char *my_strdup(const char *s);

#endif