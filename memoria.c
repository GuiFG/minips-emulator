#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memoria.h"
#include "util.h"

char *nomesRegistradores[] = {
	"$zero", "$at",
	"$v0", "$v1",
	"$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$t8", "$t9",
	"$k0", "$k1",
	"$gp", "$sp", "$fp", "$ra",
	"$pc"
};

void zeraText(HashText *text)
{
	int i;
	for (i = 0; i < TAMANHO_TEXT; i++)
		text[i].encadeada = NULL;
}

void zeraData(HashData *data)
{
	int i;
	for (i = 0; i < TAMANHO_DATA; i++)
		data[i].encadeada = NULL;
}

Registrador *criarRegistrador()
{
	Registrador *registrador = (Registrador *) malloc(sizeof(Registrador) * QTD_REGISTRADORES);

	int i;
	for (i = 0; i < QTD_REGISTRADORES; i++)
	{
		registrador[i].nome = nomesRegistradores[i];
		registrador[i].valor = 0;
	}

	registrador[28].valor = ADDR_GLOBAL_POINTER;
	registrador[29].valor = ADDR_STACK_POINTER;
	registrador[32].valor = ENDERECO_TEXT;

	return registrador;
}

Memoria *criarMemoria()
{
	Memoria *memoria = (Memoria *) malloc(sizeof(Memoria));
	if (memoria == NULL)
		return memoria;

	memoria->text = (HashText *) malloc(sizeof(HashText) * TAMANHO_TEXT);
	zeraText(memoria->text);

	memoria->data = (HashData *) malloc(sizeof(HashData) * TAMANHO_DATA);
	zeraData(memoria->data);

	memoria->registradores = criarRegistrador();
	memoria->qtdInstrucoes = 0;

	return memoria;
}

void imprimirCodigo(int *codigo)
{
	int count = 0;
	int i;
	for (i = 0; i < ALINHAMENTO; i++)
	{
		if (count % 2 == 0 && count != 0)
			printf(" ");

		printf("%02x", codigo[i]);
		count += 1;
	}

}

int verificaCodigoZero(int *codigo)
{
	int count = 0;
	int i;
	for (i = 0; i < ALINHAMENTO; i++)
		if (codigo[i] == 0)
			count += 1;

	if (count == ALINHAMENTO)
		return 0;

	return 1;
}

void copiarCodigo(int *dest, int *source)
{
	int i;
	for (i = 0; i < ALINHAMENTO; i++)
		dest[i] = source[i];
}

int getIndexHashText(unsigned long int endereco)
{
	return endereco % TAMANHO_TEXT;
}

int getIndexHashData(unsigned long int endereco)
{
	return endereco % TAMANHO_DATA;
}

Instrucao *criarInstrucao(unsigned long int endereco, int *codigo)
{
	Instrucao *novo = (Instrucao *) malloc(sizeof(Instrucao));
	novo->endereco = endereco;
	novo->codigo = malloc(sizeof(int) * ALINHAMENTO);
	copiarCodigo(novo->codigo, codigo);
	novo->binario = convertCodigoToBin(codigo);
	novo->proximo = NULL;

	return novo;
}

Data *criarData(unsigned long int endereco, int *codigo)
{
	Data *novo = (Data *) malloc(sizeof(Data));
	novo->endereco = endereco;
	novo->codigo = malloc(sizeof(int) * ALINHAMENTO);
	copiarCodigo(novo->codigo, codigo);
	novo->binario = convertCodigoToBin(codigo);
	novo->valor = convertBinToInt(novo->binario);
	novo->proximo = NULL;

	return novo;
}

int salvarInstrucao(Memoria *memoria, unsigned long int endereco, int *codigo)
{
	int index = getIndexHashText(endereco);

	HashText *lista = memoria->text;

	if (lista[index].encadeada == NULL)
	{
		Instrucao *aux = criarInstrucao(endereco, codigo);

		lista[index].encadeada = aux;
	}
	else
	{
		Instrucao *aux = lista[index].encadeada;

		while (aux->proximo != NULL)
			aux = aux->proximo;
		
		Instrucao *novo = criarInstrucao(endereco, codigo);

		aux->proximo = novo;
	}

	memoria->qtdInstrucoes += 1;

	return 1;
}

