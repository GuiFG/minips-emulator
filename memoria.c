#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memoria.h"
#include "util.h"

char LEVEL = 1;

char *nomesRegistradores[] = {
	"$zero", "$at",
	"$v0", "$v1",
	"$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$t8", "$t9",
	"$k0", "$k1",
	"$gp", "$sp", "$fp", "$ra",
	"$pc",
	"$f0", "$f1", "$f2",
	"$f3", "$f4", "$f5", "$f6", "$f7", "$f8", "$f9", "$f10",
	"$f11", "$f12", "$f13", "$f14",
	"$f15", "$f16", "$f17", "$f18",
	"$f19", "$f20", "$f21", "$f22", "$f23", "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30",
	"hi", "lo",
	"cc"
};

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

Cache criarCache(const char *tipo, const char *politica, int capacidade, int tamanhoLinha, int qtdVias)
{
	Cache cache; 
	cache.tipo = tipo;
	cache.politica = politica;
	cache.capacidade = capacidade;
	cache.hits = 0;
	cache.misses = 0;
	
	cache.qtdVias = qtdVias;
	cache.qtdSets = cache.capacidade / ( tamanhoLinha * cache.qtdVias );
	cache.tamanhoLinha = tamanhoLinha;
	
	Set *sets = (Set *) malloc(sizeof(Set) * cache.qtdSets);
	int i, j;
	for (i = 0; i < cache.qtdSets; i++)
	{
		sets[i].lines = (Line *) malloc(sizeof(Line) * cache.qtdVias);
		for (j = 0; j < cache.qtdVias; j++)
		{
			sets[i].lines[j].valido = 0;
			sets[i].lines[j].clean = 1;
			sets[i].lines[j].idade = 0;
			sets[i].lines[j].tag = 0;
			sets[i].lines[j].data = (unsigned int *) malloc(sizeof(unsigned int) * (cache.tamanhoLinha / ALINHAMENTO));
			memset(sets[i].lines[j].data, 0, sizeof(unsigned int)  * (cache.tamanhoLinha / ALINHAMENTO));
		}
	}
	cache.sets = sets;

	cache.byteOffset = getLog2(ALINHAMENTO);
	cache.blockOffset = getLog2(cache.tamanhoLinha / ALINHAMENTO);
	cache.lineIndexOffset = getLog2(cache.qtdSets);

	return cache;
}

Memoria *criarMemoria()
{
	Memoria *memoria = (Memoria *) malloc(sizeof(Memoria));
	if (memoria == NULL)
		return memoria;

	memoria->data = (HashData *) malloc(sizeof(HashData) * TAMANHO_DATA);
	zeraData(memoria->data);

	memoria->registradores = criarRegistrador();
	memoria->qtdInstrucoes = 0;

	memoria->hits= 0;
	memoria->misses = 0;

	
	memoria->tamanhoLinha = 32;
	
	if (CONFIG == 2)
	{
		memoria->modoCache = "unificada";
		memoria->cache = criarCache("unificada", "random", 1024, 32, 1);
	} 
	else if (CONFIG == 3)
	{
		memoria->modoCache = "split";
		memoria->cacheInstrucao = criarCache("instrucao", "random", 512, 32, 1);
		memoria->cacheData = criarCache("dados", "random", 512, 32, 1);
	}
	else if (CONFIG == 4)
	{
		memoria->modoCache = "split";
		memoria->cacheInstrucao = criarCache("instrucao", "LRU", 512, 32, 1);
		memoria->cacheData = criarCache("dados", "LRU", 512, 32, 1);
	}
	else if (CONFIG == 5)
	{
		memoria->modoCache = "split";
		memoria->cacheInstrucao = criarCache("instrucao", "LRU", 512, 32, 4);
		memoria->cacheData = criarCache("dados", "LRU", 512, 32, 4);
	}
	else if (CONFIG == 6)
	{
		memoria->tamanhoLinha = 64;
		memoria->modoCache = "split";
		memoria->modoCacheL2 = "unificada";
		memoria->cacheInstrucao = criarCache("instrucao", "LRU", 512, 64, 4);
		memoria->cacheData = criarCache("dados", "LRU", 512, 64, 4);
		memoria->cacheL2 = criarCache("unificada", "LRU", 2048, 64, 8);
	}


	return memoria;
}

