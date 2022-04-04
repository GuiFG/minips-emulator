#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *nomeInstrucao;
int append = 0;


void copiarString(char *dest, char *source)
{
	int tamanho = strlen(source);

	int i;
	for (i = 0; i < tamanho; i++)
		dest[i] = source[i];
}

void convertToLittleEndian(int **word, int size)
{
	int i;
	for (i = size - 1; i >= size / 2; i--)
	{
		int aux = (*word)[i];
		(*word)[i] = (*word)[size - i - 1];
		(*word)[size - i - 1] = aux;
	}
}

unsigned int getValorWord(int *word)
{
	unsigned int valor = 0;

	int i;
	for (i = 0; i < ALINHAMENTO; i++)
	{
		int byte = word[i];

		valor = valor << 8;
		valor = valor | byte;
	}

	return valor;
}

unsigned int extenderSinal(int valor, int bits)
{	
	int msb = (valor >> (bits - 1)) & 0x1;

	int aux = 0;
	int i = 0;
	while (i < (ARQUITETURA - bits))
	{
		aux = aux | msb;
		msb = msb << 1;

		i += 1;
	}

	aux = aux << bits;
	unsigned int resultado = valor | aux;

	return resultado;
}

unsigned int complemento(unsigned int valor)
{
	unsigned int result = 0;

	int i = 0;
	while (i < ARQUITETURA)
	{
		char bit = (valor >> (ARQUITETURA - 1)) & 0x1;
		result = result << 1;
		
		if (bit == 1)
			result = result | 0x0;
		else 
			result = result | 0x1;

		valor = valor << 1;
		i += 1;
	}

	result += 1;

	return result;
}

int isNegative(unsigned int valor)
{
	return (valor >> (ARQUITETURA - 1) & 0x1);
}

unsigned int convertImediatoToUpper(int imediato)
{
	return imediato << (ARQUITETURA / 2);
}

void setNumeroSigned(unsigned int *valor)
{
	int msb = *valor >> (ARQUITETURA - 1);
	if (msb == 1)
		*valor = ~(*valor) + 1;
}

void imprimirLista(Data *no)
{
	if (no == NULL)
		return;
	
	printf("Endereco: 0x%08x\t", no->endereco);
	imprimirLista(no->proximo);
}

void imprimirData(Memoria *memoria)
{
	HashData *data = memoria->data;
	int i;
	for (i = 0; i < TAMANHO_DATA; i++)
	{
		imprimirLista(data[i].encadeada);
		if (data[i].encadeada != NULL) printf("\n");
	}
}

double potencia(int base, int expoente)
{
	double resultado = 1;
	int i;

	int qtdIt = expoente;
	if (expoente < 0)
		qtdIt *= -1;

	for (i = 0; i < qtdIt; i++)
		resultado *= base;

	if (expoente < 0)
		return 1 / resultado;

	return resultado;
}

double calculaMantissa(unsigned long mantissa, int tamanho)
{
	int i;
	double resultado = 0;
	for (i = 0; i < tamanho; i++)
	{
		int bit = (mantissa >> (tamanho - i - 1)) & 0x1;
		int exp = (i + 1) * -1;
		double aux = potencia(2, exp);

		if (bit == 1)
			resultado += aux;
	}

	return resultado;
}

float getFloat(unsigned int valor)
{
	int sinal = valor >> (ARQUITETURA - 1);
	int expoente = (valor >> 23) & 0xff;
	unsigned int mantissa = (valor << 9) >> 9;
	int bias = 127;

	float resultado = potencia(-1, sinal);

	float aux = calculaMantissa(mantissa, 23);
	resultado *= (1 + aux);

	resultado *= potencia(2, expoente - bias);

	return resultado; 
}

double getDouble(unsigned long valor)
{
	int sinal = valor >> ((2 * ARQUITETURA) - 1);
	int expoente = (valor >> 52) & 0x7ff;
	unsigned long mantissa = (valor << 12) >> 12;
	int bias = 1023;

	double resultado = potencia(-1, sinal);
	
	double aux = calculaMantissa(mantissa, 52);
	resultado *= (1 + aux);

	resultado *= potencia(2, expoente - bias);

	return resultado;
}

double round(double valor)
{
	int v = (int) (valor * 1000000 + .5);
	
	return (double) v / 1000000;
}

