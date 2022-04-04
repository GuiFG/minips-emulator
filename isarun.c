#include "isarun.h"
#include <stdio.h>
#include <string.h>

char *nomeInstrucao;

int isExceptionChar(int codigo)
{
	if (codigo == 0xea)
		return 1;
	else if (codigo == 0xe9)
		return 1;
	else if (codigo == 0xe7)
		return 1;
	else if (codigo == 0xe3)
		return 1;
	else if (codigo == 0xfa)
		return 1;

	return 0;
}

void printExceptionChar(int codigo)
{
	if (codigo == 0xea)
		printf("ê");
	else if (codigo == 0xe9)
		printf("é");
	else if (codigo == 0xe7)
		printf("ç");
	else if (codigo == 0xe3)
		printf("ã");
	else if (codigo == 0xfa)
		printf("ú");
	else
		printf(" ");
}

int executaSyscall(Memoria *memoria, Isa *isa)
{
	unsigned int v0 = memoria->registradores[2].valor;

	if (v0 == 1)
	{
		printf("%d", memoria->registradores[4].valor);
	}
	else if (v0 == 2)
	{
		unsigned int f12 = memoria->registradores[IDX_INICIO_COP1 + 12].valor;

		float result = getFloat(f12);

		printf("%e", result);
	}
	else if (v0 == 3)
	{
		unsigned int f12 = memoria->registradores[IDX_INICIO_COP1 + 12].valor;
		unsigned int f13 = memoria->registradores[IDX_INICIO_COP1 + 13].valor;

		unsigned long resultado = 0;

		resultado = resultado | f13;		
		resultado = resultado << ARQUITETURA;
		resultado = resultado | f12;

		double result = getDouble(resultado);

		printf("%e", result);
	}
	else if (v0 == 4)
	{
		nomeInstrucao = "syscall-r";
		
		unsigned int a0 = memoria->registradores[4].valor;
        int offset = a0 % ALINHAMENTO;
		a0 -= offset;

		isa->countCycles -= CycleTime;

		int lendo = 1;
		while (lendo)
		{   
			if (TRACE || DEBUG)
				gerarLogTrace("R", a0, memoria->tamanhoLinha, 0);
			
			
			unsigned int valor = 0;

			if (CONFIG == 0 || CONFIG == 1)
			{
				valor = getDataValor(memoria, a0, &isa->countCycles);
			}
			else
			{
				valor = getDataCache(memoria, a0, "dados", 1, 1, &isa->countCycles);
			}
				
			
			int i;
			for (i = offset; i < ALINHAMENTO; i++)
			{
				int aux = (valor & (0xff << (i * 8))) >> (i * 8);

				if (aux != 0)
				{
					if (!isExceptionChar(aux))
                		printf("%c", aux);
					else
						printExceptionChar(aux);
				}
				else 
				{
					lendo = 0;
					break;
				}
			}

            offset = 0;
			a0 += ALINHAMENTO;
		}
	}
	else if (v0 == 5)
	{
		int inteiro;
		scanf("%d", &inteiro);
		memoria->registradores[2].valor = inteiro;
	}
	else if (v0 == 6)
	{
		float f = 0;
		scanf("%f", &f);	

		unsigned int valor = convertFloatToBits(f);
		memoria->registradores[IDX_INICIO_COP1].valor = valor;
	}
	else if (v0 == 7)
	{
		double d = 0;
		scanf("%lf", &d);

		unsigned long db = convertToLongBits(d);

		memoria->registradores[IDX_INICIO_COP1].valor = db & 0xffffffff; 
		memoria->registradores[IDX_INICIO_COP1 + 1].valor = db >> ARQUITETURA;
	}
	else if (v0 == 10)
	{
		return 10;
	}
	else if (v0 == 11)
	{
		printf("%c", (int) memoria->registradores[4].valor);
	}
	else
	{
		printf("Erro syscall. v0 = %d\n", v0);
		return 0;
	}

	return 1;
}

void setNomeTipoR(Isa *isa, unsigned int binario)
{
	if (binario == 0)
	{
		nomeInstrucao = "nop";
		return;
	}
		
	int funct = binario & 0x3f;
	if (funct == 0xd)
	{
		nomeInstrucao = "break";
		return;
	}

	if (funct != 0xc)
	{
		Registro *reg = getRegistroOpcode(isa, funct, "R");
		if (!reg)
		{
			printf("Não tem registro na ISA. type-R\t%x\n", funct);
			nomeInstrucao = "-";
			return;
		}

		nomeInstrucao = reg->nomeCodigo;
	}
	else 
	{
		nomeInstrucao = "syscall";
	}
}

