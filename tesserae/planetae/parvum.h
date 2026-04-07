/*
 * parvum.h — planeta parvum (Pluto, Ceres, lunae parvae)
 *
 * Corpora parva — eodem modo ac saxosum redduntur.
 */

#ifndef PLANETA_PARVUM_H
#define PLANETA_PARVUM_H

#include "../planeta.h"

typedef struct {
    planetella_t basis;

    /* compositio superficiei */
    double     silicata;
    double     ferrum;
    double     sulphur;
    double     carbo;
    double     glacies;
    double     glacies_co2;
    double     malachita;

    /* aqua liquida */
    double     aqua;
    double     aqua_profunditas;

    /* terrain */
    int        continentes;
    double     scala;
    double     tectonica;
    double     craterae;
    double     maria;
    double     vulcanismus;

    /* atmosphaera */
    double     pressio_kPa;
    double     n2;
    double     o2;
    double     co2;
    double     ch4;
    double     h2;
    double     he;
    double     nh3;
    double     pulvis;
    double     nubes;

    /* glacies polaris */
    double     polaris;
} planeta_parvum_t;

#endif /* PLANETA_PARVUM_H */
