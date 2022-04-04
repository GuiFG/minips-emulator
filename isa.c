#include "isa.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *binarios[] = {
	"100000", "100001", "001000", "001001",
	"001111", "100011", "101011",
	"001101", "001100",
	"000100", "000101",
	"000010", "000011", "001000",
	"101010", "000010", "000000"
};

char *codigos[] = {
	"add", "addu", "addi", "addiu",
	"lui", "lw", "sw",
	"ori", "andi",
	"beq", "bne",
	"j", "jal", "jr",
	"slt", "srl", "sll"
};

char *tipos = "RRIIIIIIIIIJJRRRR";

#define TAMANHO_TABELA (sizeof (binarios) / sizeof(char *))

Registro* criarRegistro(char *binario, char *nomeCodigo, char tipo)
{
	Registro *reg = (Registro *) malloc(sizeof(Registro));
	reg->binario = binario;
	reg->nomeCodigo = nomeCodigo;
	reg->tipo = tipo;

	return reg;
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

	return isa;
}

Registro *getRegistroOpcode(Isa *isa, char *opcode, char *tipo)
{
	int i;
	for (i = 0; i < TAMANHO_TABELA; i++)
	{
		Registro *reg = isa->tabela[i];
		if (!strcmp(reg->binario, opcode) && strchr(tipo, reg->tipo))
			return reg;
	}

	return NULL;
}

void printCodgioTipoR(Isa *isa, char *binario)
{
	char opcode[7];
	substring(binario, 0, 6, opcode); opcode[6] = 0;

	char rs[6];
	substring(binario, 6, 5, rs); rs[5] = 0;

	char rt[6];
	substring(binario, 11, 5, rt); rt[5] = 0;

	char rd[6];
	substring(binario, 16, 5, rd); rd[5] = 0;

	char shamt[6];
	substring(binario, 21, 5, shamt); shamt[5] = 0;

	char funct[7];
	substring(binario, 26, 6, funct); funct[6] = 0;

	if (convertBinToInt(funct) == 12)
	{
		printf("syscall\n");
		return;
	}

	Registro *reg = getRegistroOpcode(isa, funct, "R");
	if (!reg)
	{
		printf("Não tem registro na ISA.\t%s\n", funct);
		return;
	}
	char *nomeFunct = reg->nomeCodigo;

	char *nomeRs = getNomeRegistrador((int) convertBinToInt(rs));
	char *nomeRt = getNomeRegistrador((int) convertBinToInt(rt));
	char *nomeRd = getNomeRegistrador((int) convertBinToInt(rd));


	if (!strcmp(nomeFunct, "srl") || !strcmp(nomeFunct, "sll"))
	{
		int numeroShamt = (int) convertBinToInt(shamt);
		printf("%s %s, %s, 0x%08x\n", nomeFunct, nomeRd, nomeRt, numeroShamt);
	}
	else if (!strcmp(nomeFunct, "jr"))
		printf("%s %s\n", nomeFunct, nomeRs);
	else
		printf("%s %s, %s, %s\n", nomeFunct, nomeRd, nomeRs, nomeRt);
}

void printCodigoTipoI(Isa *isa, char *binario)
{
	char opcode[7];
	substring(binario, 0, 6, opcode); opcode[6] = 0;

	Registro *reg = getRegistroOpcode(isa, opcode, "I");
	if (!reg)
	{
		printf("Não tem registro na ISA.\t%s\n", opcode);
		return;
	}
	char *nomeOpcode = reg->nomeCodigo;

	char rs[6];
	substring(binario, 6, 5, rs); rs[5] = 0;

	char rt[6];
	substring(binario, 11, 5, rt); rt[5] = 0;

	char imediato[17];
	substring(binario, 16, 16, imediato); imediato[16] = 0;

	char *nomeRs = getNomeRegistrador((int) convertBinToInt(rs));

	char *nomeRt = getNomeRegistrador((int) convertBinToInt(rt));

	int numeroImediato = convertBinToInt(imediato);

	if (!strcmp(nomeOpcode, "lui"))
	{
		printf("%s %s, 0x%08x\n", nomeOpcode, nomeRt, numeroImediato);
		return;
	}


	setNumeroSignedInt(&numeroImediato);
	
	if (!strcmp(nomeOpcode, "lw") || !strcmp(nomeOpcode, "sw"))
	{
		printf("%s %s, 0x%08x(%s)\n", nomeOpcode, nomeRt, numeroImediato, nomeRs);
		return;
	}

	printf("%s %s, %s, 0x%08x\n", nomeOpcode, nomeRt, nomeRs, numeroImediato);
}

