# Se debe dar permiso de ejecucion con el siguiente comando en terminal: chmod +x run_executable.sh

gcc -x assembler -c res.out -o res.o
gcc res.o libraries/ctdsio.o -o res.exe
./res.exe