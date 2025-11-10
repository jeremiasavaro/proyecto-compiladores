#!/bin/bash

echo "Verifying ctds in current directory"
if [ ! -x ./ctds ]; then
    echo "Error: ./ctds cannot be found or is not executable."
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
    echo "Error: flag '$1' not recognized. Only 'correct', 'error' or none are allowed."
    exit 1
fi

# amount of tests executed
i=0

for DIR in $TEST_DIR; do
    for file in "$DIR"/*; do
        # verify that it's a file and not this script itself
        if [ -f "$file" ] ; then
            ((i++))
            echo ">>> Parsing $file"
            if ./ctds "$file" -target parse > /dev/null 2> /dev/null; then
                echo "[OK] Successful parse: $file"
            else
                echo "[ERROR] Parsing failed: $file"
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
            echo ">>> Generating intermediate code for $file"
            rm -f intermediate_code/*.codinter
            # active debug mode for printing intermediate code generation
            if ./ctds "$file" -target codinter -debug > /dev/null 2> /dev/null; then
                ic_generated=$(ls intermediate_code/*.codinter 2>/dev/null | head -1)
                if [ -n "$ic_generated" ] && [ -f "$ic_generated" ]; then
                    cp "$ic_generated" "$out_file"
                    if [ -s "$out_file" ]; then
                        echo "[OK] Intermediate code generated: $out_file"
                    else
                        echo "[WARN] Empty intermediate code file: $file"
                    fi
                else
                    echo "[INFO] No intermediate code generated: $file"
                fi
            else
                echo "[ERROR] Intermediate code generation failed for $file"
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

            echo ">>> Generating object code for $file"
            rm -f object_code/*.s

            ./ctds "$file" -target assembly > /dev/null 2> /dev/null

            obj_generated=$(ls object_code/*.s 2>/dev/null | head -1)

            if [ -n "$obj_generated" ] && [ -f "$obj_generated" ]; then
                cp "$obj_generated" "$objfile"
                echo "[OK] Object code generated: $objfile"
            else
                echo "[WARN] No object code generated for $file"
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
            echo ">>> Generating executable from $objfile"
            gcc -no-pie "$objfile" libraries/ctdsio.o -o "$exefile" 2>/dev/null
            if [ -f "$exefile" ]; then
                echo "[OK] Generated executable: $exefile"
            else
                echo "[WARN] No executable generated for $objfile"
            fi
            echo "-----------------------------------"
        fi
    done

    # Use indexed arrays instead of associative arrays
    expected_keys=(test_methods test_types test_complex_operators test_expresion_calls test_integration test_operations test_params test_recursive test_scopes_shadowing test_while_multiple test_while)
    expected_values=(7 TRUE 2 24 1 "" 3 2 42 1 5)

    expected_value_for() {
        local key="$1"
        local i
        for i in "${!expected_keys[@]}"; do
            if [ "${expected_keys[$i]}" = "$key" ]; then
                printf '%s' "${expected_values[$i]}"
                return
            fi
        done
        printf ''
    }

    RESULTS_FILE="tests/output_final"
    > "$RESULTS_FILE"

    failures=0
    total=0

    for exefile in "$EXE_DIR"/*.exe; do
        if [ -f "$exefile" ]; then
            base=$(basename "$exefile" .exe)
            echo ">>> Executing $exefile"
            output=$("$exefile" 2>/dev/null)
            expected_value="$(expected_value_for "$base")"
            ((total++))
            if [ "$output" != "$expected_value" ]; then
                ((failures++))
                echo "Test: $base expected '$expected_value' and the result was '$output'" >> "$RESULTS_FILE"
                echo "[FAIL] $base: expected '$expected_value', obtained '$output'"
            else
                echo "[OK] $base: '$output'"
            fi
            echo "-----------------------------------"
        fi
    done

    echo "Failures: $failures of $total"
    echo "Results saved in $RESULTS_FILE"
fi

echo "Tests executed: $i"