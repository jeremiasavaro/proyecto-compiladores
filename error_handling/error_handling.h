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
void error_return_void();
void error_type_id_unknown();
void error_add_argument_method(char* method_name);
void error_method_not_found(char* method_name);
void error_null_node(int line);
void error_conditional(int line);

#endif