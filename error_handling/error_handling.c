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

void error_conditional(int line){
    fprintf(stderr, "ERROR(line %d): conditional expression must be of type BOOL\n", line);
    exit(EXIT_FAILURE);
}

void additional_error(int line){
    fprintf(stderr, "ERROR(linea %d): addition is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void substraction_error(int line){
    fprintf(stderr, "ERROR(line %d): substraction is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void multiplication_error(int line){
    fprintf(stderr, "ERROR(line %d): multiplication is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void division_error(int line){
    fprintf(stderr, "ERROR(line %d): divison is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void division_zero_error(int line){
    fprintf(stderr, "ERROR(line %d): division by zero\n", line);
    exit(EXIT_FAILURE);
}

void minus_error(int line){
    fprintf(stderr, "ERROR(line %d): minus is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void and_error(int line){
    fprintf(stderr, "ERROR(line %d): AND is only for booleans \n", line);
    exit(EXIT_FAILURE);
}

void or_error(int line){
    fprintf(stderr, "ERROR(line %d): OR is only for booleans \n", line);
    exit(EXIT_FAILURE);
}

void neg_error(int line){
    fprintf(stderr, "ERROR(line %d): NEG is only for booleans \n", line);
    exit(EXIT_FAILURE);
}

void assign_error(int line){
    fprintf(stderr, "ERROR(line %d): invalid left-hand side of assignment\n", line);
    exit(EXIT_FAILURE);
}

void error_return_int(int line){
    fprintf(stderr, "ERROR(line %d): main should return int\n", line);
    exit(EXIT_FAILURE);
}

void error_return_bool(int line){
    fprintf(stderr, "ERROR(line %d): main should return bool\n", line);
    exit(EXIT_FAILURE);
}

void error_return_void(int line){
    fprintf(stderr, "ERROR(line %d): main should return void\n", line);
    exit(EXIT_FAILURE);
}

void error_unespected_return_int(int line){
    fprintf(stderr, "ERROR(line %d): main returns void when it should return int\n", line);
    exit(EXIT_FAILURE);   
}

void error_unespected_return_bool(int line){
    fprintf(stderr, "ERROR(line %d): main returns void when it should return bool\n", line);
    exit(EXIT_FAILURE);   
}

void error_unknown_operator(int line){
    fprintf(stderr, "ERROR(line %d): unknown operator in interpreter\n", line);
    exit(EXIT_FAILURE);
}

void error_variable_used_before_init(int line, char* variable){
    fprintf(stderr, "ERROR(line %d): variable '%s' used before initialization\n", line, variable);
    exit(EXIT_FAILURE);
}

void error_id_unknown_type(int line, char* variable){
    fprintf(stderr, "ERROR(line %d): identifier '%s' has unknown type\n", line, variable);
    exit(EXIT_FAILURE);
}

void error_unknown_leaf_type(int line){
    fprintf(stderr, "ERROR(line %d): unknown leaf type\n", line);
    exit(EXIT_FAILURE);
}

void error_noexistent_id(int line){
    fprintf(stderr, "ERROR(line %d): nonexistent identifier '%s'\n", line);
    exit(EXIT_FAILURE);
}

void warning_already_returned(int line){
    fprintf(stderr, "WARNING(line %d): return statement ignored, already returned once\n", line);
    return;
}

void error_missing_return(int line){
    fprintf(stderr, "ERROR(line %d): missing return statement in main\n", line);
    exit(EXIT_FAILURE);
}