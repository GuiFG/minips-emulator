#!/bin/bash


gcc -c memoria.c -o memoria.o

gcc -c isa.c -o isa.o

gcc -c util.c -o util.o

gcc -c utilcop1.c -o utilcop1.o

gcc -c isadecode.c -o isadecode.o

gcc -c isarun.c -o isarun.o


gcc emulador.c memoria.o isa.o util.o utilcop1.o isadecode.o isarun.o -o emulador
