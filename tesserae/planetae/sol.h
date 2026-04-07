/*
 * sol.h — planeta sol (stella proxima)
 *
 * Fusio completa — limb darkening, corona, granulatio.
 * PLANETA_SOL == PLANETA_GASEOSUM + fusio proxima 1.0,
 * sed renderitor separatus ob varietatem visualem.
 */

#ifndef PLANETA_SOL_H
#define PLANETA_SOL_H

#include "../planeta.h"

typedef struct {
    planeta_t basis;

    /* fusio stellaris */
    double     fusio;            /* intensitas fusionis [0,1] */
    double     temperatura;      /* temperatura photosphaericae (K) */
    double     luminositas;      /* multiplicator fulgoris */
    double     corona;           /* extensio coronae [0,1] */
    double     granulatio;       /* granulatio convectiva [0,1] */

    /* maculae (sunspots) */
    int        maculae;
    double     macula_radius;
    double     macula_obscuritas;

    /* compositio pro temperatura_ex_compositione */
    double     h2;
    double     he;
    double     ch4;
    double     nh3;
} planeta_sol_t;

#endif /* PLANETA_SOL_H */
