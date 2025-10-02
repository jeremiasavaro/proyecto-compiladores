#include "error_handling.h"

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
    exit(EXIT_FAILURE);
}

void error_variable_redeclaration(int line, char* msg) {
    fprintf(stderr, "Error(line %d): variable %s redeclared \n", line, msg);
    exit(EXIT_FAILURE);
}

void error_variable_not_declared(int line, char* msg) {
    fprintf(stderr, "Error(line %d): variable %s not declared \n", line, msg);
    exit(EXIT_FAILURE);
}

void error_type_mismatch(int line, char* variable, char* type) {
    fprintf(stderr, "Error(line %d): conflicting types for variable %s (should be %s) \n", line, variable, type);
    exit(EXIT_FAILURE);
}

void error_type_parameter(int line, char* parameter_name, char* type) {
    fprintf(stderr, "Error(line %d): conflicting types for parameter %s (should be %s) \n", line, parameter_name, type);
    exit(EXIT_FAILURE);
}

void error_type_mismatch_method(int line, char* method, int type) {
    switch (type)
    {
    case 0:
        fprintf(stderr, "Error(line %d): conflicting types for return method %s (should be integer) \n", line, method);
        break;
    case 1:
        fprintf(stderr, "Error(line %d): conflicting types for return method %s (should be bool) \n", line, method);
        break;
    default:
        fprintf(stderr, "Error(line %d): conflicting types for return method %s (should not return a value) \n", line, method);
        break;
    }
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

void error_method_not_found(const char* method_name) {
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

void error_additional(int line){
    fprintf(stderr, "ERROR(linea %d): addition is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void error_substraction(int line){
    fprintf(stderr, "ERROR(line %d): substraction is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void error_multiplication(int line){
    fprintf(stderr, "ERROR(line %d): multiplication is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void error_division(int line){
    fprintf(stderr, "ERROR(line %d): divison is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void error_minus(int line){
    fprintf(stderr, "ERROR(line %d): minus is only for integers\n", line);
    exit(EXIT_FAILURE);
}

void error_and(int line){
    fprintf(stderr, "ERROR(line %d): AND is only for booleans \n", line);
    exit(EXIT_FAILURE);
}

void error_or(int line){
    fprintf(stderr, "ERROR(line %d): OR is only for booleans \n", line);
    exit(EXIT_FAILURE);
}

void error_neg(int line){
    fprintf(stderr, "ERROR(line %d): NEG is only for booleans \n", line);
    exit(EXIT_FAILURE);
}

void error_assign(int line){
    fprintf(stderr, "ERROR(line %d): invalid left-hand side of assignment\n", line);
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

void error_non_existent_id(int line){
    fprintf(stderr, "ERROR(line %d): non-existent identifier \n", line);
    exit(EXIT_FAILURE);
}

void warning_already_returned(int line){
    fprintf(stderr, "WARNING(line %d): return statement ignored, already returned once\n", line);
    return;
}

void error_less(int line){
    fprintf(stderr, "ERROR(line %d): operator < is only for integers \n", line);
    exit(EXIT_FAILURE);
}

void error_greater(int line){
    fprintf(stderr, "ERROR(line %d): operator > is only for integers \n", line);
    exit(EXIT_FAILURE);
}

void error_equal(int line){
    fprintf(stderr, "ERROR(line %d): operator == is only for integers \n", line);
    exit(EXIT_FAILURE);
}

void error_not_equal(int line){
    fprintf(stderr, "ERROR(line %d): operator != is only for integers \n", line);
    exit(EXIT_FAILURE);
}

void error_less_equal(int line){
    fprintf(stderr, "ERROR(line %d): operator <= is only for integers \n", line);
    exit(EXIT_FAILURE);
}

void error_greater_equal(int line){
    fprintf(stderr, "ERROR(line %d): operator >= is only for integers \n", line);
    exit(EXIT_FAILURE);
}

void error_different_return_types(int line, char* firstType, char* second_type) {
    fprintf(stderr, "ERROR(line %d): returned types %s and %s are not the same \n", line, firstType, second_type);
    exit(EXIT_FAILURE);
}

void error_args_number(int line, char* method, int num_args) {
    fprintf(stderr, "ERROR(line %d): method %s should have %d args \n", line, method, num_args);
    exit(EXIT_FAILURE);
}

void warning_ignored_line(int line) {
    printf("WARN: The line %d was ignored because a return statement was already executed\n", line);
}

void error_missing_return(char* method, int type){
    if (type == 0) {
        fprintf(stderr, "ERROR: method %s should return int\n", method);
    } else {
        fprintf(stderr, "ERROR: method %s should return bool\n", method);
    }
    exit(EXIT_FAILURE);
}

void error_return_type(int line, int type, int true_type) {
    char* left_type_string;
    switch (type) {
        case 0:
            left_type_string = "INT";
            break;
        case 1:
            left_type_string = "BOOL";
            break;
        case 2:
            left_type_string = "VOID";
            break;
        default:
            left_type_string = "NULL";
    }
    char* method_return_type_string;
    switch (true_type) {
        case 0:
            method_return_type_string = "INT";
            break;
        case 1:
            method_return_type_string = "BOOL";
            break;
        case 2:
            method_return_type_string = "VOID";
            break;
        default:
            method_return_type_string = "NULL";
    }
    fprintf(stderr, "ERROR(line %d): return type is %s and should be %s \n", line, left_type_string, method_return_type_string);
    exit(EXIT_FAILURE);
}

void error_return_type_void(int line, int true_type) {
    char* method_return_type_string;
    switch (true_type) {
        case 0:
            method_return_type_string = "INT";
            break;
        case 1:
            method_return_type_string = "BOOL";
            break;
        case 2:
            method_return_type_string = "VOID";
            break;
        default:
            method_return_type_string = "NULL";
    }
    fprintf(stderr, "ERROR (line %d)(2) error return type, is VOID and should be %s \n", line, method_return_type_string);
    exit(EXIT_FAILURE);
}

void error_main_missing() {
    fprintf(stderr, "ERROR: main method not is declared\n");
    exit(EXIT_FAILURE);
}