int getIndexHashData(unsigned int endereco)
{
	return endereco % TAMANHO_DATA;
}

Data *criarData(unsigned int endereco, unsigned int valor)
{
	Data *novo = (Data *) malloc(sizeof(Data));
	novo->endereco = endereco;
	novo->valor = valor;
	novo->proximo = NULL;

	return novo;
}

int isText(unsigned int endereco)
{
	int dif = endereco - ENDERECO_TEXT;

	if (dif >= 0 && dif <= 10000) return 1;

	return 0;
}

int salvarData(Memoria *memoria, unsigned int endereco, unsigned int valor)
{
	int index = getIndexHashData(endereco);

	HashData *lista = memoria->data;

	if (lista[index].encadeada == NULL)
	{
		Data *aux = criarData(endereco, valor);

		lista[index].encadeada = aux;
	}
	else
	{
		Data *aux = lista[index].encadeada;

		while (aux->proximo != NULL)
			aux = aux->proximo;
		
		Data *novo = criarData(endereco, valor);

		aux->proximo = novo;
	}

	if (isText(endereco))
		memoria->qtdInstrucoes += 1;

	return 1;
}

Data *getData(Memoria *memoria, unsigned int endereco)
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

unsigned int getDataValor(Memoria *memoria, unsigned int endereco, unsigned int *cycles)
{
	if (cycles != NULL)
	{	
		*cycles += CycleTimeRAM;
		memoria->hits += 1;
		if (DEBUG)
		{
			char buffer[50];
			memset(buffer, 0, 50);
			snprintf(buffer, 50, "read line# 0x%08x", endereco / memoria->tamanhoLinha);
			gerarLogDebug("RAM", buffer);
			snprintf(buffer, 50, "Hit");
			gerarLogDebug("RAM", buffer);
		}
			
	}

	Data *data = getData(memoria, endereco);
	if (data == NULL) 
	{
		setDataMemoria(memoria, endereco, 0, cycles, 0);
		return getDataValor(memoria, endereco, NULL);
	}

	return data->valor;
}

void setDataMemoria(Memoria *memoria, unsigned int endereco, unsigned int valor, unsigned int *cycles, char log)
{
	if (cycles != NULL)
	{
		*cycles += CycleTimeRAM;

		if (log)
			memoria->hits += 1;

		if (DEBUG && log)
		{
			char buffer[50];
			memset(buffer, 0, 50);
			snprintf(buffer, 50, "write: 0x%08x", endereco / memoria->tamanhoLinha);
			gerarLogDebug("RAM", buffer);
			snprintf(buffer, 50, "Hit");
			gerarLogDebug("RAM", buffer);
		}
	}
		
	Data *data = getData(memoria, endereco);
	if (data == NULL) 
	{		
		salvarData(memoria, endereco, valor);
		return;
	}

	data->valor = valor;
}

void destruirLine(Line *line)
{
	if (line != NULL)
	{
		if (line->data != NULL)
			free(line->data);
		free(line);
	}
}

Line *criarLine(int size)
{
	Line *line = (Line *) malloc(sizeof(Line));
	line->valido = 1;
	line->clean = 1;
	line->idade = 0;

	line->tag = 0;
	line->data = (unsigned int *) malloc(sizeof(unsigned int) * size);
	memset(line->data, 0, sizeof(unsigned int) * size);

	return line;
}

