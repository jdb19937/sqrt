/*
 * torus.h — visio torus planus corrugatus (per helvea)
 */

#ifndef VISIO_TORUS_H
#define VISIO_TORUS_H

#include "../visio.h"
#include "../../helvea.h"

typedef struct {
    helvea_methodus_t methodus;       /* HELVEA_BORRELLI etc. */
    int               thema;          /* index in helvea_themata[] */
    int               strata;         /* profunditas corrugationis */
    double            radius_maior;   /* R tori */
    double            radius_minor;   /* r tori */
    int               gradus_u;       /* resolutio u */
    int               gradus_v;       /* resolutio v */
    double            distantia;      /* distantia camerae */
    double            elevatio;       /* angulus elevationis (radiani) */
    double            azimuthus;      /* angulus azimuthi (radiani) */
} toriculus_t;

typedef struct {
    tessella_t        avi;
    visiuncula_t      pro;
    toriculus_t res;
} torus_t;

#endif /* VISIO_TORUS_H */
