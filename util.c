#include "util.h"
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void convertToBigEndian(int **word, int size)
{
	int i;
	for (i = size - 1; i >= size / 2; i--)
	{
		int aux = (*word)[i];
		(*word)[i] = (*word)[size - i - 1];
		(*word)[size - i - 1] = aux;
	}
}

char* convertBinToString(unsigned long long bin)
{
	int qtdBits = ARQUITETURA;

	char *bits = (char *) malloc(sizeof(int) * (qtdBits + 1));
	bits[qtdBits] = 0;

	int i;
	for (i = qtdBits - 1; i >= 0; i--)
	{
		if (bin / 10)
		{
			bits[i] = (bin % 10) + '0';
			bin = bin / 10;
		}
		else if (bin == 1)
		{
			bits[i] = 1 + '0';
			bin = 0;
		}
		else
			bits[i] = '0';

	}

	return bits;
}

unsigned long long convertBinToInt(char *bin)
{
	int i;
	int tamanho = strlen(bin);

	unsigned long long resultado = 0;
	for (i = 0; i < tamanho; i++)
	{
		if (bin[i] == '1')
		{
			resultado += potencia(2, (tamanho - i - 1));
		}
	}

	return resultado;
}

unsigned long int potencia(int base, int expoente)
{
	unsigned long int resultado = 1;
	int i;
	for (i = 0; i < expoente; i++)
		resultado *= base;

	return resultado;
}

unsigned long long dec2bin(int dec)
{
	unsigned long long bin = 0;

	int count = 0;
	while (dec != 0)
	{
		int resto = dec % 2;
		unsigned long long pt = potencia(10, count);

		bin += resto * pt;

		dec /= 2;
		count += 1;
	}

	return bin;
}

unsigned long int getValorComplementoBinario(char *b, int tamanho)
{
	int i;
	for (i = 0; i < tamanho; i++)
	{
		if (b[i] == '1')
			b[i] = '0';
		else
			b[i] = '1';
	}

	unsigned long int numero = convertBinToInt(b) + 1;
	numero *= -1;

	return numero;
}

void setNumeroSigned(unsigned long int *numero)
{
	char *bin = convertValorToBin(*numero);

	if (bin[0] == '1')
	{
		*numero = getValorComplementoBinario(bin, ARQUITETURA);
		free(bin);
		return;
	}

	free(bin);
}

void setNumeroSignedInt(int *numero)
{
	char *bin = convertValorToBin(*numero);

	int size = (ARQUITETURA / 2);
	char b[size + 1];
	substring(bin, size, size, b); b[size] = 0;

	if (b[0] == '1')
	{
		*numero = getValorComplementoBinario(b, size);
		free(bin);
		return;
	}

	free(bin);
}

void substring(char *binario, int inicio, int tamanho, char *sub)
{
	strncpy(sub, binario + inicio, tamanho);

}

void imprimirVetor(int *vetor, int tamanho)
{
	int i;

	for (i = 0; i < tamanho; i++)
		printf("%02x ", vetor[i]);

	printf("\n");
}

char *getString(unsigned long long bin)
{
	int tamanho = 1;
	unsigned long long aux = bin;
	while (aux / 10)
	{
		aux /= 10;
		tamanho += 1;
	}

	char *string = (char *) malloc(sizeof(char) * (tamanho + 1));
	string[tamanho] = '\0';

	int i;
	for (i = tamanho - 1; i >= 0; i--)
	{
		string[i] = (bin % 10) + '0';
		bin /= 10;
	}

	return string;
}

char *convertBinToUpper(char *bin)
{
	int tamanhoBin = strlen(bin);

	char *upper = (char *) malloc(sizeof(char) * (ARQUITETURA + 1));
	upper[ARQUITETURA] = '\0';

	int indexInicio = (ARQUITETURA / 2) - tamanhoBin;
	if (indexInicio < 0) indexInicio = 0;

	int i;
	for (i = 0; i < ARQUITETURA; i++)
		if (i >= indexInicio && i < tamanhoBin + indexInicio)
			upper[i] = bin[i - indexInicio];
		else
			upper[i] = '0';

	return upper;
}

char *convertBinToChar(int bin)
{
	int qtdBits = ARQUITETURA / ALINHAMENTO;

	char *bits = (char *) malloc(sizeof(int) * (qtdBits + 1));
	bits[qtdBits] = 0;

	int i;
	for (i = qtdBits - 1; i >= 0; i--)
	{
		if (bin / 10)
		{
			bits[i] = (bin % 10) + '0';
			bin = bin / 10;
		}
		else if (bin == 1)
		{
			bits[i] = 1 + '0';
			bin = 0;
		}
		else
			bits[i] = '0';

	}

	return bits;
}

int convertIntToBin(int byte)
{
	if (byte / 2 == 0)
		return byte % 2;

	if (byte % 2)
		return convertIntToBin(byte / 2) * 10 + 1;

	return convertIntToBin(byte / 2) * 10;
}

char *convertCodigoToBin(int *bytes)
{
	char *bin = (char *) malloc(sizeof(char) * ARQUITETURA);
	memset(bin, '\0', sizeof(char) * ARQUITETURA);

	int i = 0;
	for (i = 0; i < ALINHAMENTO; i++)
	{
		int byteBin = convertIntToBin(bytes[i]);
		char *aux = convertBinToChar(byteBin);
		strcat(bin, aux);

		free(aux);
	}

	return bin;
}

char *convertValorToBin(unsigned long int valor)
{
	char *bin = (char *) malloc(sizeof(char) * (ARQUITETURA + 1));
	memset(bin, '0', sizeof(char) * (ARQUITETURA)); bin[ARQUITETURA] = 0;

	int restos[ARQUITETURA];
	
	int i = 0;
	while (valor / 2)
	{
		int resto = valor % 2;

		restos[i] = resto;

		i+= 1;
		valor /= 2;
	}

	int tamanho = i + 1;
	restos[i] = valor;

	int j = 0;
	for (i = tamanho - 1; i >= 0; i--)
	{
		bin[ARQUITETURA - i - 1] = restos[tamanho - j - 1] + '0';

		j += 1;
	}

	return bin;
}

int *convertBinToCodigo(char *bin)
{
	int *codigo = (int *) malloc(sizeof(int) * ALINHAMENTO);

	int byte = 8;

	int i;
	for (i = 0; i < ALINHAMENTO; i++)
	{
		char b[byte + 1];
		substring(bin, i * byte, byte, b); b[byte] = 0;

		int cod = convertBinToInt(b);
		codigo[i] = cod;
	}

	return codigo;
}

void convertCodigoToBinario(int *codigo, char *binario)
{
	memset(binario, '\0', sizeof(char) * ARQUITETURA);

	for (int i = 0; i < ALINHAMENTO; i++)
	{
		int byteBin = convertIntToBin(codigo[i]);
		char *aux = convertBinToChar(byteBin);
		strcat(binario, aux);

		free(aux);
	}
}


void copiarString(char *dest, char *source)
{
	int tamanho = strlen(source);

	int i;
	for (i = 0; i < tamanho; i++)
		dest[i] = source[i];
}