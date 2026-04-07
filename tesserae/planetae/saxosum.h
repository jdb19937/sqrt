/*
 * saxosum.h — planeta saxosum (Mercury, Venus, Terra, Mars, Luna, Io, Europa)
 *
 * Superficies solida cum compositione chimica, aqua, terrain,
 * atmosphaera, et calottibus polaribus.
 */

#ifndef PLANETA_SAXOSUM_H
#define PLANETA_SAXOSUM_H

#include "../planeta.h"

typedef struct {
    planetella_t basis;

    /* compositio superficiei (fractiones, summa <= 1) */
    double     silicata;         /* SiO2 / silicates */
    double     ferrum;           /* Fe2O3 / iron oxides */
    double     sulphur;          /* S / SO2 */
    double     carbo;            /* C / carbonaceous */
    double     glacies;          /* H2O ice */
    double     glacies_co2;      /* CO2 ice */
    double     malachita;        /* green minerals / biology */

    /* aqua liquida */
    double     aqua;             /* fractio superficiei aqua liquida 0..1 */
    double     aqua_profunditas; /* 0=vadosa, 1=profunda */

    /* terrain procedurale */
    int        continentes;      /* numerus continentium */
    double     scala;            /* scala featurum */
    double     tectonica;        /* complexitas litoralis 0..1 */
    double     craterae;         /* densitas craterarum 0..1 */
    double     maria;            /* fractio mariorum basalticorum 0..1 */
    double     vulcanismus;      /* activitas vulcanica 0..1 */

    /* atmosphaera */
    double     pressio_kPa;      /* pressio superficiei */
    double     n2;               /* nitrogen */
    double     o2;               /* oxygen */
    double     co2;              /* carbon dioxide */
    double     ch4;              /* methane */
    double     h2;               /* hydrogen */
    double     he;               /* helium */
    double     nh3;              /* ammonia */
    double     pulvis;           /* aerosoli 0..1 */
    double     nubes;            /* cooperimentum nubium 0..1 */

    /* glacies polaris */
    double     polaris;          /* extensio calottum polarium 0..1 */
} planeta_saxosum_t;

#endif /* PLANETA_SAXOSUM_H */
