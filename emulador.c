#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memoria.h"
#include "isa.h"
#include "util.h"
#include <time.h>

char *getArquivoText(char *arquivo);
int carregarText(Memoria *memoria, char *arquivo);
int carregarData(Memoria *memoria, char *arquivo);
int carregarRodata(Memoria *memoria, char *arquivo);
Memoria *carregarMemoria(char *arquivo);
void run(Memoria *memoria);
void decode(Memoria *memoria);


char CONFIG = 0;
char TRACE = 0;
char DEBUG = 0;

int main(int argc, char *argv[])
{
	char *modo;
	char *nomeArquivo;
	if (argc == 3)
	{
		modo = argv[1];
		nomeArquivo = argv[2];
	} 
	else if (argc == 4)
	{
		modo = argv[1];
		CONFIG = argv[2][0] - '0';
		nomeArquivo = argv[3];
	} 
	else return 1;

	Memoria *memoria = carregarMemoria(nomeArquivo);
	if (!memoria) return 1;

	if (!strcmp(modo, "decode")) decode(memoria);
	else if (!strcmp(modo, "run")) run(memoria);
	else if (!strcmp(modo, "trace"))
	{
		TRACE = 1;
		run(memoria);
	} else if (!strcmp(modo, "debug"))
	{
		TRACE = 1;
		DEBUG = 1;
		run(memoria);
	}

	return 0;
}

char *getArquivo(char *arquivo, const char *extensao)
{
	int size = strlen(arquivo) + strlen(extensao) + 1;

	char *aux = (char *) malloc(sizeof(char) * size);
	memset(aux, 0, size);

	copiarString(aux, arquivo);
	strcat(aux, extensao);

	return aux;
}

int carregarText(Memoria *memoria, char *arquivo)
{
	char *fileText = getArquivo(arquivo, ".text");
	FILE *file = fopen(fileText, "rb");
	if (!file)
	{
		printf("Não foi possivel abrir o arquivo .text\n");
		free(fileText);
		return 0;
	}

	unsigned int byte = 0;

	int count = 0;
	int *word = (int *) malloc(sizeof(int) * ALINHAMENTO);
	memset(word, 0, ALINHAMENTO * sizeof(int));

	int posicao = 0;

	int lendoInstrucao = 1;
	while (lendoInstrucao)
	{
		if ((byte = fgetc(file)) == EOF)
			lendoInstrucao = 0;

		if (count % ALINHAMENTO == 0 && count != 0)
		{
			convertToLittleEndian(&word, ALINHAMENTO);
			unsigned int valor = getValorWord(word);
			if (!salvarData(memoria, ENDERECO_TEXT + posicao, valor)) lendoInstrucao = 0;
			memset(word, 0, sizeof(word));
			
			posicao += ALINHAMENTO;
		}

		if (byte != EOF) word[count % ALINHAMENTO] = byte;

		count += 1;
	}

	free(fileText);
	free(word);
	fclose(file);

	return 1;
}

int carregarData(Memoria *memoria, char *arquivo)
{
	FILE *file;
	char *fileData = getArquivo(arquivo, ".data");
	
	file = fopen(fileData, "rb");
	if (!file)
	{
		printf("Não foi possivel abrir o arquivo data.\n");
		free(fileData);
		return 0;
	}

	unsigned int byte = 0;

	int count = 0;
	int *word = (int *) malloc(sizeof(int) * ALINHAMENTO);
	memset(word, 0, ALINHAMENTO * sizeof(int));

	int posicao = 0;

	int lendoDados = 1;
	while (lendoDados)
	{
		if ((byte = fgetc(file)) == EOF)
			lendoDados = 0;

		if (count % ALINHAMENTO == 0 && count != 0)
		{
			convertToLittleEndian(&word, ALINHAMENTO);
			unsigned int valor = getValorWord(word);
			if (!salvarData(memoria, ENDERECO_DATA + posicao, valor)) return 0;
			memset(word, 0, sizeof(word));

			posicao += ALINHAMENTO;
		}

		if (byte != EOF) word [count % ALINHAMENTO] = byte;

		count += 1;
	}

	free(fileData);
	free(word);
	fclose(file);

	return 1;
}

int carregarRodata(Memoria *memoria, char *arquivo)
{
	FILE *file;	
	char *fileRodata = getArquivo(arquivo, ".rodata");

	file = fopen(fileRodata, "rb");
	if (!file)
	{
		free(fileRodata);
		return 0;
	}

	unsigned int byte = 0;

	int count = 0;
	int *word = (int *) malloc(sizeof(int) * ALINHAMENTO);
	memset(word, 0, ALINHAMENTO * sizeof(int));

	int posicao = 0;

	int lendoDados = 1;
	while (lendoDados)
	{
		if ((byte = fgetc(file)) == EOF)
			lendoDados = 0;

		if (count % ALINHAMENTO == 0 && count != 0)
		{
			convertToLittleEndian(&word, ALINHAMENTO);
			unsigned int valor = getValorWord(word);
			if (!salvarData(memoria, ENDERECO_RODATA + posicao, valor)) return 0;
			memset(word, 0, sizeof(word));

			posicao += ALINHAMENTO;
		}

		if (byte != EOF) word [count % ALINHAMENTO] = byte;

		count += 1;
	}

	free(fileRodata);
	free(word);
	fclose(file);

	return 1;
}

Memoria *carregarMemoria(char *arquivo)
{
	Memoria *memoria = criarMemoria();
	if (memoria == NULL)
	{
		printf("Não foi possivel criar a memoria\n");
		return NULL;
	}

	if (!carregarText(memoria, arquivo))
	{
		liberarMemoria(memoria);
		return NULL;
	}

	if (!carregarData(memoria, arquivo))
	{
		liberarMemoria(memoria);
		return NULL;
	}

	carregarRodata(memoria, arquivo);

	return memoria;
}

void run(Memoria *memoria)
{
	Isa *isa = criarISA();

	Registrador *pc = &memoria->registradores[32];

	clock_t inicio = clock();
	int executando = 1; 
	while(executando)
	{
		unsigned int binario = 0;
		
		if (TRACE)
			gerarLogTrace("I", pc->valor, memoria->tamanhoLinha, 1);	

		if (CONFIG == 0 || CONFIG == 1)
			binario = getDataValor(memoria, pc->valor, &isa->countCycles);
		else if (!strcmp(memoria->modoCache, "unificada"))
			binario = getDataCache(memoria, pc->valor, "geral", 1, 1, &isa->countCycles);
		else 
			binario = getDataCache(memoria, pc->valor, "instrucao", 1, 1, &isa->countCycles);

		if (binario == -1) break;
		
		executando = executarCodigo(memoria, isa, binario);

		if (executando == 10 || executando == 0)
			break;	
	}

	if (executando == 10)
	{
		printf("\033[0;32m");
		printf("\nExecution finished successfully\n--------------------------\n");
		double tempoTotal = (double) (clock() - inicio) / CLOCKS_PER_SEC;
		estatistica(memoria, isa, tempoTotal);
	}
	

	liberarISA(isa);
	liberarMemoria(memoria);
}

void decode(Memoria *memoria)
{
	Isa *isa = criarISA();

	Registrador *pc = &memoria->registradores[32];

	int i = 0;
	while(i < memoria->qtdInstrucoes)
	{
		unsigned int valor = getDataValor(memoria, pc->valor, NULL);
		if (valor == -1) break;

		printInstrucaoISA(memoria, isa, valor);

		i += 1;
	}

	liberarISA(isa);
	liberarMemoria(memoria);
}
