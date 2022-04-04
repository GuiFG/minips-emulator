#include "isa.h"
#include "isadecode.h"
#include "isarun.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int binarios[] = {
	0x20, 0x21, 0x8, 0x9, 0x23,
	0xf, 0x23, 0x2b, 0x28, 0x20, 0x24,
	0xd, 0x25, 0x26, 0xc, 0x24,
	0x4, 0x5, 0x6, 0x1,
	0x2, 0x3, 0x8, 0x9,
	0x2a, 0xa, 0x2b,
	0x2, 0x0,
	0x3
};

char *codigos[] = {
	"add", "addu", "addi", "addiu", "subu",
	"lui", "lw", "sw", "sb", "lb", "lbu",
	"ori", "or", "xor", "andi", "and",
	"beq", "bne", "blez", "bgez",
	"j", "jal", "jr", "jalr",
	"slt", "slti", "sltu",
	"srl", "sll",
	"sra"
};

char *tipos = "RRIIRIIIIIIIRRIRIIIIJJRRRIRRRR";

#define TAMANHO_TABELA (sizeof (binarios) / sizeof(int))
#define TAMANHO_COP1 22

int opcodeCop[TAMANHO_COP1][2] = {
	{0x11, 0xfe}, {0x11, 0x4},
	{0x12, 0}, {0x10, 0},
	{0x18, 0}, {0x1a, 0},
	{0x31, 0}, {0x35, 0},
	{0x39, 0}, 
	{0x11, 0x6}, {0x11, 0x6},
	{0x11, 0x21}, {0x11, 0x20},
	{0x11, 0x3}, {0x11, 0x2}, {0x11, 0x2},
	{0x11, 0x0}, {0x11, 0x0}, {0x11, 0x1},
	{0x11, 0xff},
	{0x11, 0x8}, {0x11, 0x8}
};

char *codCop[] = {
	"mfc1", "mtc1",
	"mflo", "mfhi",
	"mult", "div",
	"lwc1", "ldc1",
	"swc1", 
	"mov.d", "mov.s",
	"cvt.d.w", "cvt.s.d",
	"div.d", "mul.s", "mul.d",
	"add.d", "add.s", "sub.s",
	"c.cond.fmt",
	"bc1f", "bc1t"
};

char PIPELINE = 0;
unsigned int FORWARDING = 0;

extern char *nomeInstrucao;

char TRACE;

Registro* criarRegistro(int binario, char *nomeCodigo, char tipo)
{
	Registro *reg = (Registro *) malloc(sizeof(Registro));
	reg->binario = binario;
	reg->nomeCodigo = nomeCodigo;
	reg->tipo = tipo;

	return reg;
}

Cop1* criarInstrucaoCop1(int *opcode, char *nomeCodigo)
{
	Cop1 *cop1 = (Cop1 *) malloc(sizeof(Cop1));
	cop1->opcode = opcode;
	cop1->nomeCodigo = nomeCodigo;

	return cop1;
}

Isa* criarISA()
{
	Isa *isa = (Isa *) malloc(sizeof(Isa));
	isa->tabela = (Registro **) malloc(sizeof(*isa->tabela) * TAMANHO_TABELA);

	int i = 0;

	while (i < TAMANHO_TABELA)
	{
		isa->tabela[i] = criarRegistro(binarios[i], codigos[i], tipos[i]);
		i += 1;
	}

	isa->cop1 = (Cop1 **) malloc(sizeof(*isa->cop1) * TAMANHO_COP1);
	
	i = 0;
	while (i < TAMANHO_COP1)
	{
		isa->cop1[i] = criarInstrucaoCop1(opcodeCop[i], codCop[i]);
		i += 1;
	}

	isa->instructionCount = 0;
	isa->countTypeR = 0;
	isa->countTypeI = 0;
	isa->countTypeJ = 0;
	isa->countCycles = 0;
	isa->countTypeFR = 0;
	isa->countTypeFI = 0;
	isa->frequencyPipelined = 33.8688 * 1000000;
	isa->frequencyMonocycle = 8.4672 * 1000000;

	return isa;
}

Registro* getRegistroOpcode(Isa *isa, int opcode, char *tipo)
{
	int i;

	for (i = 0; i < TAMANHO_TABELA; i++)
	{
		Registro *reg = isa->tabela[i];
		if (reg->binario == opcode && strchr(tipo, reg->tipo))
			return reg;
	}

	return NULL;
}