void printCodigoTipoJ(Isa *isa, char *binario)
{
	char opcode[7];
	substring(binario, 0, 6, opcode); opcode[6] = 0;

	Registro *reg = getRegistroOpcode(isa, opcode, "J");
	if (!reg)
	{
		printf("Não tem registro na ISA.\t%s\n", opcode);
		return;
	}

	char *nomeOpcode = reg->nomeCodigo;

	char imediato[27];
	substring(binario, 6, 26, imediato); imediato[26] = 0;

	unsigned long long im = convertBinToInt(imediato);

	im = im << 2;

	printf("%s 0x%08llx\n", nomeOpcode, im);
}

void printInstrucaoISA(Isa *isa, char *binario)
{
	char opcode[7];
	substring(binario, 0, 6, opcode); opcode[6] = 0;

	int op = (int) convertBinToInt(opcode);

	if (op == 0)
	{
		printCodgioTipoR(isa, binario);
		return;
	}


	Registro *reg = getRegistroOpcode(isa, opcode, "IJ");
	if (!reg)
	{
		printf("Não possui registro na ISA.\t%s\n", opcode);
		return;
	}

	if (reg->tipo == 'I')
		printCodigoTipoI(isa, binario);
	else
		printCodigoTipoJ(isa, binario);
}

void imprimirISA(Isa *isa)
{
	int i;
	printf("Binario\tCodigo\tTipo\n");
	for (i = 0; i < TAMANHO_TABELA; i++)
		printf("%s\t %s\t %c\n", isa->tabela[i]->binario, isa->tabela[i]->nomeCodigo, isa->tabela[i]->tipo);
}

int executaSyscall(Memoria *memoria)
{
	unsigned long int v0 = memoria->registradores[2].valor;

	if (v0 == 1)
	{
		printf("%ld", memoria->registradores[4].valor);
	}
	else if (v0 == 4)
	{
		unsigned long int a0 = memoria->registradores[4].valor;

		int offset = 1;

		int lendo = 1;
		while (lendo)
		{
			// printf("a0: 0x%08lx\n", a0);
			int *codigo = getDataCodigo(memoria, a0);
			if (!codigo)
			{
				printf("Endereco de memoria invalido.\n");
				return 0;
			}
			// imprimirVetor2(codigo, ALINHAMENTO);

			if (offset)
				offset = (a0 % ENDERECO_DATA) % ALINHAMENTO;


			int i;
			for (i = ALINHAMENTO - 1 - offset; i >= 0; i--)
			{
				if (codigo[i] != 0)
					printf("%c", codigo[i]);
					// lendo = 1;
				else
				{
					lendo = 0;
					break;
				}
			}

			a0 += ALINHAMENTO - offset;
			offset = 0;
		}
	}
	else if (v0 == 5)
	{
		int inteiro;
		scanf("%d", &inteiro);
		memoria->registradores[2].valor = inteiro;
	}
	else if (v0 == 10)
	{
		printf("\nPrograma finalizado!\n--------------------\n");
		return 0;
	}
	else if (v0 == 11)
	{
		printf("%c", (int) memoria->registradores[4].valor);
	}
	else
	{
		printf("Erro syscall. v0 = %ld\n", v0);
		return 0;
	}

	return 1;
}

