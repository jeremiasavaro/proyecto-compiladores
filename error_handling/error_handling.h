#ifndef ERROR_HANDLING
#define ERROR_HANDLING

void error_parse(int line, char* msg);
void error_lexical(int line, char* msg);
int error_open_file(char* filename);
void error_variable_redeclaration(int line, char* msg);
void error_variable_not_declared(int line, char* msg);
void error_type_mismatch(int line, char* msg, char* type);
void error_allocate_mem();

#endif