#include "isadecode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printCodigoTipoR(Isa *isa, unsigned int binario)
{
	int rs = (binario >> 21) & 0x1f;
	int rt = (binario >> 16) & 0x1f;
	int rd = (binario >> 11) & 0x1f;
	int shamt = (binario >> 6) & 0x1f;
	int funct = binario & 0x3f;

	printf("%08x\t", binario);
	
	if (binario == 0)
	{
		printf("nop\n");
		return;
	}

	if (funct == 0xc)
	{
		printf("syscall\n");
		return;
	} else if (funct == 0xd)
	{
		printf("break\n");
		return;
	}
	

	Registro *reg = getRegistroOpcode(isa, funct, "R");
	
	if (!reg)
	{
		printf("Não tem registro na ISA. R-type\t0x%08x\n", funct);
		return;
	}

	char *nomeFunct = reg->nomeCodigo;

	char *nomeRs = getNomeRegistrador(rs);
	char *nomeRt = getNomeRegistrador(rt);
	char *nomeRd = getNomeRegistrador(rd);

	if (!strcmp(nomeFunct, "srl") || !strcmp(nomeFunct, "sll") || !strcmp(nomeFunct, "sra"))
		printf("%s %s, %s, %d\n", nomeFunct, nomeRd, nomeRt, shamt);
	else if (!strcmp(nomeFunct, "jr"))
		printf("%s %s\n", nomeFunct, nomeRs);
	else if (!strcmp(nomeFunct, "jalr"))
		printf("%s %s %s\n", nomeFunct, nomeRd, nomeRs);
	else
		printf("%s %s, %s, %s\n", nomeFunct, nomeRd, nomeRs, nomeRt);
}

void printCodigoTipoI(Isa *isa, unsigned int binario)
{
	int opcode = binario >> 26;

	Registro *reg = getRegistroOpcode(isa, opcode, "I");
	if (!reg)
	{
		printf("Não tem registro na ISA.\tx0%08x\n", opcode);
		return;
	}

	char *nomeOpcode = reg->nomeCodigo;

	int rs = (binario >> 21) & 0x1f;
	int rt = (binario >> 16) & 0x1f;
	int imediato = binario & 0xffff;

    char *nomeRs = getNomeRegistrador(rs);
	char *nomeRt = getNomeRegistrador(rt);

	printf("%08x\t", binario);

	if (!strcmp(nomeOpcode, "lui"))
	{
		printf("%s %s, 0x%08x\n", nomeOpcode, nomeRt, imediato);
		return;
	}

	unsigned int im = extenderSinal(imediato, 16);
	
	if (!strcmp(nomeOpcode, "lw") || !strcmp(nomeOpcode, "sw") || !strcmp(nomeOpcode, "lb") ||
	   !strcmp(nomeOpcode, "lbu") || !strcmp(nomeOpcode, "sb"))
	{
		printf("%s %s, %d(%s)  # 0x%08x\n", nomeOpcode, nomeRt, im, nomeRs, im);
		return;
	}
	else if (!strcmp(nomeOpcode, "blez") || !strcmp(nomeOpcode, "bgez"))
	{
		printf("%s %s, 0x%08x\n", nomeOpcode, nomeRs, im);
		return;
	}

	printf("%s %s, %s, %d\n", nomeOpcode, nomeRt, nomeRs, im);
}

void printCodigoTipoJ(Isa *isa, unsigned int binario, unsigned int pc)
{
	int opcode = binario >> 26;

	Registro *reg = getRegistroOpcode(isa, opcode, "J");
	if (!reg)
	{
		printf("Não tem registro na ISA.\t0x%08x\n", opcode);
		return;
	}
	char *nomeOpcode = reg->nomeCodigo;

	unsigned int imediato = binario & 0x3ffffff;

	unsigned int im = imediato << 2;

	printf("%08x\t", binario);

	printf("%s 0x%x # 0x%x\n", nomeOpcode, imediato, im);
}

