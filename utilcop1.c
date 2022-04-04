#include "utilcop1.h"
#include <stdio.h>

void mfc1(Memoria *memoria, int rt, int fs)
{
    if (fs <= 30)
    {
        unsigned int valorFs = memoria->registradores[fs + IDX_INICIO_COP1].valor;

        unsigned int v = memoria->registradores[rt].valor;

        if (rt != 0)
            memoria->registradores[rt].valor = valorFs;

        if (DECODE)
            printf("\t0x%08x <- 0x%08x\n", v, valorFs);
    }
}

void mflo(Memoria *memoria, int rd)
{   
    unsigned int lo = memoria->registradores[IDX_LO].valor;
    if (rd != 0)
        memoria->registradores[rd].valor = lo;

    if (DECODE)
        printf("\t%s <- 0x%08x\n", memoria->registradores[rd].nome, lo);
}

void mult(Memoria *memoria, int rs, int rt)
{
    unsigned int multiplicando = memoria->registradores[rs].valor;
    unsigned int multiplicador = memoria->registradores[rt].valor;

    char bit = 0;
    unsigned long produto = 0;

    int i = 0;
    while (i < ARQUITETURA)
    {  
        bit = multiplicador & 0x1;

        if (bit == 1)
            produto += multiplicando;
        
        multiplicando = multiplicando << 1;
        multiplicador = multiplicador >> 1;

        i += 1;
    }


    memoria->registradores[IDX_HI].valor = produto >> ARQUITETURA;
    memoria->registradores[IDX_LO].valor = produto & 0xffffffff;

    if (DECODE)
        printf("\t(%x, %x) <- %x x %x\n", memoria->registradores[IDX_LO].valor, memoria->registradores[IDX_HI].valor, 
        memoria->registradores[rs].valor, memoria->registradores[rt].valor);
}

void mfhi(Memoria *memoria, int rd)
{
    unsigned int hi = memoria->registradores[IDX_HI].valor;
    if (rd != 0)
    {   
        memoria->registradores[rd].valor = hi;
    }

    if (DECODE)
        printf("%s <- 0x%08x\n", memoria->registradores[rd].nome, hi);
}

void mtc1(Memoria *memoria, int rt, int fs)
{
    unsigned int valor = memoria->registradores[rt].valor;

    fs += IDX_INICIO_COP1;
    if (fs != IDX_LO && fs != IDX_HI)
        memoria->registradores[fs].valor = valor;
    

    if (DECODE)
        printf("\t%s <- 0x%08x\n", memoria->registradores[fs].nome, valor);
}

void divCop1(Memoria *memoria, int rs, int rt)
{
    unsigned int dividendo = memoria->registradores[rs].valor;
    unsigned long valorRt = memoria->registradores[rt].valor;

    int flagNegativo = 0;
  
    if (isNegative(dividendo))
    {
        dividendo = complemento(dividendo);
        flagNegativo += 1;
    }  
    
    if (isNegative(valorRt))
    {
        valorRt = complemento(valorRt);
        flagNegativo += 2;
    }              
    
    unsigned long resto = dividendo;
    unsigned long divisor = valorRt << ARQUITETURA;
    unsigned long quociente = 0;
    
    int i = 0;
    while (i < ARQUITETURA + 1)
    {
        resto = resto - divisor;
       
        quociente = quociente << 1;

        int msb = resto >> (ARQUITETURA * 2 - 1);
        if (msb == 1)
        {
            resto = resto + divisor; 
            quociente = quociente | 0x0;
        }         
        else
            quociente = quociente | 0x1;
    
        divisor = divisor >> 1;
        i += 1;
    }

    if (flagNegativo != 0)
    {
        if (flagNegativo == 1)
            resto = complemento(resto);

        if (flagNegativo != 3)
            quociente = complemento(quociente);
    }
    
    memoria->registradores[IDX_HI].valor = resto;
    memoria->registradores[IDX_LO].valor = quociente;
}

