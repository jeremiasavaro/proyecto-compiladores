#ifndef ERROR_HANDLING
#define ERROR_HANDLING

void error_parse(int line, char* msg);
void error_lexical(int line, char* msg);
int error_open_file(char* filename);
void error_variable_redeclaration(int line, char* msg);
void error_variable_not_declared(int line, char* msg);
void error_type_mismatch(int line, char* msg, char* type);
void error_allocate_mem();
void error_method_return_data();
void error_method_data();
void error_type_id_unknown();
void error_add_argument_method(char* method_name);
void error_method_not_found(char* method_name);
void error_null_node(int line);
void error_conditional(int line);
void additional_error(int line);
void substraction_error(int line);
void multiplication_error(int line);
void division_error(int line);
void division_zero_error(int line);
void minus_error(int line);
void and_error(int line);
void or_error(int line);
void neg_error(int line);
void assign_error(int line);
void error_return_int(int line);
void error_return_bool(int line);
void error_return_void(int line);
void error_unespected_return_int(int line);
void error_unespected_return_bool(int line);
void error_unknown_operator(int line);
void error_variable_used_before_init(int line, char* variable);
void error_id_unknown_type(int line, char* variable);
void error_unknown_leaf_type(int line);
void error_noexistent_id(int line);
void warning_already_returned(int line);
void error_missing_return(int line);

#endif