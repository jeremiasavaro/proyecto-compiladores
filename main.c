#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handling.h"
#include "ast.h"
#include "print_funcs.h"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "intermediate_code.h"
#include "symbol.h"
#include "object_code.h"

extern int yylineno;
extern int yyparse();
void yyerror(const char *s);
extern FILE *yyin;

int main(int argc, char *argv[]) {
	if (argc > 1) {
		FILE *file = fopen(argv[1], "r");
		if (!file) {
			error_open_file(argv[1]);
		}
		yyin = file;
	}

	printf("=== SYNTAX ANALYSIS ===\n");
	yyparse();
	print_full_ast(head_ast);
	print_symbol_table(global_level);
	semantic_analyzer(head_ast);
	// Generate intermediate code for each top-level method declaration
	reset_code();
	for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
		gen_code(cur->sentence, NULL);
	}
	print_code_to_file("intermediate_code/intermediate_code.out");
	return 0;
}
