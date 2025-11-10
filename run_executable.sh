# Se debe dar permiso de ejecucion con el siguiente comando en terminal: chmod +x run_executable.sh

OUTNAME="object_code/$1"
gcc -x assembler -c "$OUTNAME.s" -o "$OUTNAME".o
gcc $OUTNAME.o libraries/ctdsio.o -o $OUTNAME.exe
./$OUTNAME.exe