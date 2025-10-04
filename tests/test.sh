#!/bin/bash

# Verifica que el ejecutable parser exista
echo "Verifying parser in current directory"
if [ ! -x ./parser ]; then
    echo "Error: no se encuentra ./parser o no es ejecutable."
    exit 1
fi

i=0

# Carpeta de outputs
mkdir -p tests/output

for file in tests/correct_tests/* tests/error_tests/*; do
    if [ -f "$file" ] && [ "$file" != "tests/test.sh" ]; then
        ((i++))   # incrementa i en bash
        echo ">>> Ejecutando ./parser $file"
        rm -f intermediate_code.out
        if ./parser "$file" > >(tee /tmp/parser_stdout.log) 2> >(tee /tmp/parser_stderr.log >&2); then
            # Solo si el parser terminó correctamente genero el intermedio
            base=$(basename "$file")
            out_file="tests/output/${base}.ic"
            if [ -f intermediate_code.out ]; then
                cp intermediate_code.out "$out_file" 2>/dev/null || true
                if [ -s "$out_file" ]; then
                    echo "[OK] Intermedio generado: $out_file"
                else
                    echo "[WARN] Archivo intermedio vacío tras parseo exitoso: $file"
                fi
            else
                echo "[INFO] No se generó código intermedio (posiblemente no había métodos)."
            fi
        else
            echo "[ERROR] Falló el parseo o análisis: no se genera código intermedio para $file"
        fi
        echo "-----------------------------------"
    fi
done

echo "Tests ejecutados: $i"
