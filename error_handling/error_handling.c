#include "error_handling.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void error_parse(int line, char* msg) {
    fprintf(stderr, "Parse error: %s in line %d\n", msg, line);
    exit(EXIT_FAILURE);
}

void error_lexical(int line, char* msg) {
    fprintf(stderr, "Lexical ERROR(line %d): unknown character '%s'\n", line, msg); 
    exit(EXIT_FAILURE); 
}

void error_open_file(char* filename) {
    fprintf(stderr, "Error: could not open file %s\n", filename);
    return 1;
}