void printCodigoCop1(Isa *isa, unsigned int binario)
{
	printf("%08x\t", binario);

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
			return;
		}
	}

	// printf("NomeCodigo = %s\n", nomeCodigo);

	if (!strcmp(nomeCodigo, "mfc1") || !strcmp(nomeCodigo, "mtc1"))
	{
		int rt = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;

		char *nomeRt = getNomeRegistrador(rt);
		char *nomeFs = getNomeRegistrador(fs + IDX_INICIO_COP1);

		printf("%s %s, %s\n", nomeCodigo, nomeRt, nomeFs);
	}
	else if (!strcmp(nomeCodigo, "mult") || !strcmp(nomeCodigo, "div"))
	{
		int rs = (binario >> 21) & 0x1f; 
		int rt = (binario >> 16) & 0x1f;

		char *nomeRs = getNomeRegistrador(rs);
		char *nomeRt = getNomeRegistrador(rt);

		printf("%s %s, %s\n", nomeCodigo, nomeRs, nomeRt);
	}
	else if (!strcmp(nomeCodigo, "mflo") || !strcmp(nomeCodigo, "mfhi"))
	{
		int rd = (binario >> 11) & 0x1f; 

		char *nomeRd = getNomeRegistrador(rd);

		printf("%s %s\n", nomeCodigo, nomeRd);
	}
	else if (!strcmp(nomeCodigo, "lwc1") || !strcmp(nomeCodigo, "ldc1") || !strcmp(nomeCodigo, "swc1"))
	{
		int base = (binario >> 21) & 0x1f;
		int ft = (binario >> 16) & 0x1f;
		int offset = binario & 0xffff;

		char *nomeBase = getNomeRegistrador(base); 
		char *nomeFt = getNomeRegistrador(ft + IDX_INICIO_COP1);

		printf("%s %s %d(%s)  # 0x%08x\n", nomeCodigo, nomeFt, offset, nomeBase, offset); 
	}
	else if (!strcmp(nomeCodigo, "mov.d") || !strcmp(nomeCodigo, "mov.s"))
	{
		int fmt = (binario >> 21) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFd = getNomeRegistrador(IDX_INICIO_COP1 + fd);
		
		if (fmt == 0x10)
			nomeCodigo = "mov.s";
		else if (fmt == 0x11)
			nomeCodigo = "mov.d";

		printf("%s %s, %s\n", nomeCodigo, nomeFd, nomeFs);
	}
	else if (!strcmp(nomeCodigo, "cvt.d.w") || !strcmp(nomeCodigo, "cvt.s.d"))
	{
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFd = getNomeRegistrador(IDX_INICIO_COP1 + fd);

		printf("%s %s, %s\n", nomeCodigo, nomeFd, nomeFs);
	}
	else if (!strcmp(nomeCodigo, "div.d"))
	{
		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		char *nomeFt = getNomeRegistrador(IDX_INICIO_COP1 + ft);
		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFd = getNomeRegistrador(IDX_INICIO_COP1 + fd);

		printf("%s %s, %s, %s\n", nomeCodigo, nomeFd, nomeFs, nomeFt);
	}
	else if(!strcmp(nomeCodigo, "mul.s") || !strcmp(nomeCodigo, "mul.d"))
	{
		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		char *nomeFt = getNomeRegistrador(IDX_INICIO_COP1 + ft);
		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFd = getNomeRegistrador(IDX_INICIO_COP1 + fd);

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			printf("mul.s %s, %s, %s\n", nomeFd, nomeFs, nomeFt);
		else if (fmt == 0x11)
			printf("mul.d %s, %s, %s\n", nomeFd, nomeFs, nomeFt);
	} 
	else if (!strcmp(nomeCodigo, "add.d") || !strcmp(nomeCodigo, "add.s"))
	{
		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		char *nomeFt = getNomeRegistrador(IDX_INICIO_COP1 + ft);
		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFd = getNomeRegistrador(IDX_INICIO_COP1 + fd);

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			printf("add.s %s, %s, %s\n", nomeFd, nomeFs, nomeFt);
		else if (fmt == 0x11)
			printf("add.d %s, %s, %s\n", nomeFd, nomeFs, nomeFt);

	}
	else if (!strcmp(nomeCodigo, "sub.s"))
	{
		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;
		int fd = (binario >> 6) & 0x1f;

		char *nomeFt = getNomeRegistrador(IDX_INICIO_COP1 + ft);
		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFd = getNomeRegistrador(IDX_INICIO_COP1 + fd);

		int fmt = (binario >> 21) & 0x1f;

		if (fmt == 0x10)
			printf("sub.s %s, %s, %s\n", nomeFd, nomeFs, nomeFt);
	}
	else if (!strcmp(nomeCodigo, "c.cond.fmt"))
	{
		int fmt = (binario >> 21) & 0x1f;
		int ft = (binario >> 16) & 0x1f;
		int fs = (binario >> 11) & 0x1f;

		int cond = binario & 0xf;
		char *nomeCond = NULL;
		if (cond == 0x2)
		{
			nomeCond == ".eq";
		}
		else if (cond == 0xc)
		{
			nomeCond = ".lt";
		}
		else if (cond == 0xe)
		{
			nomeCond = ".le";
		}

		char *nomeFormato = NULL;
		if (fmt == 0x10)
			nomeFormato = ".s";
		else if (fmt == 0x11)
			nomeFormato = ".d";

		char *aux = (char *) malloc(sizeof(char) * 6);
		memset(aux, 0, sizeof(char) * 6);
		aux[0] = 'c';
		strcat(aux, nomeCond);
		strcat(aux, nomeFormato);

		char *nomeFs = getNomeRegistrador(IDX_INICIO_COP1 + fs);
		char *nomeFt = getNomeRegistrador(IDX_INICIO_COP1 + ft);

		printf("%s %s, %s\n", aux, nomeFs, nomeFt);

		free(aux);
	}
	else if (!strcmp(nomeCodigo, "bc1f") || !strcmp(nomeCodigo, "bc1t"))
	{

		int imediato = binario & 0xffff;
		int tf = (binario >> 16) & 0x1;

		unsigned int im = extenderSinal(imediato, 16);

		if (tf == 1)
			nomeCodigo = "bc1t";
		else 
			nomeCodigo = "bc1f";


		printf("%s %d\n", nomeCodigo, im);
	}
}
