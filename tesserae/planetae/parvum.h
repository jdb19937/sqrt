/*
 * parvum.h — planeta parvum (Pluto, Ceres, lunae parvae)
 *
 * Corpora parva — eodem modo ac saxosum redduntur.
 */

#ifndef PLANETA_PARVUM_H
#define PLANETA_PARVUM_H

#include "../planeta.h"

typedef struct {
    /* compositio superficiei */
    double silicata;
    double ferrum;
    double sulphur;
    double carbo;
    double glacies;
    double glacies_co2;
    double malachita;

    /* aqua liquida */
    double aqua;
    double aqua_profunditas;

    /* terrain */
    int    continentes;
    double scala;
    double tectonica;
    double craterae;
    double maria;
    double vulcanismus;

    /* atmosphaera */
    double pressio_kPa;
    double n2;
    double o2;
    double co2;
    double ch4;
    double h2;
    double he;
    double nh3;
    double pulvis;
    double nubes;

    /* glacies polaris */
    double polaris;
} parvulum_t;

typedef struct {
    tessella_t   avi;
    planetella_t pro;
    parvulum_t   res;
} parvum_t;

void reddere_parvum(unsigned char *fen, const parvum_t *p);
void parvum_in_ison(FILE *f, const parvum_t *s);
void parvum_ex_ison(parvum_t *s, const char *ison);

#endif /* PLANETA_PARVUM_H */