void lwc1(Memoria *memoria, unsigned int valorBase, int rt, unsigned int im, unsigned int *cycles)
{   
    unsigned int valor = 0;

    *cycles -= CycleTime;
    if (CONFIG == 0 || CONFIG == 1)
        valor = getDataValor(memoria, valorBase + im, cycles);
    else
        valor = getDataCache(memoria, valorBase + im, "dados", 1, 1, cycles);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (rt != hi && rt != lo)
        memoria->registradores[IDX_INICIO_COP1 + rt].valor = valor;

    if (DECODE)
        printf("\t0x%08x <- 0x%08x + 0x%08x\n", memoria->registradores[IDX_INICIO_COP1 + rt].valor, valorBase, im);
}

void ldc1(Memoria *memoria, unsigned int valorBase, int rt, unsigned int im, unsigned int *cycles)
{
    unsigned int v1 = 0;
    unsigned int v2 = 0;

    *cycles -= CycleTime;
    if (CONFIG == 0 || CONFIG == 1)
    {
        v1 = getDataValor(memoria, valorBase + im, cycles);
        v2 = getDataValor(memoria, valorBase + im + ALINHAMENTO, cycles);
    } 
    else
    {
        v1 = getDataCache(memoria, valorBase + im, "dados", 1, 1, cycles);
        v2 = getDataCache(memoria, valorBase + im + ALINHAMENTO, "dados", 1, 1, cycles);
    }

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (rt != hi && rt != lo)
    {
        memoria->registradores[IDX_INICIO_COP1 + rt].valor = v1;
        memoria->registradores[IDX_INICIO_COP1 + rt + 1].valor = v2;
    }

    if (DECODE)
        printf("\t%s <- 0x%08x e %s <- 0x%08x\n", memoria->registradores[rt].nome, v2, memoria->registradores[rt + 1].nome, v1);
}

void swc1(Memoria *memoria, unsigned int valorBase, int ft, unsigned int im, unsigned int *cycles)
{
    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;

    
    *cycles -= CycleTime;
    
    if (CONFIG == 0 || CONFIG == 1)
        setDataMemoria(memoria, valorBase + im, valorFt, cycles, 1);
    else
        setDataCache(memoria, valorBase + im, valorFt, 1, 1, cycles);
        

    if (DECODE)
        printf("\tmemory[%x + %x] <- %x\n", valorBase, im, valorFt);
}

void movs(Memoria *memoria, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;

    if (fd != hi && fd != lo)
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = valorFs;

    if (DECODE)
        printf("\t%s <- 0x%x\n", memoria->registradores[IDX_INICIO_COP1 + fd].nome, valorFs);

}

void movd(Memoria *memoria, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFs2 = memoria->registradores[IDX_INICIO_COP1 + fs + 1].valor;

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;

    if (fd != hi && fd != lo)
    {
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = valorFs;
        memoria->registradores[IDX_INICIO_COP1 + fd + 1].valor = valorFs2;
    }
        
    
    if (DECODE)
        printf("\t%s <- 0x%x e %s <- 0x%x\n", memoria->registradores[IDX_INICIO_COP1 + fd].nome, valorFs, memoria->registradores[IDX_INICIO_COP1 + fd + 1].nome, valorFs2);

}

void cvtdw(Memoria *memoria, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;

    double v = (double) valorFs;

    unsigned long bits = convertToLongBits(v);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;

    unsigned int loValor = bits & 0xffffffff;
    unsigned int hiValor = bits >> ARQUITETURA;
    if (fd != hi && fd != lo)
    {
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = loValor;
        memoria->registradores[IDX_INICIO_COP1 + fd + 1].valor = hiValor;
    }

    if (DECODE)
        printf("\t%x -> %lx (lo = %x e hi = %x)\n", valorFs, bits, loValor, hiValor);
}

void cvtsd(Memoria *memoria, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFs2 = memoria->registradores[IDX_INICIO_COP1 + fs + 1].valor;

    unsigned long db = valorFs2;
    db = db << ARQUITETURA;
    db = db | valorFs;

    double v = getDouble(db);

    float valor = (float) v;

    unsigned int bits = convertFloatToBits(valor);
    
    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (fd != hi && fd != lo)
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = bits;
}

