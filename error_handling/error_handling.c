#include "error_handling.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void error_parse(int line, char* msg) {
    fprintf(stderr, "Parse error(line %d): %s \n", line, msg);
    exit(EXIT_FAILURE);
}

void error_lexical(int line, char* msg) {
    fprintf(stderr, "Lexical error(line %d): unknown character '%s'\n", line, msg);
    exit(EXIT_FAILURE); 
}

int error_open_file(char* filename) {
    fprintf(stderr, "Error: could not open file %s\n", filename);
    return 1;
}

void error_variable_redeclaration(int line, char* msg) {
    fprintf(stderr, "Error(line %d): variable %s redeclared \n", line, msg);
    exit(EXIT_FAILURE);
}

void error_variable_not_declared(int line, char* msg) {
    fprintf(stderr, "Error(line %d): variable %s not declared \n", line, msg);
    exit(EXIT_FAILURE);
}

void error_type_mismatch(int line, char* msg, char* type) {
    fprintf(stderr, "Error(line %d): conflicting types for variable %s (should be %s) \n", line, msg, type);
    exit(EXIT_FAILURE);
}

void error_allocate_mem() {
    fprintf(stderr, "Error: Failed to allocate memory \n");
    exit(EXIT_FAILURE);
}