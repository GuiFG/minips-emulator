#ifndef	__MEMORIA_H
#define __MEMORIA_H

typedef struct registrador {
	char *nome;
	unsigned int valor;
} Registrador;

typedef struct data {
	unsigned int endereco;
	unsigned int valor;
	struct data *proximo;
} Data;

typedef struct hashData {
	Data *encadeada;
} HashData;

typedef struct line {
	char valido;
	char clean;
	int idade;

	unsigned int tag;
	unsigned int *data;
} Line;

typedef struct set {
	Line *lines;
} Set;

typedef struct cache {
	const char *tipo;
	const char *politica;
	int capacidade;
	int tamanhoLinha;
	int qtdSets;
	int qtdVias;

	int byteOffset;
	int blockOffset;
	int lineIndexOffset;

	unsigned int hits;
	unsigned int misses;

	Set *sets;
} Cache;

typedef struct Memoria {
	int qtdInstrucoes;
	int tamanhoLinha;
	unsigned int hits;
	unsigned int misses;
	Registrador *registradores;
	HashData *data;

	const char *modoCache;
	const char *modoCacheL2;
	Cache cache;
	Cache cacheInstrucao;
	Cache cacheData;
	Cache cacheL2;
} Memoria;

#define ALINHAMENTO 4
#define ARQUITETURA 32
#define TAMANHO_DATA 20480
#define QTD_REGISTRADORES 67
#define ENDERECO_TEXT 0x00400000
#define ENDERECO_DATA 0x10010000
#define ENDERECO_RODATA 0x00800000
#define ADDR_GLOBAL_POINTER 0x10008000
#define ADDR_STACK_POINTER 0x7fffeffc
#define CC 66

#define IDX_INICIO_COP1 33
#define IDX_LO 64
#define IDX_HI 65

#define DECODE 0
#define RUNDEC 0	

extern char CONFIG;
extern char TRACE;
extern char DEBUG;

extern char LEVEL;

Memoria *criarMemoria();
void liberarMemoria(Memoria*);

void copiarCodigo(int*, int*);

int salvarInstrucao(Memoria *, unsigned int, unsigned int);
int salvarData(Memoria *, unsigned int, unsigned int);

unsigned int getInstrucao(Memoria *, unsigned int);
unsigned int getDataValor(Memoria *, unsigned int, unsigned int *);
void setDataMemoria(Memoria *, unsigned int, unsigned int, unsigned int *, char);

unsigned int getDataCache(Memoria *, unsigned int, const char *, int, char, unsigned int *);
void setDataCache(Memoria *, unsigned int, unsigned int, int, char, unsigned int *);

char *getNomeRegistrador(int);

void imprimirCodigo(int*);
void imprimirMemoria(Memoria*);

#endif