OUTNAME="$1"
gcc -x assembler -c "$OUTNAME".s -o "$OUTNAME".o
EXE_NAME="$(basename "$OUTNAME")"
gcc "$OUTNAME".o libraries/ctdsio.o -o "$EXE_NAME"