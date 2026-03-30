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

/* miscere duos colores per interpolationem linearem.
 * t ∈ [0, 1]: 0 = color a, 1 = color b. */
static inline color_t miscere(color_t a, color_t b, double t)
{
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return (color_t){
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

#endif /* COLOR_H */
