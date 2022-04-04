#ifndef __ISA_H
#define __ISA_H

#include "memoria.h"

typedef struct registro {
	char *binario;
	char *nomeCodigo;
	char tipo;
} Registro;


typedef struct isa {
	Registro **tabela;
} Isa;


Isa* criarISA();
void liberarISA(Isa*);

void printInstrucaoISA(Isa*, char*);
int executarCodigo(Memoria *, Isa *, char*);

void imprimirISA(Isa*);

#endif