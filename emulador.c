#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memoria.h"
#include "isa.h"
#include "util.h"

char *getArquivoText(char *arquivo);
int carregarText(Memoria *memoria, char *arquivo);
int carregarData(Memoria *memoria, char *arquivo);
Memoria *carregarMemoria(char *arquivo);
void run(Memoria *memoria);
void decode(Memoria *memoria);

int main(int argc, char *argv[])
{
	char *modo;
	char *nomeArquivo;
	if (argc == 3)
	{
		modo = argv[1];
		nomeArquivo = argv[2];
	} else return 1;

	Memoria *memoria = carregarMemoria(nomeArquivo);
	if (!memoria) return 1;

	if (!strcmp(modo, "decode")) decode(memoria);
	else if (!strcmp(modo, "run")) run(memoria);

	return 0;
}

char *getArquivoText(char *arquivo)
{
	int size = strlen(arquivo) + strlen(".text") + 1;

	char *aux = (char *) malloc(sizeof(char) * size);
	copiarString(aux, arquivo);
	strcat(aux, ".text");

	aux[size] = '\0';

	return aux;
}

char *getArquivoData(char *arquivo)
{
	int size = strlen(arquivo) + strlen(".data") + 1;

	char *aux = (char *) malloc(sizeof(char) * size);
	copiarString(aux, arquivo);
	strcat(aux, ".data");

	aux[size] = '\0';

	return aux;
}

int carregarText(Memoria *memoria, char *arquivo)
{
	FILE *file;
	char *fileText = getArquivoText(arquivo);
	file = fopen(fileText, "rb");
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
			convertToBigEndian(&word, ALINHAMENTO);
			if (!salvarInstrucao(memoria, ENDERECO_TEXT + posicao, word)) lendoInstrucao = 0;
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
	char *fileData = getArquivoData(arquivo);
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
			convertToBigEndian(&word, ALINHAMENTO);
			if (!salvarData(memoria, ENDERECO_DATA + posicao, word)) return 0;
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

	return memoria;
}

void run(Memoria *memoria)
{
	Isa *isa = criarISA();

	Registrador *pc = &memoria->registradores[32];

	int i;
	int executando = 1;
	int count = 0;
	while(executando)
	{
		Instrucao *inst = getIntrucao(memoria, pc->valor);
		if (inst == NULL) break;
		
		// printf("0x%08lx\n", pc->valor);
		if (!executarCodigo(memoria, isa, inst->binario))
			executando = 0;
	}

	liberarISA(isa);
	liberarMemoria(memoria);
}

void decode(Memoria *memoria)
{
	Isa *isa = criarISA();

	int i;
	for (i = 0; i < memoria->qtdInstrucoes; i++)
	{
		Instrucao *inst = getIntrucao(memoria, ENDERECO_TEXT + (ALINHAMENTO * i));
		if (inst == NULL) break;

		printInstrucaoISA(isa, inst->binario);
	}


	liberarISA(isa);
	liberarMemoria(memoria);
}