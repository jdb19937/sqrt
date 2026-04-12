/*
 * visio.h — visiones proximae, 2048x2048
 *
 * Res quae prope conspiciuntur: torus, fractale, zeppelinus.
 * Planetae 512x512 sunt; visiones 2048x2048.
 * Hereditas C99: primum membrum cuiusque subtypi est visio_t.
 */

#ifndef VISIO_H
#define VISIO_H

#include "../tessella.h"

#include <math.h>
#include <stdio.h>

/* ================================================================
 * constantes
 * ================================================================ */

#define VISIO_FENESTRA  2048

/* ================================================================
 * genera visionum
 * ================================================================ */

typedef enum {
    VISIO_TORUS,       /* torus planus quadratus corrugatus */
    VISIO_NAVIS,       /* navis ardens (burning ship fractal) */
    VISIO_ZEPPELINUS   /* zeppelinus aerius */
} visualis_t;

#define VISIO_NUMERUS_GENERUM 3

/* ================================================================
 * visiuncula_t — basis communis (diminutiva)
 * ================================================================ */

typedef struct {
    unsigned      semen;
} visiuncula_t;

/* ================================================================
 * genera visionum
 * ================================================================ */

#include "visiones/torus.h"
#include "visiones/navis.h"
#include "visiones/zeppelinus.h"

/* ================================================================
 * visio_t — unio omnium generum visionum
 *
 * Continet quodlibet genus visionis in acervo.
 * Accessio basis: visio.visiuncula.genus, visio.visiuncula.semen
 * Accessio generis: visio.torus.methodus, visio.navis.amplitudo, ...
 * ================================================================ */

typedef struct {
    visualis_t qui;

    union {
        torus_t      torus;
        navis_t      navis;
        zeppelinus_t zeppelinus;
    } ubi;
} visio_t;

/* ================================================================
 * functiones
 * ================================================================ */

/*
 * visio_reddere — reddit visionem in fenestram 2048x2048 RGBA.
 * Dispatch per genus.
 */
void visio_reddere(unsigned char *fenestra, const visio_t *visio);

/*
 * visio_ex_ison — legit visionem ex chorda ISON.
 * Reddit visio_t* allocatum (vocans liberet per free()).
 */
visio_t *visio_ex_ison(const char *ison);

/*
 * visio_in_ison — scribit visionem in ISON ad FILE*.
 * Dispatch per genus.
 */
void visio_in_ison(FILE *f, const visio_t *v);

#endif /* VISIO_H */
