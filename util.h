#ifndef __UTIL_H
#define __UTIL_H

void convertToBigEndian(int **, int);
char* convertBinToString(unsigned long long);
unsigned long long convertBinToInt(char *);
unsigned long int potencia(int, int);
unsigned long long dec2bin(int);
unsigned long int getValorComplementoBinario(char *, int);
void setNumeroSigned(unsigned long int *);
void setNumeroSignedInt(int *);
void substring(char *, int, int, char *);
void imprimirVetor(int *, int);
char *getString(unsigned long long);
char *convertBinToUpper(char *);
char *convertBinToChar(int);
int convertIntToBin(int);
char *convertCodigoToBin(int *);
char *convertValorToBin(unsigned long int);
int *convertBinToCodigo(char *);
void convertCodigoToBinario(int *, char *);
void copiarString(char *, char *);

#endif 