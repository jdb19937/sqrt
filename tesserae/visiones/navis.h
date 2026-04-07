/*
 * navis.h — visio navis ardens (burning ship fractal)
 *
 * z_{n+1} = (|Re(z_n)| + i|Im(z_n)|)^2 + c
 * Michalski & Zyczkowski (2003).
 */

#ifndef VISIO_NAVIS_H
#define VISIO_NAVIS_H

#include "../visio.h"

typedef struct {
    visiuncula_t basis;
    double  centrum_re;      /* centrum regionis (reale) */
    double  centrum_im;      /* centrum regionis (imaginarium) */
    double  amplitudo;       /* latitudo regionis in plano complexo */
    int     iterationes;     /* maximum iterationum */
    double  color_cyclus;    /* periodus cycli coloris */
    double  color_phase;     /* phase initialis coloris */
    double  saturatio;       /* saturatio colorum 0..2 */
} visio_navis_t;

#endif /* VISIO_NAVIS_H */
