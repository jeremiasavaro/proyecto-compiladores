#!/bin/bash

# Verifica que el ejecutable parser exista
if [ ! -x ./parser ]; then
    echo "Error: no se encuentra ./parser o no es ejecutable."
    exit 1
fi

# Verifica que exista la carpeta tests
if [ ! -d tests ]; then
    echo "Error: no existe la carpeta 'tests'."
    exit 1
fi

# Recorre todos los archivos en tests
for file in tests/*; do
    if [ -f "$file" ]; then
        echo ">>> Ejecutando ./parser $file"
        ./parser "$file"
        echo "-----------------------------------"
    fi
done
