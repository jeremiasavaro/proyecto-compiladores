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

void error_method_return_data() {
    fprintf(stderr, "Error: can't assign data to a method id, use add_method_return_data() instead\n");
    exit(EXIT_FAILURE);
}

void error_method_data() {
    fprintf(stderr, "Error: can't assign data to a non-method id, use add_data() instead\n");
    exit(EXIT_FAILURE);
}

void error_return_void() {
    fprintf(stderr, "Error: can't assign return value of type VOID\n");
    exit(EXIT_FAILURE);
}

void error_type_id_unknown() {
    fprintf(stderr, "Error: id's type unknown\n");
    exit(EXIT_FAILURE);
}

void error_add_argument_method(char* method_name) {
    fprintf(stderr, "Error: trying to add argument to non-method %s\n", method_name);
    exit(EXIT_FAILURE);
}

void error_method_not_found(char* method_name) {
    fprintf(stderr, "Error: method %s not found \n", method_name);
    exit(EXIT_FAILURE);
}

void error_null_node(int line){
    fprintf(stderr, "ERROR(line %d): NULL node in eval()\n", line);
    exit(EXIT_FAILURE);
}