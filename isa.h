#ifndef __ISA_H
#define __ISA_H

#include "memoria.h"

typedef struct registro {
	int binario;
	char *nomeCodigo;
	char tipo;
} Registro;

typedef struct Cop1 {
	int *opcode;
	char *nomeCodigo;
} Cop1;

typedef struct isa {
	Registro **tabela;
	Cop1 **cop1;
	unsigned int instructionCount;
	unsigned int countTypeR;
	unsigned int countTypeI;
	unsigned int countTypeJ;
	unsigned int countTypeFR;
	unsigned int countTypeFI;
	unsigned int countCycles;
	double frequencyPipelined;
	double frequencyMonocycle;
} Isa;

#define CycleTime 1
#define CycleTimeL1 1
#define CycleTimeL2 10
#define CycleTimeRAM 100

extern char PIPELINE; 
extern unsigned int FORWARDING;

Isa* criarISA();
void liberarISA(Isa*);

void printInstrucaoISA(Memoria *, Isa*, unsigned int);
int executarCodigo(Memoria *, Isa *, unsigned int);

Registro* getRegistroOpcode(Isa *, int, char *);
char* getNomeInstrucaoCop1(Isa *, int, int);

#endif