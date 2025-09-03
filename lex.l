%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strdup(const char *s);

/* Stats counter */
int num_tokens = 0;
int num_lines = 1;

%}

%option noyywrap
%option yylineno

digit     [0-9]
letter    [a-zA-Z]

%%

{digit}+                 { printf("NUMBER: %s\n", yytext); num_tokens++; return DIG; }

"program"                 { num_tokens++; return PROGRAM;}
"if"                      { num_tokens++; return IF; }
"else"                    { num_tokens++; return ELSE; }
"then"                    { num_tokens++; return THEN;}
"while"                   { num_tokens++; return WHILE; }
"end"                     { num_tokens++; return END; }
"void"                    { num_tokens++; return VOID;}
"return"                  { num_tokens++; return RETURN;}
"extern"                  { num_tokens++; return EXTERN;}
"bool"                    { num_tokens++; return BOOL;}
"integer"                 { num_tokens++; return INTEGER;}
"false"                   { num_tokens++; return FALSE;}
"true"                    { num_tokens++; return TRUE;}


{letter}({letter}|{digit}|_)*  { yylval.sval = strdup(yytext); num_tokens++; return ID; }


"//"({letter}|digit|_)*       { yylval.sval = strdup(yytext); num_tokens++; return INLINECOMMENT; }
"/*"({letter}|digit|_)*"*/"   { yylval.sval = strdup(yytext); num_tokens++; return COMMENT; }

"&&"                   { return AND; }
"||"                   { return OR; }
"!"                    { return NEG; }
[+\-*/%=]              { return *yytext; }
[;(){}]                { return *yytext; }
[ \t\r\n]+             { }
.                      { fprintf(stderr, "Lexical ERROR(line %d): unknown character '%s'\n", yylineno, yytext); exit(EXIT_FAILURE); }

%%

int main(int argc, char *argv[]) {
    FILE *archive;
    int token;

    if (argc > 1) {
        archive = fopen(argv[1], "r");
        if (!archive) {
            printf("Error: Cant open the provided file %s\n", argv[1]);
            return 1;
        }
        yyin = archive;
    } else {
        printf("Press (Ctrl+D for shut down):\n");
        yyin = stdin;
    }

    printf("\n=== LEXICAL ANALYSIS ===\n");

    while ((token = yylex()) != 0) {
    }

    printf("\n=== STATS ===\n");
    printf("Recognized tokens: %d\n", num_tokens);
    printf("Proccesed lines: %d\n", num_lines);

    if (argc > 1) {
        fclose(archive);
    }
    return 0;
}