int getQtdBitsFracao(double fracao, int total)
{
	unsigned long result = 0;

	int qtd = 0;

	int count = 0;
	while (count < total)
	{
		double aux = fracao * 2;
		if (aux == 0)
			break;
		int bit = aux;

		fracao = aux - bit;
		fracao = round(fracao);
		
		result = result << 1;
		result = result | bit;
		qtd += 1;

		count += 1;
	}

	return qtd;
}

unsigned long getBitsFracao(double fracao, int total)
{
	unsigned long result = 0;

	int count = 0;
	while (count < total)
	{
		//printf("fracao = %.15lf\t", fracao);
		double aux = fracao * 2;
		//printf("fracao_aux = %.15lf\t", aux);
		if (aux == 0)
			break;
		
		int bit = aux;

		//printf("%d", bit);
	
		fracao = aux - bit;
		fracao = round(fracao);
		if (fracao == 0)
			fracao = aux - bit;
		//printf("fracao_aux = %.15lf\n", fracao);
		
		result = result << 1;
		result = result | bit;

		count += 1;
	}

	printf("\n");
	return result;
}

int getQtdBitsDeslocamento(int dec, double fracao, int isDouble)
{
	int contar = 0;

	int qtd = 0;
	int i;
	if (dec != 0)
	{
		for (i = 0; i < ARQUITETURA; i++)
		{
			int bit = (dec >> (ARQUITETURA - i - 1)) & 0x1;

			if (contar == 1)
				qtd += 1;

			if (bit == 1)
				contar = 1;
		}
	}
	else
	{	
		unsigned long fBits = 0; 
		int qtdFbits = 0;

		if (isDouble)
		{
			fBits = getBitsFracao(fracao, 52);
			qtdFbits = getQtdBitsFracao(fracao, 52);
		}
		else
		{
			fBits = getBitsFracao(fracao, 23);
			qtdFbits = getQtdBitsFracao(fracao, 23);
		} 

		printf("fBits = %lx (%d bits)\n", fBits, qtdFbits);
		qtd = -1;
		for (i = 0; i < qtdFbits; i++)
		{
			int bit = (fBits >> (qtdFbits - i - 1)) & 0x1;

			if (bit == 1)
				break;
			
			qtd -= 1;
		}
	}	

	return qtd;
}

unsigned long getBitsUm(int tamanho)
{
	unsigned long bits = 0;

	int i;
	for (i = 0; i < tamanho; i++)
	{
		bits = bits << 1;
		bits = bits | 1;
	}

	return bits;
}

unsigned long getMantissa(int decimal, double fracao, int isDouble)
{
	unsigned long fBits = 0; 
	int qtdFbits = 0;

	if (isDouble)
	{
		fBits = getBitsFracao(fracao, 52);
		qtdFbits = getQtdBitsFracao(fracao, 52);
	}
	else
	{
		fBits = getBitsFracao(fracao, 23);
		qtdFbits = getQtdBitsFracao(fracao, 23);
	} 

	unsigned long mantissa = 0;
	int qtdBits = getQtdBitsDeslocamento(decimal, fracao, isDouble);
	//printf("qtdFbits = %d\n", qtdFbits);
	//printf("fBits = %lx\n", fBits);

	if (qtdBits > 0)
	{
		//printf("Decimal = %x - %d\n", decimal, decimal);
		mantissa = decimal & getBitsUm(qtdBits);
		//printf("Mantissa = %lx\n", mantissa);

		if (isDouble)
			mantissa = mantissa << (52 - qtdBits);
		else
			mantissa = mantissa << (23 - qtdBits);

		fBits = fBits >> qtdBits;
		mantissa = mantissa | fBits;
	}
	else
	{
		qtdBits *= -1;

		fBits = fBits << qtdBits;
		fBits = fBits & getBitsUm(qtdFbits);
		//printf("fBits = %lx\n", fBits);

		mantissa = mantissa | fBits;
		//printf("Mantissa = %lx\n", mantissa);
		if (isDouble)
		{
			if (qtdFbits < 52)
				mantissa = mantissa << (52 - qtdFbits);
		}
		else
		{
			if (qtdFbits < 23)
				mantissa = mantissa << (23 - qtdFbits);
		} 
			
	}

	return mantissa;
}

