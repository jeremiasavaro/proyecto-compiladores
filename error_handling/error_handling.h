#ifndef ERROR_HANDLING
#define ERROR_HANDLING

void error_parse(int line, char* msg);
void error_lexical(int line, char* msg);
int error_open_file(char* filename);

#endif