/*
 * zeppelinus.h — visio zeppelinus aerius
 *
 * Corpus ellipsoidale cum gondola et pinnis.
 * Illuminatio Phong simplex.
 */

#ifndef VISIO_ZEPPELINUS_H
#define VISIO_ZEPPELINUS_H

#include "../visio.h"
#include "../../color.h"

typedef struct {
    visiuncula_t basis;
    double  ratio;           /* ratio longitudinis ad diametrum */
    double  inclinatio;      /* angulus inclinationis (radiani) */
    double  azimuthus;       /* rotatio circa axem verticalem */
    double  lux_angulus;     /* directio lucis (radiani) */
    double  lux_elevatio;    /* elevatio lucis (radiani) */
    color_t involucrum;      /* color involucri */
    color_t gondola;         /* color gondolae */
    double  elevatio;        /* elevatio camerae (radiani) */
    double  fenestrae;       /* numerus fenestrarum gondolae */
    double  pinnae;          /* magnitudo pinnarum 0..1 */
} visio_zeppelinus_t;

#endif /* VISIO_ZEPPELINUS_H */
