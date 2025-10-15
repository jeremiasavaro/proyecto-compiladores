#ifndef OBJECT_CODE_H
#define OBJECT_CODE_H

#include <stdio.h>
#include "intermediate_code.h"
#include "symbol.h"

/* Mapa simple de temporales a offsets en el stack */
typedef struct {
    char* temp_name;
    int stack_offset;
} TempMap;

/* Función principal que genera código assembly x86-64 desde código intermedio */
void generate_assembly(const char* output_filename);

/* Funciones auxiliares para traducir instrucciones específicas */
void translate_loadval(FILE* f, INFO* var1, INFO* reg);
void translate_add(FILE* f, INFO* var1, INFO* var2, INFO* reg);

#endif