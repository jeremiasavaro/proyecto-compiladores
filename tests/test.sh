#!/bin/bash

# Verifica que el ejecutable parser exista
echo "Verifying parser in current directory"
if [ ! -x ./main ]; then
    echo "Error: no se encuentra ./main o no es ejecutable."
    exit 1
fi

i=0

# Carpeta de outputs
mkdir -p tests/output_intermediate_code

for file in tests/correct_tests/* tests/error_tests/*; do
    if [ -f "$file" ] && [ "$file" != "tests/test.sh" ]; then
        ((i++))   # incrementa i en bash
        echo ">>> Ejecutando ./main $file"
        rm -f intermediate_code/intermediate_code.out
        if ./main "$file" > >(tee /tmp/parser_stdout.log) 2> >(tee /tmp/parser_stderr.log >&2); then
            # Solo si el parser terminó correctamente genero el intermedio
            base=$(basename "$file")
            out_file="tests/output_intermediate_code/${base}.ic"
            if [ -f intermediate_code/intermediate_code.out ]; then
                cp intermediate_code/intermediate_code.out "$out_file" 2>/dev/null || true
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

OBJ_DIR="tests/output_object_code"
mkdir -p "$OBJ_DIR"

for file in tests/correct_tests/* tests/error_tests/*; do
    if [ -f "$file" ] && [ "$file" != "tests/test.sh" ]; then
        base=$(basename "$file")
        objfile="$OBJ_DIR/$base.out"

        rm -f res.out

        ./main "$file"

        if [ -f res.out ]; then
            cp res.out "$objfile"
            echo "[OK] Object code generado: $objfile"
        else
            echo "[WARN] No se generó el codigo objeto para $file"
        fi
    fi
done

EXE_DIR="tests/output_executables"
mkdir -p "$EXE_DIR"

for objfile in tests/output_object_code/*.out; do
    base=$(basename "$objfile" .out)
    exefile="$EXE_DIR/$base.exe"

    gcc -x assembler -c "$objfile" -o "$EXE_DIR/$base.o"
    gcc -no-pie "$EXE_DIR/$base.o" -o "$exefile"

    if [ -f "$exefile" ]; then
        echo "[OK] Ejecutable generado: $exefile"
    else
        echo "[WARN] No se generó ejecutable para $objfile"
    fi
done

# Ejecutar todos los ejecutables y guardar resultados en output_final
RESULTS_FILE="tests/output_final"
> "$RESULTS_FILE"  # Vacía el archivo si existe

for exefile in tests/output_executables/*.exe; do
    base=$(basename "$exefile" .exe)
    output=$("$exefile")
    echo "${base^} = $output" >> "$RESULTS_FILE"
done

echo "Tests ejecutados: $i"
echo "Resultados guardados en $RESULTS_FILE"
