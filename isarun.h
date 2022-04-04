#ifndef __ISARUN_H
#define __ISARUN_H

#include "isa.h"
#include "util.h"
#include "utilcop1.h"

int isExceptionChar(int codigo);
void printExceptionChar(int codigo);

int executaSyscall(Memoria *memoria, Isa *isa);
void setNomeTipoR(Isa *isa, unsigned int binario);
int runCodigoTipoR(Memoria *memoria, Isa *isa, unsigned int binario);

void setNomeTipoI(Isa *isa, unsigned int binario);
int runCodigoTipoI(Memoria *memoria, unsigned int binario, unsigned int *);

void setNomeTipoJ(Isa *isa, unsigned int binario);
int runCodigoTipoJ(Memoria *memoria, unsigned int binario);

void setNomeTipoCop1(Isa *isa, unsigned int binario);
int runCodigoCop1(Memoria *memoria, Isa *isa, unsigned int binario);

#endif