void executaOpTipoR(Memoria *memoria, char *funct, int valores[])
{
	if (!strcmp(funct, "add"))
	{
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];
		setNumeroSigned(&rs);
		setNumeroSigned(&rt);

		if (rd != 0)
			memoria->registradores[rd].valor = rs + rt;

		// printf("add -> 0x%08lx\n", memoria->registradores[rd].valor);
	}
	else if (!strcmp(funct, "addu"))
	{
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];
		setNumeroSigned(&rs);
		setNumeroSigned(&rt);

		if(rd != 0)
			memoria->registradores[rd].valor = rs + rt;

		// printf("addu -> 0x%08lx\t rs=0x%08lx e rt=0x%08lx\n", memoria->registradores[rd].valor, rs, rt);
	}
	else if (!strcmp(funct, "slt"))
	{
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		setNumeroSigned(&rs);
		setNumeroSigned(&rt);

		int rd = valores[2];

		if (rs < rt && rd != 0)
			memoria->registradores[rd].valor = 1;
		else if (rd != 0)
			memoria->registradores[rd].valor = 0;

		// printf("slt -> 0x%08lx\n", memoria->registradores[rd].valor);

	}
	else if (!strcmp(funct, "srl"))
	{
		int rd = valores[2];
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		int shamt = valores[3];
		setNumeroSigned(&rt);
		setNumeroSignedInt(&shamt);

		if (rd != 0)
			memoria->registradores[rd].valor = rt >> shamt;

		// printf("srl -> 0x%08lx\n", memoria->registradores[rd].valor);

	}
	else if (!strcmp(funct, "sll"))
	{
		int rd = valores[2];
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		int shamt = valores[3];
		setNumeroSigned(&rt);
		setNumeroSignedInt(&shamt);

		if (rd != 0)
			memoria->registradores[rd].valor = rt << shamt;

		// printf("sll -> 0x%08lx\trt=0x%08lx e shamt=%d\n", memoria->registradores[rd].valor, rt, shamt);
	}
	else if (!strcmp(funct, "jr"))
	{
		unsigned long int rs = memoria->registradores[valores[0]].valor;

		memoria->registradores[32].valor = rs - ALINHAMENTO;
		// printf("jr -> pc= 0x%08lx\trs=0x%08lx e alin= %d\n", memoria->registradores[32].valor, rs, ALINHAMENTO);
	}
}

int runCodigoTipoR(Memoria *memoria, Isa *isa, char *binario)
{
	char rs[6];
	substring(binario, 6, 5, rs); rs[5] = 0;

	char rt[6];
	substring(binario, 11, 5, rt); rt[5] = 0;

	char rd[6];
	substring(binario, 16, 5, rd); rd[5] = 0;

	char shamt[6];
	substring(binario, 21, 5, shamt); shamt[5] = 0;

	char funct[7];
	substring(binario, 26, 6, funct); funct[6] = 0;


	char *nomeFunct;
	int valorFunct = (int) convertBinToInt(funct);
	if (valorFunct != 12)
	{
		Registro *reg = getRegistroOpcode(isa, funct, "R");
		if (!reg)
		{
			printf("Não tem registro na ISA.\t%s\n", funct);
			return 0;
		}

		nomeFunct = reg->nomeCodigo;
	}
	else
		return executaSyscall(memoria);

	int numeroRS = (int) convertBinToInt(rs);
	int numeroRT = (int) convertBinToInt(rt);
	int numeroRD = (int) convertBinToInt(rd);
	int valorShamt = (int) convertBinToInt(shamt);

	int valores[5] = { numeroRS, numeroRT, numeroRD, valorShamt, valorFunct };
	executaOpTipoR(memoria, nomeFunct, valores);

	return 1;
}

void executaOpTipoI(Memoria *memoria, char *opcode, int valores[])
{
	if (!strcmp(opcode, "addi"))
	{
		int rt = valores[1];
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		if (rt != 0)
			memoria->registradores[rt].valor = rs + imediato;

		// printf("addi -> 0x%08lx\n", memoria->registradores[rt].valor);
	}
	else if(!strcmp(opcode, "addiu"))
	{
		int rt = valores[1];
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		if (rt != 0)
			memoria->registradores[rt].valor = rs + imediato;

		// printf("addiu -> 0x%08lx\trs=0x%08lx e im=%d\n", memoria->registradores[rt].valor, rs, imediato);
	}
	else if(!strcmp(opcode, "lui"))
	{
		int rt = valores[1];
		unsigned long long bin = dec2bin(valores[2]);
		char *binString = getString(bin);
		char *upper = convertBinToUpper(binString);

		unsigned long int endereco = convertBinToInt(upper);

		if (rt != 0)
			memoria->registradores[rt].valor = endereco;

		// printf("lui -> 0x%08lx\n", memoria->registradores[rt].valor);

		free(upper);
		free(binString);
	}
	else if (!strcmp(opcode, "lw"))
	{
		int rt = valores[1];
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);
		
		char* bin = getDataBinario(memoria, rs + imediato);
		unsigned long int valorMemoria = convertBinToInt(bin);
		
		if (rt != 0)
			memoria->registradores[rt].valor = valorMemoria;
		
	}
	else if (!strcmp(opcode, "sw"))
	{
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		setDataMemoria(memoria, rs + imediato, rt);
		
		// printf("sw -> endereco=0x%08lx\t rs=0x%08lx im=%d\t valor=0x%08lx\n", rs + imediato, rs, imediato, rt);
	}
	else if (!strcmp(opcode, "ori"))
	{
		int rt = valores[1];
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		if (rt != 0)
			memoria->registradores[valores[1]].valor = rs | imediato;

		// printf("ori -> 0x%08lx\n", memoria->registradores[rt].valor);
	}
	else if (!strcmp(opcode, "andi"))
	{
		int rt = valores[1];
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		if (rt != 0)
			memoria->registradores[rt].valor = rs & imediato;

		// printf("andi -> 0x%08lx\n", memoria->registradores[rt].valor);
	}
	else if (!strcmp(opcode, "beq"))
	{
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		if (rs == rt)
			memoria->registradores[32].valor += (imediato * ALINHAMENTO);
		// printf("\n");
	}
	else if (!strcmp(opcode, "bne"))
	{
		unsigned long int rs = memoria->registradores[valores[0]].valor;
		unsigned long int rt = memoria->registradores[valores[1]].valor;
		int imediato = valores[2];
		setNumeroSignedInt(&imediato);

		if (rs != rt)
			memoria->registradores[32].valor += (imediato * ALINHAMENTO);
		// printf("\n");
	}
}

