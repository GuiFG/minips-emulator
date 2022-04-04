#ifndef __UTILCOP1_H
#define __UTILCOP1_H

#include "memoria.h"
#include "util.h"

void mfc1(Memoria *, int, int);
void mflo(Memoria *, int);
void mult(Memoria *, int, int);
void mfhi(Memoria *, int);
void mtc1(Memoria *, int, int);
void divCop1(Memoria *, int, int);
void lwc1(Memoria *, unsigned int, int, unsigned int, unsigned int *);
void ldc1(Memoria *, unsigned int, int, unsigned int, unsigned int *);
void swc1(Memoria *, unsigned int, int, unsigned int, unsigned int *);
void movd(Memoria *, int, int);
void movs(Memoria *, int, int);
void cvtdw(Memoria *, int, int);
void cvtsd(Memoria *, int, int);
void divd(Memoria *, int, int, int);
void muls(Memoria *, int, int, int);
void muld(Memoria *, int, int, int);
void adds(Memoria *, int, int, int);
void addd(Memoria *, int, int, int);
void subs(Memoria *, int, int, int);
unsigned int bc1t(Memoria *, int);
unsigned int bc1f(Memoria *, int);
void cconds(Memoria *, int, int, int);
void ccondd(Memoria *, int, int, int);

#endif