/*
 * planeta.h — renderer planetarum et lunarum
 *
 * Reddit corpora in fenestra 512×512 pixelorum.
 * Colores emergunt ex proprietatibus physicis: compositione
 * chimica superficiei, pressione atmosphaerica, etc.
 * Nullae overrides colorum — physica sola determinat apparentiam.
 */

#ifndef PLANETA_H
#define PLANETA_H

#include "../tessella.h"
#include "../perceptus.h"
#include "../color.h"
#include "../instrumentum.h"

#include <math.h>
#include <stdio.h>

/* ================================================================
 * constantes
 * ================================================================ */

#define PLANETA_FENESTRA  512
#define PLANETA_SEMI      256

/* ================================================================
 * typi
 * ================================================================ */

typedef enum {
    PLANETA_SAXOSUM,     /* Mercury, Venus, Terra, Mars, Luna, Io, Europa */
    PLANETA_GASEOSUM,    /* Jupiter, Saturnus */
    PLANETA_GLACIALE,    /* Uranus, Neptunus */
    PLANETA_PARVUM,      /* Pluto, Ceres, lunae parvae */
    PLANETA_SOL,         /* stella proxima — fusio completa */
    PLANETA_NEBULA       /* nubes gasei procedurale */
} planetarius_t;

/* proprietates communes omnium planetarum (diminutiva) */
typedef struct {
    double          radius;           /* 0.0-1.0: fractio fenestrae */
    double          inclinatio;       /* inclinatio axialis (radiani) */
    double          rotatio;          /* longitudo centralis visibilis (radiani) */
    unsigned        semen;            /* semen procedurale */
} planetella_t;

/* ================================================================
 * genera planetarum
 * ================================================================ */

#include "planetae/saxosum.h"
#include "planetae/gaseosum.h"
#include "planetae/glaciale.h"
#include "planetae/parvum.h"
#include "planetae/sol.h"
#include "planetae/nebula.h"

/* ================================================================
 * planeta_t — unio omnium generum planetarum
 *
 * Continet quodlibet genus planetae in acervo.
 * Accessio basis: planeta.planetella.genus, planeta.planetella.radius, ...
 * Accessio generis: planeta.saxosum.silicata, planeta.gaseosum.fasciae, ...
 * ================================================================ */

typedef struct {
    planetarius_t qui;

    union {
        saxosum_t  saxosum;
        gaseosum_t gaseosum;
        glaciale_t glaciale;
        parvum_t   parvum;
        sol_t      sol;
        nebula_t   nebula;
    } ubi;
} planeta_t;

/* ================================================================
 * functiones
 * ================================================================ */

void planeta_reddere(
    unsigned char *fenestra, const planeta_t *planeta
);

void planeta_illuminationem_applicare(
    unsigned char *fen, double radius,
    const planeta_perceptus_t *perc
);

planeta_t *planeta_ex_ison(const char *ison);

void planeta_in_ison(FILE *f, const planeta_t *p);

void planeta_instrumentum_applicare(
    double pressio_kPa,
    instrumentum_t *instr
);

#endif /* PLANETA_H */
