#!/bin/bash

# Verifica que el ejecutable parser exista
echo "Verifying parser in current directory"
if [ ! -x ./main ]; then
    echo "Error: no se encuentra ./main o no es ejecutable."
    exit 1
fi

TEST_DIR="tests/correct_tests"

if [ "$1" == "error" ]; then
    TEST_DIR="tests/error_tests"
elif [ "$1" == "correct" ]; then
    TEST_DIR="tests/correct_tests"
elif [ -z "$1" ]; then
    TEST_DIR="tests/correct_tests tests/error_tests"
else 
    echo "Error: flag '$1' no reconocido. Solo se permite 'correct', 'error' o ninguno."
    exit 1
fi

i=0

for DIR in $TEST_DIR; do
    for file in "$DIR"/*; do
        if [ -f "$file" ] && [ "$file" != "tests/test.sh" ]; then
            ((i++))
            echo ">>> Parseando $file"
            if ./main "$file" > /dev/null 2> /dev/null; then
                echo "$file" >> "$PARSE_OK_LIST"
                echo "[OK] Parseo exitoso: $file"
            else
                echo "[ERROR] Falló el parseo: $file"
            fi
            echo "-----------------------------------"
        fi
    done
done

if [ "$TEST_DIR" = "tests/correct_tests" ]; then
    # Carpeta de outputs
    mkdir -p tests/output_intermediate_code
    while read file; do
        base=$(basename "$file")
        out_file="tests/output_intermediate_code/${base}.ic"
        echo ">>> Generando código intermedio para $file"
        rm -f intermediate_code/intermediate_code.out
        if ./main "$file" > >(tee /tmp/parser_stdout.log) 2> >(tee /tmp/parser_stderr.log >&2); then
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
            echo "[ERROR] Falló la generación de código intermedio para $file"
        fi
        echo "-----------------------------------"
    done < "$PARSE_OK_LIST"

    OBJ_DIR="tests/output_object_code"
    mkdir -p "$OBJ_DIR"
    for file in tests/correct_tests/*; do
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
        gcc -no-pie "$EXE_DIR/$base.o" libraries/ctdsio.o -o "$exefile"

        if [ -f "$exefile" ]; then
            echo "[OK] Ejecutable generado: $exefile"
        else
            echo "[WARN] No se generó ejecutable para $objfile"
        fi
    done

    declare -A expected # define a array with expected results
    expected["test_methods.ctds"]="7"
    expected["test_types.ctds"]="TRUE"
    expected["test_complex_operators.ctds"]="2"
    expected["test_expresion_calls.ctds"]="24"
    expected["test_integration.ctds"]="1"
    expected["test_operations.ctds"]=""
    expected["test_params.ctds"]="3"
    expected["test_recursive.ctds"]="2"
    expected["test_scopes_shadowing.ctds"]="42"
    expected["test_while_multiple.ctds"]="1"
    expected["test_while.ctds"]="5"

    # Ejecutar todos los ejecutables y guardar resultados en output_final
    RESULTS_FILE="tests/output_final"
    > "$RESULTS_FILE"  # Vacía el archivo si existe

    failures=0

    for exefile in tests/output_executables/*.exe; do
        base=$(basename "$exefile" .exe)
        output=$("$exefile")
        expected_value="${expected[$base]}"
        if [ "$output" != "$expected_value" ]; then
            ((failures++))
            echo "Test: $base esperaba $expected_value y el resultado fue $output" >> "$RESULTS_FILE"
        fi
        ((total++))
    done

    echo "Fallos: $failures"
    echo "Resultados guardados en $RESULTS_FILE"

fi

echo "Tests ejecutados: $i"
