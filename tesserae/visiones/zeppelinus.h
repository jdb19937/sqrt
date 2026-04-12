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
} zeppelinulus_t;

typedef struct {
    tessella_t           avi;
    visiuncula_t         pro;
    zeppelinulus_t res;
} zeppelinus_t;

void zeppelinus_reddere(unsigned char *fenestra, const zeppelinus_t *z);
void zeppelinus_in_ison(FILE *f, const zeppelinus_t *s);
void zeppelinus_ex_ison(zeppelinus_t *s, const char *ison);

#endif /* VISIO_ZEPPELINUS_H */