Line *getLineNivelSuperior(Memoria *memoria, Cache *cache, unsigned int endereco, int nivel, unsigned int *cycles)
{
	int blockOffset = (endereco >> cache->byteOffset) & getBitsUm(cache->blockOffset);
	if (blockOffset > 0)
		endereco -= (blockOffset * ALINHAMENTO);
	

	if (DEBUG)
		LEVEL = nivel + 1;

	unsigned int binario = 0;
	if (CONFIG != 6)
	{
		binario = getDataValor(memoria, endereco, cycles);
	}
	else
	{
		if (nivel == 1)
			binario = getDataCache(memoria, endereco, cache->tipo, nivel + 1, 1, cycles);
		else if (nivel == 2)
			binario = getDataValor(memoria, endereco, cycles);
	}

	int size = cache->tamanhoLinha / ALINHAMENTO;
	Line *line = criarLine(size);
	line->data[0] = binario;

	for (int i = 1; i < size; i++)
	{
		if (CONFIG != 6)
			binario = getDataValor(memoria, endereco + (ALINHAMENTO * i), NULL);
		else
		{
			if (nivel == 1)
				binario = getDataCache(memoria, endereco + (ALINHAMENTO * i), cache->tipo, nivel + 1, 0, NULL);
			else if (nivel == 2)
				binario = getDataValor(memoria, endereco + (ALINHAMENTO * i), NULL);
		}

		line->data[i] = binario;
	}

	unsigned int addrLine = endereco / cache->tamanhoLinha;
	line->tag = addrLine >> cache->lineIndexOffset;

	return line;
}

Line *getLine(Set *set, int qtdVias, unsigned int tag, int *index)
{
	int i;
	for (i = 0; i < qtdVias; i++)
	{
		if (set->lines[i].valido)
		{
			
			if (set->lines[i].tag == tag)
			{
				if (index != NULL)
					*index = i;
				
				return &set->lines[i];
			}
				
		}	
	}
	
	return NULL;
}

Line *getLineCacheIrma(Memoria *memoria, Cache *cache, unsigned int endereco)
{
	if (!strcmp(cache->tipo, "instrucao"))
		cache = &memoria->cacheData;
	else if (!strcmp(cache->tipo, "dados"))
		cache = &memoria->cacheInstrucao;
	
	unsigned int addrLine = endereco / cache->tamanhoLinha;
	int indexSet = addrLine & getBitsUm(cache->lineIndexOffset);
	unsigned int tagSet = addrLine >> (cache->lineIndexOffset);

	return getLine(&cache->sets[indexSet], cache->qtdVias, tagSet, NULL);
}

unsigned int getDataLine(Cache *cache, Line *line, unsigned int endereco)
{
	int blockOffset = (endereco >> cache->byteOffset) & getBitsUm(cache->blockOffset);

	return line->data[blockOffset];
}

void copyLine(Line *dest, Line *source, int tamanho)
{
	dest->valido = source->valido;
	dest->clean = source->clean;
	dest->tag = source->tag;
	
	int i;
	for (i = 0; i < tamanho; i++)
		dest->data[i] = source->data[i];
}

void enviarLineNivelSuperior(Memoria *memoria, Line *line, unsigned int endereco, int tamanho, int nivel, unsigned int *cycles)
{
	if (DEBUG)
		LEVEL = nivel + 1;

	if (CONFIG != 6)
	{
		setDataMemoria(memoria, endereco, line->data[0], cycles, 1);
	}
	else 
	{
		if (nivel == 1)
			setDataCache(memoria, endereco, line->data[0], nivel + 1, 1, cycles);
		else if (nivel == 2)
			setDataMemoria(memoria, endereco, line->data[0], cycles, 1);
	}

	int i;
	for (i = 1; i < tamanho; i++)
	{
		if (CONFIG != 6)
			setDataMemoria(memoria, endereco + (ALINHAMENTO * i), line->data[i], NULL, 0);
		else
		{
			if (nivel == 1)
				setDataCache(memoria, endereco + (ALINHAMENTO * i), line->data[i], nivel + 1, 0, cycles);
			else if (nivel == 2)
				setDataMemoria(memoria, endereco + (ALINHAMENTO * i), line->data[i], NULL, 0);
		}
	}

	line->valido = 1;
	line->clean = 1;
}

