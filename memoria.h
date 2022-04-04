#ifndef	MEMORIA_H
#define MEMORIA_H

typedef struct registrador {
	char *nome;
	unsigned long int valor;

} Registrador;


typedef struct instrucao {
	unsigned long int endereco;
	int *codigo;
	char *binario;
	struct instrucao *proximo;
} Instrucao;

typedef struct data {
	unsigned long int endereco;
	int *codigo;
	char *binario;
	unsigned long int valor;
	struct data *proximo;
} Data;

typedef struct hashData {
	Data *encadeada;
} HashData;

typedef struct hashText {
	Instrucao *encadeada;
} HashText;


typedef struct Memoria {
	int qtdInstrucoes;
	Registrador *registradores;
	HashText *text;
	HashData *data;
} Memoria;

#define ALINHAMENTO 4
#define ARQUITETURA 32
#define TAMANHO_TEXT 8192
#define TAMANHO_DATA 8192
#define QTD_REGISTRADORES 33
#define ENDERECO_TEXT 0x00400000
#define ENDERECO_DATA 0x10010000
#define ADDR_GLOBAL_POINTER 0x10008000
#define ADDR_STACK_POINTER 0x7fffeffc


Memoria *criarMemoria();
void liberarMemoria(Memoria*);

void copiarCodigo(int*, int*);

int salvarInstrucao(Memoria*, unsigned long int, int*);
int salvarData(Memoria *, unsigned long int, int *);


Instrucao *getIntrucao(Memoria *, unsigned long int);

char *getDataBinario(Memoria *, unsigned long int);
int *getDataCodigo(Memoria *, unsigned long int);
unsigned long int getDataValor(Memoria *, unsigned long int);
void setDataMemoria(Memoria *, unsigned long int, unsigned long int);

char *getNomeRegistrador(int);

void imprimirCodigo(int*);
void imprimirMemoria(Memoria*);

#endif