int runCodigoTipoI(Memoria *memoria, Isa *isa, char *binario)
{
	char opcode[7];
	substring(binario, 0, 6, opcode); opcode[6] = 0;

	Registro *reg = getRegistroOpcode(isa, opcode, "I");
	if (!reg)
	{
		printf("Não tem registro na ISA.\t%s\n", opcode);
		return 0;
	}
	char *nomeOpcode = reg->nomeCodigo;

	char rs[6];
	substring(binario, 6, 5, rs); rs[5] = 0;

	char rt[6];
	substring(binario, 11, 5, rt); rt[5] = 0;

	char imediato[17];
	substring(binario, 16, 16, imediato); imediato[16] = 0;

	int numeroRS = (int) convertBinToInt(rs);
	int numeroRT = (int) convertBinToInt(rt);
	int valorImediato = (int) convertBinToInt(imediato);

	int valores[3] = { numeroRS, numeroRT, valorImediato };
	executaOpTipoI(memoria, nomeOpcode, valores);

	return 1;
}

void executaOpTipoJ(Memoria *memoria, char *opcode, unsigned long int imediato)
{
	if (!strcmp(opcode, "j"))
	{
		memoria->registradores[32].valor = imediato - ALINHAMENTO;
		// printf("\n");
	}
	else if (!strcmp(opcode, "jal"))
	{
		memoria->registradores[31].valor = memoria->registradores[32].valor + ALINHAMENTO;
		memoria->registradores[32].valor = imediato - ALINHAMENTO;
		// printf("jal -> pc= 0x%08lx\t ra= 0x%08lx\n", memoria->registradores[32].valor + ALINHAMENTO, memoria->registradores[31].valor);
	}
}

int runCodigoTipoJ(Memoria *memoria, char *binario, char *nomeOpcode)
{
	char imediato[27];
	substring(binario, 6, 26, imediato); imediato[26] = 0;

	unsigned long long im = convertBinToInt(imediato);

	im = im << 2;

	executaOpTipoJ(memoria, nomeOpcode, im);
}

int executarCodigo(Memoria *memoria, Isa *isa, char *binario)
{
	//printInstrucaoISA(isa, binario);
	// printf(" ");

	char opcode[7];
	substring(binario, 0, 6, opcode); opcode[6] = 0;

	int op = convertBinToInt(opcode);

	int correto = 0;
	if (op == 0)
	{
		correto = runCodigoTipoR(memoria, isa, binario);
		if (correto)
			memoria->registradores[32].valor += ALINHAMENTO;
		return correto;
	}

	Registro *reg = getRegistroOpcode(isa, opcode, "IJ");
	if (!reg)
	{
		printf("Nao possui registro na ISA.\t%s\n", opcode);
		return 0;
	}

	if (reg->tipo == 'I')
		correto = runCodigoTipoI(memoria, isa, binario);
	else
		correto = runCodigoTipoJ(memoria, binario, reg->nomeCodigo);

	if (correto)
		memoria->registradores[32].valor += ALINHAMENTO;

	return correto;
}

void liberarISA(Isa *isa)
{
	int i;
	for (i = 0; i < TAMANHO_TABELA; i++)
		free(isa->tabela[i]);
	free(isa->tabela);
	free(isa);
}