void executaOpTipoR(Memoria *memoria, int valores[])
{	
	if (!strcmp(nomeInstrucao, "add"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];
		
		if (rd != 0)
			memoria->registradores[rd].valor = rs + rt;

		if (DECODE)
			printf("\t0x%08x + 0x%08x -> 0x%08x\n", rs, rt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "addu"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];

		if(rd != 0)
			memoria->registradores[rd].valor = rs + rt;

		if (DECODE)
			printf("\t0x%08x + 0x%08x -> 0x%08x\n", rs, rt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "subu"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];

		if(rd != 0)
			memoria->registradores[rd].valor = rs - rt;

		if (DECODE)
			printf("\t0x%08x - 0x%08x -> 0x%08x\n", rs, rt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "slt"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;

		int rd = valores[2];

		if (rd != 0)
		{
			unsigned int resultado = rs - rt;

			int msb = resultado >> (ARQUITETURA - 1);
			if (msb == 1)
				memoria->registradores[rd].valor = 1;
			else
				memoria->registradores[rd].valor = 0;
		}

		if (DECODE)
			printf("\t0x%08x < 0x%08x -> 0x%08x\n", rs, rt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "sltu"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;

		int rd = valores[2];

		if (rd != 0)
		{
			unsigned int resultado = rs - rt;

			int msb = resultado >> (ARQUITETURA - 1);
			if (msb == 1)
				memoria->registradores[rd].valor = 1;
			else
				memoria->registradores[rd].valor = 0;
		}

		if (DECODE)
			printf("\t0x%08x < 0x%08x -> 0x%08x\n", rs, rt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "srl"))
	{
		int rd = valores[2];
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int shamt = valores[3];

		if (rd != 0)
			memoria->registradores[rd].valor = rt >> shamt;

		if (DECODE)
			printf("\t0x%08x >> 0x%08x -> 0x%08x\n", rt, shamt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "sra"))
	{
		int rd = valores[2];
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int shamt = valores[3];

		if (rd != 0)
		{
			unsigned int result = rt >> shamt;
			
			char bit = (rt >> (ARQUITETURA - 1)) & 0x1;
			if (bit == 1)
			{
				unsigned int bits = getBitsUm(shamt);
				bits = bits << (ARQUITETURA - shamt);

				result = result | bits;
			}

			memoria->registradores[rd].valor = result;
		}
			 
		if (DECODE)
			printf("\t0x%08x >> 0x%08x -> 0x%08x\n", rt, shamt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "sll"))
	{
		int rd = valores[2];
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int shamt = valores[3];

		if (rd != 0)
			memoria->registradores[rd].valor = rt << shamt;

		if (DECODE)
			printf("\t0x%08x << 0x%08x -> 0x%08x\n", rt, shamt, memoria->registradores[rd].valor);
	}
	else if (!strcmp(nomeInstrucao, "jr"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;

		FORWARDING = rs;

		if (DECODE)
			printf("\tpc <- 0x%08x\n", FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "jalr"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		int rd = valores[2];

		if (rd == 0)
			rd = 31;

		unsigned int returnAddr = memoria->registradores[32].valor + (2 * ALINHAMENTO);
		memoria->registradores[rd].valor = returnAddr;

		FORWARDING = rs;

		if (DECODE)
			printf("\t%s <- 0x%08x, pc <- 0x%08x\n",  memoria->registradores[rd].nome, returnAddr, FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "or"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];
		
		if (rd != 0)
			memoria->registradores[rd].valor = rs | rt;

		if (DECODE)
			printf("\t0x%08x <- 0x%08x or 0x%08x\n", memoria->registradores[rd].valor, rs, rt);
		
	}
	else if (!strcmp(nomeInstrucao, "xor"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];

		if (rd != 0)
			memoria->registradores[rd].valor = rs ^ rt;

	}
	else if (!strcmp(nomeInstrucao, "and"))
	{
		unsigned int rs = memoria->registradores[valores[0]].valor;
		unsigned int rt = memoria->registradores[valores[1]].valor;
		int rd = valores[2];

		if (rd != 0)
			memoria->registradores[rd].valor = rs & rt;

	}
}

int runCodigoTipoR(Memoria *memoria, Isa *isa, unsigned int binario)
{	
	if (!strcmp(nomeInstrucao, "-"))
		return 0;

	if (!strcmp(nomeInstrucao, "nop") || !strcmp(nomeInstrucao, "break"))
		return 1;

	if (!strcmp(nomeInstrucao, "syscall"))
		return executaSyscall(memoria, isa);
	
	int valorRs = (binario >> 21) & 0x1f;
	int valorRt = (binario >> 16) & 0x1f;
	int valorRD = (binario >> 11) & 0x1f;
	int valorShamt = (binario >> 6) & 0x1f;

	int valores[4] = { valorRs, valorRt, valorRD, valorShamt };
	executaOpTipoR(memoria, valores);

	return 1;
}

void setNomeTipoI(Isa *isa, unsigned int binario)
{
	int opcode = binario >> 26;
	Registro *reg = getRegistroOpcode(isa, opcode, "I");
	if (!reg)
	{
		printf("Não tem registro na ISA. [run] - type-I\t0x%08x\n", opcode);
		nomeInstrucao = "-";
		return;
	}

	nomeInstrucao = reg->nomeCodigo;
}

void executaOpTipoI(Memoria *memoria, int rs, int rt, int imediato, unsigned int *cycles)
{
	if (!strcmp(nomeInstrucao, "addi"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (rt != 0)
			memoria->registradores[rt].valor = valorRs + imediato;
		
		if (DECODE)
			printf("\t0x%08x + 0x%08x -> 0x%08x\n", valorRs, imediato, memoria->registradores[rt].valor);
	}
	else if(!strcmp(nomeInstrucao, "addiu"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (rt != 0)
			memoria->registradores[rt].valor = valorRs + im;

		if (DECODE)
			printf("\t0x%08x <- 0x%08x + 0x%08x\n", memoria->registradores[rt].valor, valorRs, im);
	}
	else if(!strcmp(nomeInstrucao, "lui"))
	{
		unsigned int upperIm = convertImediatoToUpper(imediato);

		if (rt != 0)
			memoria->registradores[rt].valor = upperIm;

		if (DECODE)
			printf("\t0x%08x <- 0x%08x << %d\n", memoria->registradores[rt].valor, imediato, (ARQUITETURA / 2));
	}
	else if (!strcmp(nomeInstrucao, "lw"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("R", valorRs + im, memoria->tamanhoLinha, 0);

		*cycles -= CycleTime;

		unsigned int valorMemoria = 0;
		if (CONFIG == 0 || CONFIG == 1)
			valorMemoria = getDataValor(memoria, valorRs + im, cycles);
		else
			valorMemoria = getDataCache(memoria, valorRs + im, "dados", 1, 1, cycles);
		
		if (rt != 0)
			memoria->registradores[rt].valor = valorMemoria;

		if (DECODE)
			printf("\t0x%08x <- 0x%08x + 0x%08x\n", memoria->registradores[rt].valor, valorRs, im);	
	}
	else if (!strcmp(nomeInstrucao, "lb"))
	{
		unsigned int base = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("R", base + im, memoria->tamanhoLinha, 0);

		*cycles -= CycleTime;

		unsigned int valor = 0;
		if (CONFIG == 0 || CONFIG == 1)
			valor = getDataValor(memoria, base + im, cycles);
		else
			valor = getDataCache(memoria, base + im, "dados", 1, 1, cycles);
		
		valor = valor & 0xff;
		valor = extenderSinal(valor, 8);

		if (rt != 0)
			memoria->registradores[rt].valor = valor;

		if (DECODE)
			printf("\t0x%08x <- 0x%08x + 0x%08x\n", memoria->registradores[rt].valor, base, im);
	}
	else if (!strcmp(nomeInstrucao, "lbu"))
	{
		unsigned int base = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("R", base + im, memoria->tamanhoLinha, 0);

		*cycles -= CycleTime;

		unsigned int valor = 0;
		if (CONFIG == 0 || CONFIG == 1)
			valor = getDataValor(memoria, base + im, cycles);
		else
			valor = getDataCache(memoria, base + im, "dados", 1, 1, cycles);

		unsigned int aux = 0 << (ARQUITETURA - 1);
		valor = aux | (valor & 0xff);

		if (rt != 0)
			memoria->registradores[rt].valor = valor;

		if (DECODE)
			printf("\t0x%08x <- memory[0x%08x + 0x%08x]\n", memoria->registradores[rt].valor, base, im);
	}
	else if (!strcmp(nomeInstrucao, "sw"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int valorRt = memoria->registradores[rt].valor;
		
		unsigned int im = extenderSinal(imediato, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("W", valorRs + im, memoria->tamanhoLinha, 0);
				
		*cycles -= CycleTime;

		if (CONFIG == 0 || CONFIG == 1)
			setDataMemoria(memoria, valorRs + im, valorRt, cycles, 1);
		else
			setDataCache(memoria, valorRs + im, valorRt, 1, 1, cycles);

		if (DECODE)
			printf("\t0x%08x + 0x%08x -> 0x%08x\n", valorRs, im, valorRt);
	}
	else if (!strcmp(nomeInstrucao, "sb"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int valorRt = memoria->registradores[rt].valor & 0xff;
		
		unsigned int im = extenderSinal(imediato, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("W", valorRs + im, memoria->tamanhoLinha, 0);

		*cycles -= CycleTime;

		if (CONFIG == 0 || CONFIG == 1)
			setDataMemoria(memoria, valorRs + im, valorRt, cycles, 1);
		else
			setDataCache(memoria, valorRs + im, valorRt, 1, 1, cycles);
		
		if (DECODE)
			printf("\tmemory[%x + %x] <- %x\n", valorRs, im, valorRt);
	}
	else if (!strcmp(nomeInstrucao, "ori"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;

		if (rt != 0)
			memoria->registradores[rt].valor = valorRs | imediato;

		if (DECODE)
			printf("\t0x%08x or 0x%08x -> 0x%08x\n", valorRs, imediato, memoria->registradores[rt].valor);
	}
	else if (!strcmp(nomeInstrucao, "andi"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;

		if (rt != 0)
			memoria->registradores[rt].valor = valorRs & imediato;

		if (DECODE)
			printf("\t0x%08x and 0x%08x -> 0x%08x\n", valorRs, imediato, memoria->registradores[rt].valor);
	}
	else if (!strcmp(nomeInstrucao, "beq"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int valorRt = memoria->registradores[rt].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (valorRs == valorRt)
			FORWARDING = memoria->registradores[32].valor + (im * ALINHAMENTO) + ALINHAMENTO;
		
		if (DECODE)
			printf("\tpc=0x%08x\n", FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "bne"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int valorRt = memoria->registradores[rt].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (valorRs != valorRt)
			FORWARDING = memoria->registradores[32].valor + (im * ALINHAMENTO) + ALINHAMENTO;
		
		if (DECODE)
			printf("\tpc=0x%08x\n", FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "blez"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (valorRs <= 0)
			FORWARDING = memoria->registradores[32].valor + (im * ALINHAMENTO) + ALINHAMENTO;
		
		if (DECODE)
			printf("\t%x <= 0 then %x\n", valorRs, FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "bgez"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (valorRs >= 0)
			FORWARDING = memoria->registradores[32].valor + (im * ALINHAMENTO) + ALINHAMENTO;
		
		if (DECODE)
			printf("\t%x >= 0 then %x\n", valorRs, FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "slti"))
	{
		unsigned int valorRs = memoria->registradores[rs].valor;
		unsigned int im = extenderSinal(imediato, 16);

		if (rt != 0)
		{
			unsigned int resultado = valorRs - im;

			int msb = resultado >> (ARQUITETURA - 1);
			if (msb == 1)
				memoria->registradores[rt].valor = 1;
			else
				memoria->registradores[rt].valor = 0;
		}

		if (DECODE)
			printf("\t0x%08x < 0x%08x = 0x%08x\n", rs, imediato, memoria->registradores[rt].valor);
	}
	
}

int runCodigoTipoI(Memoria *memoria, unsigned int binario, unsigned int *cycles)
{
	if (!strcmp(nomeInstrucao, "-"))
		return 0;

	int rs = (binario >> 21) & 0x1f;
	int rt = (binario >> 16) & 0x1f;
	int imediato = binario & 0xffff;

	executaOpTipoI(memoria, rs, rt, imediato, cycles);

	return 1;
}

void setNomeTipoJ(Isa *isa, unsigned int binario)
{
	int opcode = binario >> 26;
	Registro *reg = getRegistroOpcode(isa, opcode, "J");
	if (!reg)
	{
		printf("Não tem registro na ISA. [run] - type-J\t0x%08x\n", opcode);
		nomeInstrucao = "-";
		return;
	}

	nomeInstrucao = reg->nomeCodigo;
}

void executaOpTipoJ(Memoria *memoria, unsigned int imediato)
{
	if (!strcmp(nomeInstrucao, "j"))
	{
		FORWARDING =  imediato;

		if (DECODE)
			printf("\tpc= 0x%08x\n", FORWARDING);
	}
	else if (!strcmp(nomeInstrucao, "jal"))
	{
		memoria->registradores[31].valor = memoria->registradores[32].valor + (2 * ALINHAMENTO);
		FORWARDING = imediato;

		
		if (DECODE)
			printf("\tpc= 0x%08x\t ra= 0x%08x\n", FORWARDING, memoria->registradores[31].valor);
	}
}

int runCodigoTipoJ(Memoria *memoria, unsigned int binario)
{
	if (!strcmp(nomeInstrucao, "-"))
		return 0;

	unsigned int imediato = binario & 0x3ffffff;

	imediato = imediato << 2;

	executaOpTipoJ(memoria, imediato);

	return 1;
}

void setNomeTipoCop1(Isa *isa, unsigned int binario)
{
	int opcode = binario >> 26;

	if (opcode == 0)
		opcode = binario & 0x3f;
	
	int func = 0;
	char *nomeCodigo = NULL;
	if (opcode != 0x11)
		nomeCodigo = getNomeInstrucaoCop1(isa, opcode, func);
	else
	{
		func = (binario >> 21) & 0x1f;

		if (func != 0)
			nomeCodigo = getNomeInstrucaoCop1(isa, opcode, func);
		else
		{
			func = binario & 0x3ff;
			int mf = (binario >> 21) & 0x1f;

			if (func == 0 && mf == 0) 
				func = 0xfe;

			nomeCodigo = getNomeInstrucaoCop1(isa, opcode, func);
		}
	}

	if (!nomeCodigo)
	{
		func = binario & 0x3f;
		nomeCodigo = getNomeInstrucaoCop1(isa, opcode, func);

		if (!nomeCodigo)
		{
			func = binario & 0xf;
			int cc = (binario >> 7) & 0xf; 

			if ((func == 0x2 || func == 0xc || func == 0xe)  && cc == 0)// c.cond.fmt
			{
				func = 0xff;
				nomeCodigo = getNomeInstrucaoCop1(isa, opcode, func);
			}
		}
	
		if (!nomeCodigo)
		{
			printf("Não possui instrução na ISA. cop1 - %x\n", opcode);
			nomeInstrucao = "-";
			return;
		}
	}

	nomeInstrucao = nomeCodigo;
}

int runCodigoCop1(Memoria *memoria, Isa *isa, unsigned int binario)
{
	if (!strcmp(nomeInstrucao, "-"))
		return 0;

	if (!strcmp(nomeInstrucao, "mfc1") )
	{
		isa->countTypeFR += 1;

		int mf = (binario >> 21) & 0x1f;
		int rt = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int bits = binario & 0x3ff; 	

		mfc1(memoria, rt, fs);
	}
	else if (!strcmp(nomeInstrucao, "mtc1"))
	{
		isa->countTypeFR += 1;

		int rt = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;

		mtc1(memoria, rt, fs);
	}
	else if (!strcmp(nomeInstrucao, "mult"))
	{
		isa->countTypeFR += 1;

		int rs = (binario >> 21) & 0x1f; 
		int rt = (binario >> 16) & 0x1f;

		mult(memoria, rs, rt);
	}
	else if (!strcmp(nomeInstrucao, "div"))
	{
		isa->countTypeFR += 1; 

		int rs = (binario >> 21) & 0x1f; 
		int rt = (binario >> 16) & 0x1f;	

		divCop1(memoria, rs, rt);
	}
	else if (!strcmp(nomeInstrucao, "mflo"))
	{
		isa->countTypeFR += 1;
		
		int rd = (binario >> 11) & 0x1f; 

		mflo(memoria, rd);
	}
	else if (!strcmp(nomeInstrucao, "mfhi"))
	{
		isa->countTypeFR += 1;

		int rd = (binario >> 11) & 0x1f;

		mfhi(memoria, rd);
	}
	else if (!strcmp(nomeInstrucao, "lwc1"))
	{
		isa->countTypeI += 1;

		int base = (binario >> 21) & 0x1f;
		int rt = (binario >> 16) & 0x1f;
		int offset = binario & 0xffff;

		unsigned int valorBase = memoria->registradores[base].valor;
    	unsigned int im = extenderSinal(offset, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("R", valorBase + im, memoria->tamanhoLinha, 0);
		
		lwc1(memoria, valorBase, rt, im, &isa->countCycles);
	}
	else if (!strcmp(nomeInstrucao, "ldc1"))
	{
		isa->countTypeI += 1;

		int base = (binario >> 21) & 0x1f;
		int rt = (binario >> 16) & 0x1f;
		int offset = binario & 0xffff;

		unsigned int valorBase = memoria->registradores[base].valor;
   		unsigned int im = extenderSinal(offset, 16);

		if (TRACE || DEBUG)
		{
			gerarLogTrace("R", valorBase + im, memoria->tamanhoLinha, 0);
			gerarLogTrace("R", valorBase + im + ALINHAMENTO, memoria->tamanhoLinha, 0);
		}

		ldc1(memoria, valorBase, rt, im, &isa->countCycles);
	}
	else if (!strcmp(nomeInstrucao, "swc1"))
	{
		isa->countTypeI += 1;

		int base = (binario >> 21) & 0x1f;
		int rt = (binario >> 16) & 0x1f;
		int offset = binario & 0xffff;

		unsigned int valorBase = memoria->registradores[base].valor;
   	 	unsigned int im = extenderSinal(offset, 16);

		if (TRACE || DEBUG)
			gerarLogTrace("W", valorBase + im, memoria->tamanhoLinha, 0);
		

		swc1(memoria, valorBase, rt, im, &isa->countCycles);
	}
	else if (!strcmp(nomeInstrucao, "mov.d") || !strcmp(nomeInstrucao, "mov.s"))
	{
		isa->countTypeFR += 1;

		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			movs(memoria, fs, fd);
		else if (fmt == 0x11)
			movd(memoria, fs, fd);
	}
	else if (!strcmp(nomeInstrucao, "cvt.d.w"))
	{
		isa->countTypeFR += 1;

		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		cvtdw(memoria, fs, fd);
	}
	else if (!strcmp(nomeInstrucao, "cvt.s.d"))
	{
		isa->countTypeFR += 1;

		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		cvtsd(memoria, fs, fd);
	}
	else if (!strcmp(nomeInstrucao, "div.d"))
	{
		isa->countTypeFR += 1;

		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		divd(memoria, ft, fs, fd);
	}
	else if (!strcmp(nomeInstrucao, "mul.s") || !strcmp(nomeInstrucao, "mul.d"))
	{
		isa->countTypeFR += 1;

		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			muls(memoria, ft, fs, fd);
		else if (fmt == 0x11)	
			muld(memoria, ft, fs, fd);
	}
	else if (!strcmp(nomeInstrucao, "add.s") || !strcmp(nomeInstrucao, "add.d"))
	{
		isa->countTypeFR += 1;

		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			adds(memoria, ft, fs, fd);
		else if (fmt == 0x11)
			addd(memoria, ft, fs, fd);
	}
	else if (!strcmp(nomeInstrucao, "sub.s"))
	{
		isa->countTypeFR += 1;

		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			subs(memoria, ft, fs, fd);

	}
	else if (!strcmp(nomeInstrucao, "c.cond.fmt"))
	{
		isa->countTypeFR += 1;

		int fmt = (binario >> 21) & 0x1f;
		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;

		int cond = binario & 0xf;

		if (fmt == 0x10)
			cconds(memoria, ft, fs, cond);
		else if (fmt == 0x11)
			ccondd(memoria, ft, fs, cond);
	}
	else if (!strcmp(nomeInstrucao, "bc1f") || !strcmp(nomeInstrucao, "bc1t"))
	{
		isa->countTypeFI += 1;

		int tf = (binario >> 16) & 0x1;
		int imediato = binario & 0xffff;

		if (tf == 1)
			FORWARDING = bc1t(memoria, imediato);
		else 
			FORWARDING = bc1f(memoria, imediato);
	}

	return 1;
}