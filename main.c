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
//void yyerror(const char *s);
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
	char* outname = "res"; // Default name
	char* sourcename = NULL;

	if (argc == 1) {
		fprintf(stderr, "Error: Must provide source file.\n");
		return 1;
	}
	if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		printf("\n");
		printf("╭──────────────────────────────────────────────╮\n");
		printf("│      Compilador CTDS — Opciones de uso       │\n");
		printf("╰──────────────────────────────────────────────╯\n\n");

		printf("Uso:\n");
		printf("  %s <archivo.ctds> [opciones]\n\n", argv[0]);

		printf("Opciones:\n");
		printf("  %-22s %s\n", "-h, --help", "Muestra esta ayuda y termina");
		printf("  %-22s %s\n", "-o <archivo>", "Especifica el nombre del archivo de salida (por defecto: res)");
		printf("  %-22s %s\n", "-target <etapa>", "Ejecuta hasta la etapa indicada:");
		printf("                                scan | parse | codinter | assembly\n");
		printf("  %-22s %s\n", "-opt", "Activa optimizaciones del compilador");
		printf("  %-22s %s\n", "-debug", "Muestra el AST y la tabla de símbolos (para depuración)\n");

		printf("Ejemplo de uso:\n");
		printf("  %s -target codinter \n", argv[0]);
		printf("  Genera HASTA el codigo intermedio \n\n");

		printf("Etapas disponibles:\n");
		printf("  scan        Analiza léxicamente el código fuente\n");
		printf("  parse       Construye el AST (árbol de sintaxis)\n");
		printf("  codinter    Genera código intermedio\n");
		printf("  assembly    Genera código ensamblador u objeto\n");
		printf("  executable  Ejecuta el binario final (por defecto, sin flag)\n\n");

		return 0;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-opt") == 0) {
			optimizations = 1;
		} else if (strcmp(argv[i], "-debug") == 0) {
			debug = 1;
		} else if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 < argc) { // If -o is specified, a filename after that is required
				if (argv[i + 1][0] == '-') {
					fprintf(stderr, "Error: -o requires a valid filename.\n");
					return 1;
				}
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
		if (debug) {
			printf("----- SCANNING  -----\n");
		}
		while (yylex() != 0) {
    	
		}
	}

	if (stage > 0 || debug) {
		if (debug) {
			printf("----- PARSING  -----\n");
		}
		yyparse();
		semantic_analyzer(head_ast);
	}
	// Generate intermediate code for each top-level method declaration
	if (stage > 1) {
		reset_code();
		for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
			gen_code(cur->sentence, NULL);
		}
		if (optimizations) {
			optimize_memory(cant_ap_h);
		}
		if (debug) {
			print_temp_list(cant_ap_h);
			char inter_path[128];
			char aux_file[256];
			snprintf(inter_path, sizeof(inter_path), "intermediate_code/%s", outname);
			snprintf(aux_file, sizeof(aux_file), "%s.codinter", inter_path);
			print_code_to_file(aux_file);
		}
	}

	if (debug) {
		if (head_ast != NULL && global_level != NULL) {
			print_full_ast(head_ast);
			print_symbol_table(global_level);
		}
	}

	if (stage > 2 || debug) { 
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
	
	if (stage > 3) {
		char command[256];
		snprintf(command, sizeof(command), "./run_executable.sh object_code/%s", outname);
		system(command);
	}
	return 0;
}

void str_to_lower(char *s) {
    for (int i = 0; s[i]; i++) {
        s[i] = tolower((unsigned char)s[i]);
    }
}