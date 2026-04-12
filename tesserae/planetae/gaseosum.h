/*
 * gaseosum.h — planeta gaseosum (Jupiter, Saturnus)
 *
 * Gigantes gaseousi cum fasciis, maculis, atmosphaera,
 * et fusione optionali (transitus ad stellam).
 */

#ifndef PLANETA_GASEOSUM_H
#define PLANETA_GASEOSUM_H

#include "../planeta.h"

typedef struct {
    /* atmosphaera */
    double n2;
    double o2;
    double co2;
    double ch4;
    double h2;
    double he;
    double nh3;
    double pulvis;

    /* fasciae (cloud bands) */
    int    fasciae;           /* numerus fasciarum */
    double fasciae_contrast;  /* 0..1 */

    /* maculae (GRS etc.) */
    int    maculae;           /* numerus macularum */
    double macula_lat;        /* latitudo principalis (-1..1) */
    double macula_lon;        /* longitudo (radiani) */
    double macula_radius;     /* magnitudo 0..1 */
    double macula_obscuritas; /* <0=lucidior, >0=obscurior */

    /* fusio stellaris */
    double fusio;             /* intensitas fusionis [0,1] */
    double temperatura;       /* temperatura photosphaericae (K) */
    double luminositas;       /* multiplicator fulgoris */
    double corona;            /* extensio coronae [0,1] */
    double granulatio;        /* granulatio convectiva [0,1] */
} gaseosculum_t;

typedef struct {
    tessella_t    avi;
    planetella_t  pro;
    gaseosculum_t res;
} gaseosum_t;

void reddere_gaseosum(unsigned char *fen, const gaseosum_t *p);
void gaseosum_in_ison(FILE *f, const gaseosum_t *s);
void gaseosum_ex_ison(gaseosum_t *s, const char *ison);

#endif /* PLANETA_GASEOSUM_H */
