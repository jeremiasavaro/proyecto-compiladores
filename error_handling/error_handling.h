#ifndef ERROR_HANDLING
#define ERROR_HANDLING

void error_parse(int line, char* msg);
void error_lexical(int line, char* msg);
int error_open_file(char* filename);
void error_variable_redeclaration(int line, char* msg);
void error_variable_not_declared(int line, char* msg);
void error_type_mismatch(int line, char* variable, char* type);
void error_type_mismatch_method(int line, char* method, int type);
void error_allocate_mem();
void error_method_return_data();
void error_method_data();
void error_type_id_unknown();
void error_add_argument_method(char* method_name);
void error_method_not_found(const char* method_name);
void error_null_node(int line);
void error_conditional(int line);
void error_additional(int line);
void error_substraction(int line);
void error_multiplication(int line);
void error_division(int line);
void error_minus(int line);
void error_and(int line);
void error_or(int line);
void error_neg(int line);
void error_assign(int line);
void error_unknown_operator(int line);
void error_variable_used_before_init(int line, char* variable);
void error_id_unknown_type(int line, char* variable);
void error_unknown_leaf_type(int line);
void error_non_existent_id(int line);
void warning_already_returned(int line);
void error_lesser(int line);
void error_greater(int line);
void error_equal(int line);
void error_not_equal(int line);
void error_less_equal(int line);
void error_greater_equal(int line);
void error_different_return_types(int line, char* firstType, char* secondType);
void error_args_number(int line, char* method, int num_args);
void warning_ignored_line(int line);
void error_missing_return(char* method, int type);

#endif