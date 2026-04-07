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

#include "../perceptus.h"
#include "../color.h"
#include "../instrumentum.h"

#include <math.h>

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
} planeta_genus_t;

/* proprietates communes omnium planetarum */
typedef struct {
    planeta_genus_t genus;
    double     radius;           /* 0.0-1.0: fractio fenestrae */
    double     inclinatio;       /* inclinatio axialis (radiani) */
    double     rotatio;          /* longitudo centralis visibilis (radiani) */
    unsigned   semen;            /* semen procedurale */
} planeta_t;

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
 * functiones
 * ================================================================ */

void planeta_reddere(
    unsigned char *fenestra, const planeta_t *planeta,
    const planeta_perceptus_t *perceptus
);

planeta_t *planeta_ex_ison(const char *ison);

void planeta_instrumentum_applicare(
    double pressio_kPa,
    instrumentum_t *instr
);

#endif /* PLANETA_H */
