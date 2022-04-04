gcc -c memoria.c -o memoria.o

gcc -c isa.c -o isa.o

gcc -c util.c -o util.o

gcc emulador.c memoria.o isa.o util.o -o emulador.exe