unsigned long convertFlutuanteToBits(double pf, int isDouble)
{
	printf("pf = %.15lf\n", pf);
	int sinal = 0;

	if (pf < 0)
	{
		sinal = 1;
		pf *= -1;
	}
	
	int decimal = pf;
	double fracao = pf - decimal;

	printf("Decimal = %d\n", decimal);
	printf("Fracao = %.15lf\n", fracao);

	int qtdBits = getQtdBitsDeslocamento(decimal, fracao, isDouble);
	printf("qtdBits = %d\n", qtdBits);

	int expoente = 0;
	
	if (isDouble)
		expoente = qtdBits + 1023; 
	else
		expoente = qtdBits + 127;
	
	printf("Expoente: %x\n", expoente);

	unsigned long mantissa = getMantissa(decimal, fracao, isDouble);

	//printf("Mantissa: %lx\n", mantissa);

	unsigned long resultado = sinal;

	if (isDouble)
		resultado = resultado << 11;
	else
		resultado = resultado << 8;
	
	resultado = resultado | expoente;
	
	if (isDouble)
		resultado = resultado << 52;
	else
		resultado = resultado << 23;
	
	resultado = resultado | mantissa;

	return resultado;
}

unsigned int convertFloatToBits(float valor)
{
	unsigned int buf[sizeof(double) /sizeof(unsigned int)];

    memcpy(buf, &valor, sizeof(valor));
  
	return *buf;
}

// https://www.microchip.com/forums/m949093.aspx
unsigned long convertToLongBits(double valor)
{
	unsigned long buf[sizeof(double) /sizeof(unsigned long)];

    memcpy(buf, &valor, sizeof(valor));
  
	return *buf;
}

void escreverArquivo(const char *nomeArquivo, const char *modo, const char* conteudo)
{
	FILE *pFile;
	pFile = fopen(nomeArquivo, modo);
	if (pFile == NULL)
	{
		printf("Erro ao escrever no arquivo.\n");
		exit(1);
	}

	fprintf(pFile, "%s", conteudo);

	fclose(pFile);
}

int isInstrucaoLeitura()
{
	if (!strcmp(nomeInstrucao, "lw") || !strcmp(nomeInstrucao, "lb") || !strcmp(nomeInstrucao, "lbu")
		|| !strcmp(nomeInstrucao, "lwc1") || !strcmp(nomeInstrucao, "ldc1")) 
		return 1;
	
	return 0;
}

int isIntrucaoEscrita()
{
	if (!strcmp(nomeInstrucao, "sw") || !strcmp(nomeInstrucao, "sb") || !strcmp(nomeInstrucao, "swc1"))
		return 1;
	
	return 0;
}

const char *getTipoLog(int isCpu)
{
	if ((isInstrucaoLeitura() || !strcmp(nomeInstrucao, "syscall-r")) && !isCpu)
		return "R";
	
	if (isIntrucaoEscrita() && !isCpu)
		return "W";
	
	return "I";
}

char *getLogTrace(const char *tipoInstrucao, unsigned int endereco, int tamanhoLinha)
{
	char *log = (char *) malloc(sizeof(char) * 33);
	memset(log, 0, 33);

	strcat(log, tipoInstrucao);
	strcat(log, " ");

	char instrucao[12];
	memset(instrucao, 0, 11);
	snprintf(instrucao, 11, "0x%08x", endereco);

	strcat(log, instrucao);
	strcat(log, " ");

	char linhaMemoria[21];
	memset(instrucao, 0, 21);
	unsigned int line = endereco / tamanhoLinha;

	snprintf(linhaMemoria, 20, "(line# 0x%08x)\n", line);

	strcat(log, linhaMemoria);

	return log;
}

void gerarLogTrace(const char *tipoLog, unsigned int endereco, int tamanhoLinha, int isCpu)
{
	char *log = getLogTrace(tipoLog, endereco, tamanhoLinha);

	if (append)
		escreverArquivo("emulador.trace", "a", log);
	else
	{
		escreverArquivo("emulador.trace", "w", log);
		append = 1;
	}

	free(log);
}


void gerarLogDebug(const char *tipoMemoria, char *conteudo)
{
	char buffer[100];
	memset(buffer, 0, 100);

	int i;
	for (i = 0; i < LEVEL; i++)
		strcat(buffer, "\t");

	strcat(buffer, tipoMemoria);
	strcat(buffer, ": ");
	strcat(buffer, conteudo);
	strcat(buffer, "\n");

	if (append)
		escreverArquivo("emulador.trace", "a", buffer);
	else
	{
		escreverArquivo("emulador.trace", "w", buffer);
		append = 1;
	}
}

