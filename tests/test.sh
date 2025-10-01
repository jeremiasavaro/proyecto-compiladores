#!/bin/bash

# Verifica que el ejecutable parser exista
echo "Verifying parser in current directory"
if [ ! -x ./parser ]; then
    echo "Error: no se encuentra ./parser o no es ejecutable."
    exit 1
fi

i=0

# Recorre todos los archivos en tests
for file in tests/*/*; do
    if [ -f "$file" ] && [ "$file" != "tests/test.sh" ]; then
        ((i++))   # incrementa i en bash
        echo ">>> Ejecutando ./parser $file"
        ./parser "$file"
        echo "-----------------------------------"
    fi
done

echo "Tests ejecutados: $i"