Line *escolherLineLRU(Set *set, int qtdVias, int *indexVia)
{
	int i;

	int maior = 0;
	for (i = 1; i < qtdVias; i++)
	{
		if (set->lines[i].idade > set->lines[maior].idade)
			maior = i;	
	}

	*indexVia = maior;
	return &set->lines[maior];
}

void mudarIdadeLine(Set *set, int qtdVias, int indexVia)
{
	int i;
	for (i = 0; i < qtdVias; i++)	
		set->lines[i].idade += 1;
	
	set->lines[indexVia].idade = 0;
}

const char *getTipoMemoria(const char* tipoCache, int nivel)
{
	if (!strcmp(tipoCache, "unificada"))
	{
		if (nivel == 1)
			return "L1";
		else if (nivel == 2)
			return "L2";
	}
	else if (!strcmp(tipoCache, "instrucao"))
		return "L1i";
	else if (!strcmp(tipoCache, "dados"))
		return "L1d";
}

Line *replaceLine(Memoria *memoria, Cache *cache, unsigned int endereco, int nivel, int *indexVia, unsigned int *cycles)
{
	unsigned int addrLine = endereco / cache->tamanhoLinha;
	int indexSet = addrLine & getBitsUm(cache->lineIndexOffset);
	unsigned int tagSet = addrLine >> (cache->lineIndexOffset);

	Line *novaLine = NULL;

	if (!strcmp(memoria->modoCache, "split"))
	{
		Line *aux = getLineCacheIrma(memoria, cache, endereco);
		if (aux != NULL)
		{		
			int size = cache->tamanhoLinha / ALINHAMENTO;
			novaLine = criarLine(size);
			copyLine(novaLine, aux, size);

			novaLine->valido = 1;
			novaLine->tag = tagSet;

			if (DEBUG)
			{
				const char *tipoMemoria = getTipoMemoria(cache->tipo, nivel);
				char buffer[50]; memset(buffer, 0, 50);
				
				const char *tmp;
				if (!strcmp(tipoMemoria, "L1d"))
					tmp = "L1i";
				else 
					tmp = "L1d";

				snprintf(buffer, 50, "Line found on %s", tmp);
				gerarLogDebug(tipoMemoria, buffer);
			}
		}
	}

	if (novaLine == NULL)
	{
		novaLine = getLineNivelSuperior(memoria, cache, endereco, nivel, cycles);
	}
		
	if (DEBUG)
	{	
		LEVEL = nivel;
		const char *tipoMemoria = getTipoMemoria(cache->tipo, nivel);
		char buffer[50]; memset(buffer, 0, 50);
		snprintf(buffer, 50, "Replace to include line# 0x%08x", endereco / memoria->tamanhoLinha);
		gerarLogDebug(tipoMemoria, buffer);		
	}

	Line *antigaLine = NULL;
	int policy = 0;
	if (!strcmp(cache->politica, "LRU"))
	{
		antigaLine = escolherLineLRU(&cache->sets[indexSet], cache->qtdVias, indexVia);
		policy = 1;
	}
	else 
	{
		antigaLine = &cache->sets[indexSet].lines[0];
	}

	if (DEBUG)
	{
		const char *tipoMemoria = getTipoMemoria(cache->tipo, nivel);
		char buffer[50]; memset(buffer, 0, 50);
		
		if (policy)
			snprintf(buffer, 50, "LRU replacement policy. Way#%d", *indexVia);
		else
			snprintf(buffer, 50, "Random replacement policy. Way#%d", 0);
		
		gerarLogDebug(tipoMemoria, buffer);
		
		memset(buffer, 0, 50);
		if (antigaLine->clean)
			snprintf(buffer, 50, "Line clean/invalid. No need to write back.");
		else
		{
			unsigned int addr = antigaLine->tag << (cache->lineIndexOffset);
			addr = addr | indexSet;
			snprintf(buffer, 50, "Writing back line: 0x%08x", addr);
		}
			

		gerarLogDebug(tipoMemoria, buffer);
	}
	
	if (!antigaLine->clean)
	{
		unsigned int addr = antigaLine->tag << (cache->lineIndexOffset);
		addr = addr | indexSet;
		addr = addr * cache->tamanhoLinha;

		enviarLineNivelSuperior(memoria, antigaLine, addr, cache->tamanhoLinha / ALINHAMENTO, nivel, cycles);
	}

	copyLine(antigaLine, novaLine, cache->tamanhoLinha / ALINHAMENTO);
	destruirLine(novaLine);

	return antigaLine;
}