int getLog2(int valor)
{
	int expoente = 0;

	while (valor != 1)
	{
		expoente += 1;

		valor /= 2; 
	}

	return expoente;
}

void statsGeral(Isa *isa)
{
	printf("Instruction count: %d (", isa->instructionCount);
	printf("R: %u ", isa->countTypeR);
	printf("I: %u ", isa->countTypeI);
	printf("J: %u ", isa->countTypeJ);
	printf("FR: %u  ", isa->countTypeFR);
	printf("FI: %u)\n", isa->countTypeFI);
}

void statsCpu(Isa *isa, double tempo)
{
	printf("Simulation Time: %.2lf sec.\n", tempo);
	printf("Average IPS: %.2lf\n", isa->instructionCount / tempo);
	printf("\n\n");


	printf("Simulated execution times for:\n");
	printf("-----------------------------\n");
	printf("Monocycle\n");
	printf("\tCycles: %u\n", isa->countCycles);
	printf("\tFrequency: %.4lf MHz\n", isa->frequencyMonocycle / 1000000);
	double estimatedTime = isa->countCycles / isa->frequencyMonocycle;
	printf("\tEstimated execution time: %.4lf sec.\n", estimatedTime);
	printf("\tIPC = %.2lf\n", (double) isa->instructionCount / isa->countCycles);
	double mipsMonocycle = (double) isa->instructionCount / (estimatedTime * 1000000);
	printf("\tMIPS = %.2lf\n", mipsMonocycle);


	printf("Pipelined\n");
	printf("\tCycles: %u\n", isa->countCycles + 4);
	printf("\tFrequency: %.4lf MHz\n", isa->frequencyPipelined / 1000000);
	estimatedTime = (isa->countCycles + 4) / isa->frequencyPipelined;
	printf("\tEstimated execution time: %.4lf sec.\n", estimatedTime);
	printf("\tIPC = %.2lf\n", (double) isa->instructionCount / (isa->countCycles + 4));
	double mipsPipelined = (double) isa->instructionCount / (estimatedTime * 1000000);
	printf("\tMIPS = %.2lf\n", mipsPipelined);
	printf("Speedup Monocycle/Pipeline: %.2lfx", (mipsMonocycle) == 0 ? 4 : (mipsPipelined / mipsMonocycle));
	printf("\n\n");
}

void printHitsMisses(unsigned int hits, unsigned int misses)
{
	printf("%12u", hits);
	printf("%14u", misses);
	unsigned int total = hits + misses;
	printf("%14u", total);


	double taxaMiss = (double) misses / total; taxaMiss *= 100;
	printf("%12.2lf%%\n", taxaMiss);
}

void statsMemoria(Memoria *memoria)
{
	printf("Memory Information\n");
	printf("------------------\n");
	printf("Level Hits           Misses        Total         Miss Rate\n");
	printf("-----  ------------  ------------  ------------  ----------\n");

	if (CONFIG == 2)
	{
		printf("%4s  ", "L1");
		printHitsMisses(memoria->cache.hits, memoria->cache.misses);
	}
	else if (CONFIG == 3 || CONFIG == 4 || CONFIG == 5)
	{
		printf("%4s  ", "L1i");
		printHitsMisses(memoria->cacheInstrucao.hits, memoria->cacheInstrucao.misses);
		printf("%4s  ", "L2d");
		printHitsMisses(memoria->cacheData.hits, memoria->cacheData.misses);
	}
	else if (CONFIG == 6)
	{
		printf("%4s  ", "L1i");
		printHitsMisses(memoria->cacheInstrucao.hits, memoria->cacheInstrucao.misses);
		printf("%4s  ", "L2d");
		printHitsMisses(memoria->cacheData.hits, memoria->cacheData.misses);
		printf("%4s  ", "L2");
		printHitsMisses(memoria->cacheL2.hits, memoria->cacheL2.misses);
	}

	printf("%4s  ", "RAM");
	printHitsMisses(memoria->hits, memoria->misses);
}

void estatistica(Memoria *memoria, Isa *isa, double tempo)
{
	statsGeral(isa);
	
	statsCpu(isa, tempo);

	statsMemoria(memoria);
}
