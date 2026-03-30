/*
 * color.h — typi et operationes colorum communes
 *
 * Color RGBA et gamma correctio.
 * Omnia programmata graphica hanc bibliothecam adhibent.
 */

#ifndef COLOR_H
#define COLOR_H

#include <math.h>

/* ================================================================
 * typus coloris
 * ================================================================ */

typedef struct { double r, g, b, a; } color_t;

/* ================================================================
 * operationes colorum (inline in capite)
 * ================================================================ */

static inline unsigned char gamma_corrigere(double valor)
{
    if (valor < 0.0) valor = 0.0;
    if (valor > 1.0) valor = 1.0;
    return (unsigned char)(pow(valor, 1.0 / 2.2) * 255.0 + 0.5);
}

#endif /* COLOR_H */