void divd(Memoria *memoria, int ft, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFs2 = memoria->registradores[IDX_INICIO_COP1 + fs + 1].valor;

    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;
    unsigned int valorFt2 = memoria->registradores[IDX_INICIO_COP1 + ft + 1].valor;


    unsigned long dbFs = valorFs2;
    dbFs = dbFs << ARQUITETURA;
    dbFs = dbFs | valorFs;

    unsigned long dbFt = valorFt2;
    dbFt = dbFt << ARQUITETURA;
    dbFt = dbFt | valorFt;

    double d1 = getDouble(dbFs);
    double d2 = getDouble(dbFt);

    double resultado = d1 / d2;

    unsigned long bits = convertToLongBits(resultado);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (fd != hi && fd != lo)
    {
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = bits & 0xffffffff;
        memoria->registradores[IDX_INICIO_COP1 + fd + 1].valor = bits >> ARQUITETURA;
    }

}

void muls(Memoria *memoria, int ft, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;

    float f1 = getFloat(valorFs);
    float f2 = getFloat(valorFt);

    float resultado = f1 * f2;

    unsigned int bits = convertFloatToBits(resultado);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (fd != hi && fd != lo)
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = bits;
    
    if (DECODE)
        printf("\t0x%x <- 0x%x x 0x%x (%f = %f X %f)\n", memoria->registradores[IDX_INICIO_COP1 + fd].valor, valorFs, valorFt, resultado, f1, f2);
}

void muld(Memoria *memoria, int ft, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFs2 = memoria->registradores[IDX_INICIO_COP1 + fs + 1].valor;

    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;
    unsigned int valorFt2 = memoria->registradores[IDX_INICIO_COP1 + ft + 1].valor;


    unsigned long dbFs = valorFs2;
    dbFs = dbFs << ARQUITETURA;
    dbFs = dbFs | valorFs;

    unsigned long dbFt = valorFt2;
    dbFt = dbFt << ARQUITETURA;
    dbFt = dbFt | valorFt;

    double d1 = getDouble(dbFs);
    double d2 = getDouble(dbFt);

    double resultado = d1 * d2;

    unsigned long bits = convertToLongBits(resultado);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;

    unsigned int loValor = bits & 0xffffffff;
    unsigned int hiValor = bits >> ARQUITETURA;
    if (fd != hi && fd != lo)
    {
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = loValor;
        memoria->registradores[IDX_INICIO_COP1 + fd + 1].valor = hiValor;
    }

    if (DECODE)
        printf("\t%.15lf * %.15lf -> %.15lf = %lx (lo = %x e hi = %x)\n", d1, d2, resultado, bits, loValor, hiValor);
}

void adds(Memoria *memoria, int ft, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;

    float f1 = getFloat(valorFs);
    float f2 = getFloat(valorFt);

    float resultado = f1 + f2;

    unsigned int bits = convertFloatToBits(resultado);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (fd != hi && fd != lo)
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = bits;
    
    if (DECODE)
        printf("\t0x%x <- 0x%x + 0x%x (%f = %f + %f)\n", memoria->registradores[IDX_INICIO_COP1 + fd].valor, valorFs, valorFt, resultado, f1, f2);
}

void addd(Memoria *memoria, int ft, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFs2 = memoria->registradores[IDX_INICIO_COP1 + fs + 1].valor;

    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;
    unsigned int valorFt2 = memoria->registradores[IDX_INICIO_COP1 + ft + 1].valor;


    unsigned long dbFs = valorFs2;
    dbFs = dbFs << ARQUITETURA;
    dbFs = dbFs | valorFs;

    unsigned long dbFt = valorFt2;
    dbFt = dbFt << ARQUITETURA;
    dbFt = dbFt | valorFt;

    double d1 = getDouble(dbFs);
    double d2 = getDouble(dbFt);

    double resultado = d1 + d2;

    unsigned long bits = convertToLongBits(resultado);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (fd != hi && fd != lo)
    {
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = bits & 0xffffffff;
        memoria->registradores[IDX_INICIO_COP1 + fd + 1].valor = bits >> ARQUITETURA;
    }
        
}