unsigned int getDataCache(Memoria *memoria, unsigned int endereco, const char *tipoCache, int nivel, char count, unsigned int *cycles)
{		
	Cache *cache = NULL;
	if (nivel == 1)
	{	
		if (!strcmp(memoria->modoCache, "unificada"))
			cache = &memoria->cache;
		else if (!strcmp(tipoCache, "instrucao"))
			cache = &memoria->cacheInstrucao;
		else if (!strcmp(tipoCache, "dados"))
			cache = &memoria->cacheData;
	} 
	else if (nivel == 2)	
		cache = &memoria->cacheL2;

	const char *tipoMemoria;
	if (cycles != NULL)
	{
		if (nivel == 1)
		{
			*cycles += CycleTimeL1;

			if (DEBUG)
			{
				LEVEL = 1;
				tipoMemoria = getTipoMemoria(cache->tipo, nivel);
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "read line# 0x%08x", endereco / memoria->tamanhoLinha);
				gerarLogDebug(tipoMemoria, buffer);
			}
		}
		else if (nivel == 2)
		{
			*cycles += CycleTimeL2;

			if (DEBUG)
			{
				LEVEL = 2;
				tipoMemoria = getTipoMemoria(cache->tipo, nivel);
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "read line# 0x%08x", endereco / cache->tamanhoLinha);
				gerarLogDebug(tipoMemoria, buffer);
			}
		}
			
	}

	unsigned int addrLine = endereco / cache->tamanhoLinha;

	int indexSet = addrLine & getBitsUm(cache->lineIndexOffset);
	unsigned int tagSet = addrLine >> (cache->lineIndexOffset);

	int indexVia = 0;
	Line *line = getLine(&cache->sets[indexSet], cache->qtdVias, tagSet, &indexVia);
	if (line == NULL)
	{	
		if (count)
		{
			cache->misses += 1;

			if (DEBUG)
			{
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "Miss");
				gerarLogDebug(tipoMemoria, buffer);
			}
		}
		
		line = replaceLine(memoria, cache, endereco, nivel, &indexVia, cycles);
	} 
	else 
	{
		if (count)
		{
			cache->hits += 1;

			if (DEBUG)
			{
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "Hit");
				gerarLogDebug(tipoMemoria, buffer);
			}
		}
			
	}

	if (!strcmp(cache->politica, "LRU"))
		mudarIdadeLine(&cache->sets[indexSet], cache->qtdVias, indexVia);

	unsigned int data = getDataLine(cache, line, endereco);

	return data;
}

void setDataLine(Cache *cache, Line *line, unsigned int endereco, unsigned int valor)
{
	int blockOffset = (endereco >> cache->byteOffset) & getBitsUm(cache->blockOffset);

	line->data[blockOffset] = valor;

	line->clean = 0;	
}

