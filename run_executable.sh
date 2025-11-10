# Se debe dar permiso de ejecucion con el siguiente comando en terminal: chmod +x run_executable.sh

OUTNAME="$1"
gcc -x assembler -c "$OUTNAME".s -o "$OUTNAME".o
EXE_NAME="$(basename "$OUTNAME").exe"
gcc "$OUTNAME".o libraries/ctdsio.o -o "$EXE_NAME"
"./$EXE_NAME"