int salvarData(Memoria *memoria, unsigned long int endereco, int *codigo)
{
	int index = getIndexHashData(endereco);

	HashData *lista = memoria->data;

	if (lista[index].encadeada == NULL)
	{
		Data *aux = criarData(endereco, codigo);

		lista[index].encadeada = aux;
	}
	else
	{
		Data *aux = lista[index].encadeada;

		while (aux->proximo != NULL)
			aux = aux->proximo;
		
		Data *novo = criarData(endereco, codigo);

		aux->proximo = novo;
	}

	return 1;
}

Instrucao *getIntrucao(Memoria *memoria, unsigned long int endereco)
{
	int index = getIndexHashText(endereco);

	Instrucao *aux = memoria->text[index].encadeada;
	
	while (aux != NULL)
	{
		if (aux->endereco == endereco)
			return aux;
		
		aux = aux->proximo;
	}

	printf("getInstrucao - retornando NULL.\t addr= 0x%08lx\n", endereco);
	return NULL;
}


Data *getData(Memoria *memoria, unsigned long int endereco)
{
	HashData *lista = memoria->data;

	int index = getIndexHashData(endereco);

	if (lista[index].encadeada == NULL) 
		return NULL;

	Data *aux = lista[index].encadeada;

	while(aux != NULL)
	{
		if (aux->endereco == endereco)
			return aux;
		
		aux = aux->proximo;
	}

	return NULL;
}	

void imprimirLista(Data *no)
{
	if (no == NULL)
		return;
	
	printf("Endereco: 0x%08lx\t", no->endereco);
	imprimirLista(no->proximo);
}

void imprimirData(HashData *data)
{
	int i;
	for (i = 0; i < TAMANHO_DATA; i++)
	{
		imprimirLista(data[i].encadeada);
		if (data[i].encadeada != NULL) printf("\n");
	}
}

char *getDataBinario(Memoria *memoria, unsigned long int endereco)
{
	Data *data = getData(memoria, endereco);
	if (data == NULL) 
	{
		setDataMemoria(memoria, endereco, 0);
		return getDataBinario(memoria, endereco);
	}

	return data->binario;
}

int *getDataCodigo(Memoria *memoria, unsigned long int endereco)
{
	Data *data = getData(memoria, endereco);
	if (data == NULL) 
	{
		setDataMemoria(memoria, endereco, 0);
		return getDataCodigo(memoria, endereco);
	}

	return data->codigo;
}

unsigned long int getDataValor(Memoria *memoria, unsigned long int endereco)
{
	Data *data = getData(memoria, endereco);
	if (data == NULL) 
	{
		setDataMemoria(memoria, endereco, 0);
		return getDataValor(memoria, endereco);
	}

	return data->valor;
}

void setCodigo(Data *data, unsigned long int valor)
{
	char *bin = convertValorToBin(valor);

	int byteSize = 8;

	char byte[9]; byte[8] = 0;

	int j = 0;
	int i;
	for (i = 0; i < ARQUITETURA; i++)
	{
		if (i % byteSize == 0 && i != 0)
		{
			int codigo = convertBinToInt(byte);
			data->codigo[j] = codigo;
			j++;
		}

		byte[i % byteSize] = bin[i];
	}

	int codigo = convertBinToInt(byte);
	data->codigo[j] = codigo;

	free(bin);
}

void setDataMemoria(Memoria *memoria, unsigned long int endereco, unsigned long int valor)
{
	Data *data = getData(memoria, endereco);
	if (data == NULL) 
	{
		char *bin = convertValorToBin(valor);
		int *codigo = convertBinToCodigo(bin);
		salvarData(memoria, endereco, codigo);

		free(bin);
		free(codigo);
		return;
	}

	setCodigo(data, valor);
	convertCodigoToBinario(data->codigo, data->binario);
	data->valor = valor;
}

char *getNomeRegistrador(int numero)
{
	return nomesRegistradores[numero];
}

void liberarText(Instrucao *no)
{
	if (no == NULL)
		return;

	liberarText(no->proximo);

	free(no->codigo);
	free(no->binario);
	free(no);
}

void liberarData(Data *no)
{
	if (no == NULL)
		return;
	
	liberarData(no->proximo);

	free(no->codigo);
	free(no->binario);
	free(no);
}

void liberarMemoria(Memoria *memoria)
{
	int i;
	for (i = 0; i < TAMANHO_TEXT; i++)
		liberarText(memoria->text[i].encadeada);

	for (i = 0; i < TAMANHO_DATA; i++)
		liberarData(memoria->data[i].encadeada);

	free(memoria->registradores);
	free(memoria->text);
	free(memoria->data);
	free(memoria);
}