#!/bin/bash

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

# quantity of tests executed
i=0

for DIR in $TEST_DIR; do
    for file in "$DIR"/*; do
        # verify that it's a file and not this script itself
        if [ -f "$file" ] ; then
            ((i++))
            echo ">>> Parseando $file"
            if ./main "$file" -target parse > /dev/null 2> /dev/null; then
                echo "[OK] Parseo exitoso: $file"
            else
                echo "[ERROR] Falló el parseo: $file"
            fi
            echo "-----------------------------------"
        fi
    done
done

if [ "$TEST_DIR" != "tests/error_tests" ]; then
    INTER_DIR="tests/output_intermediate_code"
    mkdir -p "$INTER_DIR"
    
    for file in tests/correct_tests/*; do
        if [ -f "$file" ]; then
            base=$(basename "$file" .ctds)
            out_file="$INTER_DIR/${base}.codinter"
            echo ">>> Generando código intermedio para $file"
            rm -f intermediate_code/*.codinter
            # active debug mode for printing intermediate code generation 
            if ./main "$file" -target codinter -debug > /dev/null 2> /dev/null; then
                ic_generated=$(ls intermediate_code/*.codinter 2>/dev/null | head -1)
                if [ -n "$ic_generated" ] && [ -f "$ic_generated" ]; then
                    cp "$ic_generated" "$out_file"
                    if [ -s "$out_file" ]; then
                        echo "[OK] Intermedio generado: $out_file"
                    else
                        echo "[WARN] Archivo intermedio vacío: $file"
                    fi
                else
                    echo "[INFO] No se generó código intermedio: $file"
                fi
            else
                echo "[ERROR] Falló la generación de código intermedio para $file"
            fi
            echo "-----------------------------------"
        fi
    done

    OBJ_DIR="tests/output_object_code"
    mkdir -p "$OBJ_DIR"

    for file in tests/correct_tests/*; do
        if [ -f "$file" ]; then
            base=$(basename "$file" .ctds)
            objfile="$OBJ_DIR/${base}.s"

            echo ">>> Generando código objeto para $file"
            rm -f object_code/*.s

            ./main "$file" -target assembly > /dev/null 2> /dev/null

            obj_generated=$(ls object_code/*.s 2>/dev/null | head -1)
            
            if [ -n "$obj_generated" ] && [ -f "$obj_generated" ]; then
                cp "$obj_generated" "$objfile"
                echo "[OK] Object code generado: $objfile"
            else
                echo "[WARN] No se generó el codigo objeto para $file"
            fi
            echo "-----------------------------------"
        fi
    done

    EXE_DIR="tests/output_executables"
    mkdir -p "$EXE_DIR"

    for objfile in "$OBJ_DIR"/*.s; do
        if [ -f "$objfile" ]; then
            base=$(basename "$objfile" .s)
            exefile="$EXE_DIR/${base}.exe"
            echo ">>> Generando ejecutable desde $objfile"
            gcc -no-pie "$objfile" libraries/ctdsio.o -o "$exefile" 2>/dev/null
            if [ -f "$exefile" ]; then
                echo "[OK] Ejecutable generado: $exefile"
            else
                echo "[WARN] No se generó ejecutable para $objfile"
            fi
            echo "-----------------------------------"
        fi
    done

    declare -A expected
    expected["test_methods"]="7"
    expected["test_types"]="TRUE"
    expected["test_complex_operators"]="20"
    expected["test_expresion_calls"]="24"
    expected["test_integration"]="1"
    expected["test_operations"]=""
    expected["test_params"]="3"
    expected["test_recursive"]="2"
    expected["test_scopes_shadowing"]="42"
    expected["test_while_multiple"]="1"
    expected["test_while"]="5"

    RESULTS_FILE="tests/output_final"
    > "$RESULTS_FILE"

    failures=0
    total=0

    for exefile in "$EXE_DIR"/*.exe; do
        if [ -f "$exefile" ]; then
            base=$(basename "$exefile" .exe)
            echo ">>> Ejecutando $exefile"
            output=$("$exefile" 2>/dev/null)
            expected_value="${expected[$base]}"
            ((total++))
            if [ "$output" != "$expected_value" ]; then
                ((failures++))
                echo "Test: $base esperaba '$expected_value' y el resultado fue '$output'" >> "$RESULTS_FILE"
                echo "[FAIL] $base: esperado '$expected_value', obtenido '$output'"
            else
                echo "[OK] $base: '$output'"
            fi
            echo "-----------------------------------"
        fi
    done

    echo "Fallos: $failures de $total"
    echo "Resultados guardados en $RESULTS_FILE"
fi

echo "Tests ejecutados: $i"