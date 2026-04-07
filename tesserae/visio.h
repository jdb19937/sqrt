/*
 * visio.h — visiones proximae, 2048x2048
 *
 * Res quae prope conspiciuntur: torus, fractale, zeppelinus.
 * Planetae 512x512 sunt; visiones 2048x2048.
 * Hereditas C99: primum membrum cuiusque subtypi est visio_t.
 */

#ifndef VISIO_H
#define VISIO_H

#include <math.h>

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
} visio_genus_t;

#define VISIO_NUMERUS_GENERUM 3

/* ================================================================
 * visio_t — basis communis (primum membrum cuiusque subtypi)
 * ================================================================ */

typedef struct {
    visio_genus_t genus;
    unsigned      semen;
} visio_t;

/* ================================================================
 * genera visionum
 * ================================================================ */

#include "visiones/torus.h"
#include "visiones/navis.h"
#include "visiones/zeppelinus.h"

/* ================================================================
 * functiones
 * ================================================================ */

/*
 * visio_reddere — reddit visionem in fenestram 2048x2048 RGBA.
 * Dispatch per genus. visio debet ad subtypum rectum punctare.
 */
void visio_reddere(unsigned char *fenestra, const visio_t *visio);

/*
 * visio_ex_ison — legit visionem ex chorda ISON.
 * Reddit visio_t* ad subtypum allocatum (vocans liberet per free()).
 */
visio_t *visio_ex_ison(const char *ison);

#endif /* VISIO_H */