char* getNomeInstrucaoCop1(Isa *isa, int opcode, int func)
{
	int i;
	for (i = 0; i < TAMANHO_COP1; i++)
	{
		Cop1 *cop1 = isa->cop1[i];
		
		if (cop1->opcode[0] == opcode && cop1->opcode[1] == func)
			return cop1->nomeCodigo;
	}

	return NULL;
}

int isInstrucaoCop1(Isa *isa, int opcode)
{
	int i;
	for (i = 0; i < TAMANHO_COP1; i++)
	{
		Cop1 *aux = isa->cop1[i];

		if (aux->opcode[0] == opcode)
			return 1;
	}

	return 0;
}

int getTipoInstrucao(Isa *isa, unsigned int binario)
{
	int op = binario >> 26;

	int correto = 0;
	if (op == 0)
	{
		op = binario & 0x3f;
		if (!isInstrucaoCop1(isa, op))
			return 1;
	}

	if (isInstrucaoCop1(isa, op))
		return 4;

	Registro *reg = NULL;
	reg = getRegistroOpcode(isa, op, "IJ");	

	if (reg != NULL)
	{
		if (reg->tipo == 'I')
			return 2;
	
		return 3;
	}

	return 0;	
}

void setNomeInstrucao(Isa *isa, int tipoInstrucao, unsigned int binario)
{
	switch(tipoInstrucao)
	{
		case 1:
			setNomeTipoR(isa, binario);
			break;
		case 2:
			setNomeTipoI(isa, binario);
			break;
		case 3:
			setNomeTipoJ(isa, binario);
			break;
		case 4:
			setNomeTipoCop1(isa, binario);
			break;
		default:
			return;
	}
}

void printInstrucaoISA(Memoria *memoria, Isa *isa, unsigned int binario)
{
	unsigned int pc = memoria->registradores[32].valor;
	printf("%08x:\t", pc);

	int tipoInstrucao = getTipoInstrucao(isa, binario);

	switch(tipoInstrucao)
	{
		case 1:
			printCodigoTipoR(isa, binario);
			break;
		case 2:
			printCodigoTipoI(isa, binario);
			break;
		case 3:
			printCodigoTipoJ(isa, binario, pc);
			break;
		case 4:
			printCodigoCop1(isa, binario);
			break;
		default:
			printf("Não possui instrução na ISA.\n");
			return;
	}

	if (!RUNDEC)
		memoria->registradores[32].valor += ALINHAMENTO;
}

int executarCodigo(Memoria *memoria, Isa *isa, unsigned int binario)
{
	
	if (RUNDEC)
		printInstrucaoISA(memoria, isa, binario);

	isa->instructionCount += 1;

	
	int tipoInstrucao = getTipoInstrucao(isa, binario);
	setNomeInstrucao(isa, tipoInstrucao, binario);

	int correto = 0;
	switch(tipoInstrucao)
	{
		case 1:
			isa->countTypeR += 1;

			correto = runCodigoTipoR(memoria, isa, binario);
			break;
		case 2:
			isa->countTypeI += 1;

			correto = runCodigoTipoI(memoria, binario, &isa->countCycles);
			break;
		case 3:
			isa->countTypeJ += 1;

			correto = runCodigoTipoJ(memoria, binario);
			break;
		case 4:
			correto = runCodigoCop1(memoria, isa, binario);
			break;
		default:
			return 0;
	}

	if (correto)
	{
		isa->countCycles += CycleTime;
			
		if (PIPELINE == 0 && FORWARDING != 0)
		{
			PIPELINE = 1;
		}
		else if (PIPELINE == 1)
		{
			memoria->registradores[32].valor = FORWARDING;
			FORWARDING = 0;
			PIPELINE = 0;
			return 1;
		}
		
		memoria->registradores[32].valor += ALINHAMENTO;
	} 

	if (DEBUG)
		LEVEL = 1;
		
	return correto;
}

void liberarISA(Isa *isa)
{
	int i;
	for (i = 0; i < TAMANHO_TABELA; i++)
		free(isa->tabela[i]);
	
	for (i = 0; i < TAMANHO_COP1; i++)
		free(isa->cop1[i]);
	
	free(isa->tabela);
	free(isa->cop1);
	free(isa);
}