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
#include <ctype.h>

extern int yylineno;
extern int yyparse();
void yyerror(const char *s);
extern FILE *yyin;

typedef enum STAGE {
	SCAN,
	PARSE,
	CODINTER,
	ASSEMBLY,
	EXECUTABLE
} STAGE;

int main(int argc, char *argv[]) {
	// Flags
	int optimizations = 0; // TODO: Handle different optimizations
	int debug = 0;
	STAGE stage = EXECUTABLE; // Run all stages by default
	char* outname = "nombreArchivo.out"; // Default name
	char* sourcename = NULL;

	if (argc == 1) {
		fprintf(stderr, "Error: Must provide source file.\n");
		return 1;
	}

	// TODO: implement a -h flag to print help information about the compilation proccess

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-opt") == 0) {
			optimizations = 1;
		} else if (strcmp(argv[i], "-debug") == 0) {
			debug = 1;
		} else if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 < argc) { // If -o is specified, a filename after that is required
				outname = argv[i + 1];
				i++;
			} else {
				fprintf(stderr, "Error: -o requires a filename.\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-target") == 0) {
			if (i + 1 < argc) { // If -target is specified, a target stage after that is required
				str_to_lower(argv[i + 1]);
				if (strcmp(argv[i + 1], "scan") == 0) {
					stage = SCAN;
				} else if (strcmp(argv[i + 1], "parse") == 0) {
					stage = PARSE;
				} else if (strcmp(argv[i + 1], "codinter") == 0) {
					stage = CODINTER;
				} else if (strcmp(argv[i + 1], "assembly") == 0) {
					stage = ASSEMBLY;
				} else {
					fprintf(stderr, "Error: unknown stage specifier after -target.\n");
					return 1;
				}
				i++;
			} else {
				fprintf(stderr, "Error: -target requires a target stage.\n");
				return 1;
			}
		} else {
			if (sourcename == NULL) {
				sourcename = argv[i];
			} else {
				fprintf(stderr, "Warning: Multiple source files specified. Using first one: %s\n", sourcename);
			}
		}
	}

	if (sourcename == NULL) {
		fprintf(stderr, "Error: No source file provided.\n");
		return 1;
	}

	FILE *file = fopen(sourcename, "r");
	if (!file) {
		error_open_file(argv[1]);
	}
	yyin = file;

	if (stage == 0) {
		printf("=== SYNTAX ANALYSIS ===\n");
		while (yylex() != 0) {
		}		
		printf("No syntactic errors.\n");
	}

	if (stage > 0) {
		printf("=== SYNTAX AND SEMANTIC ANALYSIS ===\n");
		yyparse();
		semantic_analyzer(head_ast);
	}
	// Generate intermediate code for each top-level method declaration
	if (stage > 1) {
		reset_code();
		for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
			gen_code(cur->sentence, NULL);
		}
		print_code_to_file("intermediate_code/intermediate_code.out");
	}
	if (stage > 2) { 
		FILE* out = fopen(outname, "w");
		if (!out) {
			error_open_file(outname);
		}
		generate_object_code(out);
		fclose(out);
	}
	if (stage > 3) {
		system("./run_executable.sh");
	}
	if (debug) {
		if (head_ast != NULL && global_level != NULL) {
			print_full_ast(head_ast);
			print_symbol_table(global_level);
		}
	}
	return 0;
}

void str_to_lower(char *s) {
    for (int i = 0; s[i]; i++) {
        s[i] = tolower((unsigned char)s[i]);
    }
}