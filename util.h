#ifndef __UTIL_H
#define __UTIL_H

#include "memoria.h"
#include "isa.h"

void copiarString(char *, char *);
void convertToLittleEndian(int **, int);
unsigned int getValorWord(int *);
unsigned int extenderSinal(int, int);
unsigned int complemento(unsigned int);
int isNegative(unsigned int);
unsigned int convertImediatoToUpper(int);
void setNumeroSigned(unsigned int *);
void imprimirData(Memoria *);
double potencia(int , int);
float getFloat(unsigned int);
double getDouble(unsigned long);
unsigned long convertFlutuanteToBits(double, int);
unsigned long getBitsUm(int);
unsigned long convertToLongBits(double);
unsigned int convertFloatToBits(float);
void escreverArquivo(const char *, const char*, const char*);
int isInstrucaoLeitura();
int isIntrucaoEscrita();
void gerarLogTrace(const char *, unsigned int, int, int);
void gerarLogDebug(const char *, char *);
int getLog2(int);
void estatistica(Memoria *, Isa *, double);

#endif 