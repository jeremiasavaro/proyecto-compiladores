#!/bin/bash

# Verifica que el ejecutable parser exista
echo "Verifying parser in current directory"
if [ ! -x ./parser ]; then
    echo "Error: no se encuentra ./parser o no es ejecutable."
    exit 1
fi

i=0

# Recorre todos los archivos en tests
mkdir -p tests/output

for file in tests/*/*; do
    if [ -f "$file" ] && [ "$file" != "tests/test.sh" ]; then
        ((i++))   # incrementa i en bash
        echo ">>> Ejecutando ./parser $file"
        ./parser "$file"
        base=$(basename "$file")
        out_file="tests/output/${base}.ic"
        cp intermediate_code.out "$out_file" 2>/dev/null || true
        if [ -s "$out_file" ]; then
            echo "[OK] Intermedio generado: $out_file"
        else
            echo "[WARN] Archivo intermedio vacío para $file"
        fi
        echo "-----------------------------------"
    fi
done

echo "Tests ejecutados: $i"
