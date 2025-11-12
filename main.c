#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handling.h"
#include "ast.h"
#include "print_funcs.h"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "intermediate_code.h"
#include "optimization.h"
#include "symbol.h"
#include "object_code.h"
#include <ctype.h>

extern int yylineno;
extern int yyparse();
extern FILE *yyin;
extern int yylex();

typedef enum STAGE {
	SCAN,
	PARSE,
	CODINTER,
	ASSEMBLY,
	EXECUTABLE
} STAGE;

int optimizations = 0;
int debug = 0;

void str_to_lower(char *s);

int main(int argc, char *argv[]) {
	// Flags
	STAGE stage = EXECUTABLE; // Run all stages by default
	char* outname = "out"; // Default name
	char* sourcename = NULL;

	if (argc == 1) {
		fprintf(stderr, "Error: Must provide source file. See \"ctds -h\" for usage help.\n");
		return 1;
	}
	if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0)) {
		printf("\n");
		printf("╭──────────────────────────────────────────────╮\n");
		printf("│         CTDS Compiler — Usage options        │\n");
		printf("╰──────────────────────────────────────────────╯\n\n");

		printf("Usage:\n");
		printf("  %s <file.ctds> [options]\n\n", argv[0]);

		printf("Options:\n");
		printf("  %-22s %s\n", "-h, -help", "Shows this help message");
		printf("  %-22s %s\n", "-o <file>", "Specifies the name of the output file (default: out)");
		printf("  %-22s %s\n", "-t, -target <stage>", "Run until the indicated stage: scan | parse | codinter | assembly | executable (default: executable)");
		printf("  %-22s %s\n", "-opt", "Enable compiler optimizations");
		printf("  %-22s %s\n", "-d, -debug", "Shows debugging information (AST structure, lexer tokens, intermediate code, etc.)\n");

		printf("Use example:\n");
		printf("  %s -target codinter \n", argv[0]);
		printf("  Generate up to the intermediate code \n\n");

		printf("Available stages:\n");
		printf("  scan        Lexically parse the source code\n");
		printf("  parse       Build the AST (abstract syntax tree)\n");
		printf("  codinter    Generate intermediate code\n");
		printf("  assembly    Generate assembly or object code\n");
		printf("  executable  Generate the executable\n\n");

		return 0;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-opt") == 0) {
			optimizations = 1;
		} else if (strcmp(argv[i], "-debug") == 0 || strcmp(argv[i], "-d") == 0) {
			debug = 1;
		} else if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 < argc) { // If -o is specified, a filename after that is required
				if (argv[i + 1][0] == '-') {
					fprintf(stderr, "Error: -o requires a valid filename. See ctds -h for usage help.\n");
					return 1;
				}
				outname = argv[i + 1];
				i++;
			} else {
				fprintf(stderr, "Error: -o requires a filename. See ctds -h for usage help.\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-target") == 0 || strcmp(argv[i], "-t") == 0) {
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
					fprintf(stderr, "Error: unknown stage specifier after -target. See ctds -h for usage help.\n");
					return 1;
				}
				i++;
			} else {
				fprintf(stderr, "Error: -target requires a target stage. See ctds -h for usage help.\n");
				return 1;
			}
		} else {
			if (argv[i][0] == '-') {
				fprintf(stderr, "Error: unknown or misused flag. See ctds -h for usage help.\n");
				return 1;
			}
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

	if (stage == SCAN) {
		if (debug) {
			printf("\n----- SCANNING -----\n");
		}
		while (yylex() != 0) {
    	
		}
	}

	if (stage > SCAN || debug) {
		if (debug && stage > SCAN) {
			printf("----- PARSING -----\n");
		}
		if (stage > SCAN) {
			yyparse();
			semantic_analyzer(head_ast);
		}
	}
	// Generate intermediate code for each top-level method declaration
	if (stage > PARSE) {
		reset_code();
		for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
			gen_code(cur->sentence, NULL);
		}
		if (debug) {
			print_temp_list(cant_ap_h); // Print temp lists before optimizations
		}
		if (optimizations) {
			optimize_memory(cant_ap_h);
		}
		if (debug || stage == CODINTER) {
			char inter_path[128];
			char aux_file[256];
			snprintf(inter_path, sizeof(inter_path), "intermediate_code/%s", outname);
			snprintf(aux_file, sizeof(aux_file), "%s.codinter", inter_path);
			print_code_to_file(aux_file);
			printf("\nIntermediate code dumped in %s \n", aux_file);
		}
	}

	if (debug) {
		if (head_ast != NULL && global_level != NULL) {
			print_full_ast(head_ast);
			print_symbol_table(global_level);
		}
	}

	if (stage > CODINTER || debug) {
		char inter_path[256];
		char aux_file[256];
		snprintf(inter_path, sizeof(inter_path), "object_code/%s", outname);
		snprintf(aux_file, sizeof(aux_file), "%s.s", inter_path);
		FILE* out = fopen(aux_file, "w");
		if (!out) {
			error_open_file(aux_file);
		}
		generate_object_code(out, cant_ap_h);
		fclose(out);
	}
	
	if (stage > ASSEMBLY) {
		char command[256];
		snprintf(command, sizeof(command), "./link.sh object_code/%s", outname);
		system(command);
	}
	return 0;
}

void str_to_lower(char *s) {
    for (int i = 0; s[i]; i++) {
        s[i] = tolower((unsigned char)s[i]);
    }
}