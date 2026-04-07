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

#include "../helvea.h"
#include "../color.h"

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
 * visio_torus_t — torus planus corrugatus (per helvea)
 *
 * Adhibet codicem helvea existentem: superficiem, illuminationem,
 * themata. Parametri camerae et corrugationis exponuntur.
 * ================================================================ */

typedef struct {
    visio_t            basis;
    helvea_methodus_t  methodus;       /* HELVEA_BORRELLI etc. */
    int                thema;          /* index in helvea_themata[] */
    int                strata;         /* profunditas corrugationis (0=praefinitum) */
    double             radius_maior;   /* R tori (praefinitum 1.0) */
    double             radius_minor;   /* r tori (praefinitum 0.42) */
    int                gradus_u;       /* resolutio u (praefinitum 800) */
    int                gradus_v;       /* resolutio v (praefinitum 400) */
    double             distantia;      /* distantia camerae (praefinitum 3.5) */
    double             elevatio;       /* angulus elevationis (radiani) */
    double             azimuthus;      /* angulus azimuthi (radiani) */
} visio_torus_t;

/* ================================================================
 * visio_navis_t — navis ardens (burning ship fractal)
 *
 * Fractale "burning ship": z_{n+1} = (|Re(z_n)| + i|Im(z_n)|)^2 + c
 * Michalski & Życzkowski (2003). Focus ad primam navem minimam.
 * ================================================================ */

typedef struct {
    visio_t basis;
    double  centrum_re;      /* centrum regionis (reale) */
    double  centrum_im;      /* centrum regionis (imaginarium) */
    double  amplitudo;       /* latitudo regionis in plano complexo */
    int     iterationes;     /* maximum iterationum */
    double  color_cyclus;    /* periodus cycli coloris */
    double  color_phase;     /* phase initialis coloris */
    double  saturatio;       /* saturatio colorum 0..2 */
} visio_navis_t;

/* ================================================================
 * visio_zeppelinus_t — zeppelinus aerius
 *
 * Corpus ellipsoidale cum gondola et pinnis.
 * Illuminatio Phong simplex.
 * ================================================================ */

typedef struct {
    visio_t basis;
    double  ratio;           /* ratio longitudinis ad diametrum (praef. 4.0) */
    double  inclinatio;      /* angulus inclinationis (radiani) */
    double  azimuthus;       /* rotatio circa axem verticalem */
    double  lux_angulus;     /* directio lucis (radiani) */
    double  lux_elevatio;    /* elevatio lucis (radiani) */
    color_t involucrum;      /* color involucri */
    color_t gondola;         /* color gondolae */
    double  elevatio;        /* elevatio camerae (radiani, praef. 0.15) */
    double  fenestrae;       /* numerus fenestrarum gondolae */
    double  pinnae;          /* magnitudo pinnarum 0..1 */
} visio_zeppelinus_t;

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
