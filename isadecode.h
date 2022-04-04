#ifndef __ISADECODE_H
#define __ISADECODE_h

#include "isa.h"
#include "util.h"

void printCodigoTipoR(Isa *, unsigned int);
void printCodigoTipoI(Isa *, unsigned int);
void printCodigoTipoJ(Isa *, unsigned int, unsigned int pc);
void printCodigoCop1(Isa *, unsigned int);

#endif