void subs(Memoria *memoria, int ft, int fs, int fd)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;

    float f1 = getFloat(valorFs);
    float f2 = getFloat(valorFt);

    float resultado = f1 - f2;

    unsigned int bits = convertFloatToBits(resultado);

    int hi = IDX_HI - IDX_INICIO_COP1;
    int lo = IDX_LO - IDX_INICIO_COP1;
    if (fd != hi && fd != lo)
        memoria->registradores[IDX_INICIO_COP1 + fd].valor = bits;
}

unsigned int bc1t(Memoria *memoria, int imediato)
{
    unsigned int im = extenderSinal(imediato, 16);
    unsigned int forwarding = 0;

    if (memoria->registradores[CC].valor == 1)
        forwarding = memoria->registradores[32].valor + (im * ALINHAMENTO) + ALINHAMENTO;
    
    if (DECODE)
        printf("\tpc=0x%08x\n", forwarding);
    
    return forwarding;
}

unsigned int bc1f(Memoria *memoria, int imediato)
{
    unsigned int im = extenderSinal(imediato, 16);
    unsigned int forwarding = 0;

    if (memoria->registradores[CC].valor == 0)
        forwarding = memoria->registradores[32].valor + (im * ALINHAMENTO) + ALINHAMENTO;
    
    if (DECODE)
        printf("\tpc=0x%08x\n", forwarding);
    
    return forwarding;
}

void cconds(Memoria *memoria, int ft, int fs, int cond)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;

    float f1 = getFloat(valorFs);
    float f2 = getFloat(valorFt);
   
    switch (cond)
    {
        case 0x2:
            if (f1 == f2)
                memoria->registradores[CC].valor = 1;
            else 
                memoria->registradores[CC].valor = 0;

            break;
        case 0xc:
            if (f1 < f2)
                memoria->registradores[CC].valor = 1;
            else 
                memoria->registradores[CC].valor = 0;

            if (DECODE)
                printf("\tcc = %x <- %x < %x (%f < %f)\n", memoria->registradores[CC].valor, valorFs, valorFt, f1, f2);
            break;
        case 0xe:
            if (f1 >= f2)
                memoria->registradores[CC].valor = 1;
            else
                memoria->registradores[CC].valor = 0;

            break;
        default:
            printf("c.cond.fmt não executou.\n");
            return;
    }
}

void ccondd(Memoria *memoria, int ft, int fs, int cond)
{
    unsigned int valorFs = memoria->registradores[IDX_INICIO_COP1 + fs].valor;
    unsigned int valorFs2 = memoria->registradores[IDX_INICIO_COP1 + fs + 1].valor;

    unsigned int valorFt = memoria->registradores[IDX_INICIO_COP1 + ft].valor;
    unsigned int valorFt2 = memoria->registradores[IDX_INICIO_COP1 + ft + 1].valor;


    unsigned long dbFs = valorFs2;
    dbFs = dbFs << ARQUITETURA;
    dbFs = dbFs | valorFs;

    unsigned long dbFt = valorFt2;
    dbFt = dbFt << ARQUITETURA;
    dbFt = dbFt | valorFt;

    double d1 = getDouble(dbFs);
    double d2 = getDouble(dbFt);


    switch (cond)
    {
        case 0x2:
            if (d1 == d2)
                memoria->registradores[CC].valor = 1;
            else 
                memoria->registradores[CC].valor = 0;

            break;
        case 0xc:
            if (d1 < d2)
                memoria->registradores[CC].valor = 1;
            else 
                memoria->registradores[CC].valor = 0;

            break;
        case 0xe:
            if (d1 <= d2)
                memoria->registradores[CC].valor = 1;
            else
                memoria->registradores[CC].valor = 0;

            break;
        default:
            return;
    }
}