void setDataCache(Memoria *memoria, unsigned int endereco, unsigned int valor, int nivel, char count, unsigned int *cycles)
{
	Cache *cache = NULL;
	if (!strcmp(memoria->modoCache, "split"))
		cache = &memoria->cacheData;
	else 
		cache = &memoria->cache;
	
	const char *tipoMemoria;
	if (nivel == 1)
	{
		*cycles += CycleTimeL1;
		if (DEBUG)
		{
			LEVEL = 1;
			tipoMemoria = getTipoMemoria(cache->tipo, nivel);
			char buffer[50]; memset(buffer, 0, 50);
			snprintf(buffer, 50, "write: 0x%08x", endereco / memoria->tamanhoLinha);
			gerarLogDebug(tipoMemoria, buffer);
		}
	}
	else if (nivel == 2)
	{
		*cycles += CycleTimeL2;
		if (DEBUG)
		{
			LEVEL = 2;
			tipoMemoria = getTipoMemoria(cache->tipo, nivel);
			char buffer[50]; memset(buffer, 0, 50);
			snprintf(buffer, 50, "write: 0x%08x", endereco / cache->tamanhoLinha);
			gerarLogDebug(tipoMemoria, buffer);
		}
	}

	unsigned int addrLine = endereco / cache->tamanhoLinha;
	int indexSet = addrLine & getBitsUm(cache->lineIndexOffset);
	unsigned int tagSet = addrLine >> (cache->lineIndexOffset);

	int indexVia = 0;
	Line *line = getLine(&cache->sets[indexSet], cache->qtdVias, tagSet, &indexVia);
	if (line == NULL)
	{	
		if (count)
		{
			cache->misses += 1;

			if (DEBUG)
			{
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "Miss");
				gerarLogDebug(tipoMemoria, buffer);
			}
		}

		line = replaceLine(memoria, cache, endereco, nivel, &indexVia, cycles);
	} 
	else 
	{
		if (count)
		{
			cache->hits += 1;

			if (DEBUG)
			{
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "Hit");
				gerarLogDebug(tipoMemoria, buffer);

				memset(buffer, 0, 50);
				snprintf(buffer, 50, "updating line# 0x%08x", endereco / cache->tamanhoLinha);
				gerarLogDebug(tipoMemoria, buffer);
			}
		}
	}

	if (!strcmp(cache->politica, "LRU"))
		mudarIdadeLine(&cache->sets[indexSet], cache->qtdVias, indexVia);
	
	setDataLine(cache, line, endereco, valor);

	if (!strcmp(memoria->modoCache, "split"))
	{
		Line *irma = getLineCacheIrma(memoria, cache, endereco);
		if (irma != NULL)
		{	
			irma->valido = 0;
			irma->clean = 1;

			if (DEBUG)
			{
				const char *tmp;
				if (!strcmp(tipoMemoria, "L1d"))
					tmp = "L1i";
				
				char buffer[50]; memset(buffer, 0, 50);
				snprintf(buffer, 50, "invalidating line 0x%08x", endereco / memoria->tamanhoLinha);
				gerarLogDebug(tmp, buffer);
			}
		}
	}		
}

char *getNomeRegistrador(int numero)
{
	return nomesRegistradores[numero];
}

void liberarData(Data *no)
{
	if (no == NULL)
		return;
	
	liberarData(no->proximo);

	free(no);
}

void liberarLine(Line *line, int qtdVias)
{
	int i;
	for (i = 0; i < qtdVias; i++)
		free(line[i].data);
	
	free(line);
}

void liberarSets(Set *sets, int qtdSets, int qtdVias)
{
	if (sets == NULL)
		return;
		
	int i;
	for (i = 0; i < qtdSets; i++)
		liberarLine(sets[i].lines, qtdVias);
	
	free(sets);
}

void liberarCache(Cache cache)
{
	liberarSets(cache.sets, cache.qtdSets, cache.qtdVias);
}

void liberarMemoria(Memoria *memoria)
{
	int i;
	for (i = 0; i < TAMANHO_DATA; i++)
		liberarData(memoria->data[i].encadeada);
	
	liberarCache(memoria->cache);
	liberarCache(memoria->cacheInstrucao);
	liberarCache(memoria->cacheData);
	liberarCache(memoria->cacheL2);

	free(memoria->registradores);
	free(memoria->data);
	free(memoria);
}