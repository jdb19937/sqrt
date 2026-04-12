/*
 * visio.c — visiones proximae, renderer
 *
 * Torus per helvea, navis ardens per escape-time,
 * zeppelinus per raymarching analyticum.
 */

#include "visio_communia.h"

#include <stdio.h>

/* ================================================================
 * dispatch
 * ================================================================ */

void visio_reddere(unsigned char *fenestra, const visio_t *visio)
{
    switch (visio->qui) {
    case VISIO_TORUS:
        torus_reddere(fenestra, &visio->ubi.torus);
        break;
    case VISIO_NAVIS:
        navis_reddere(fenestra, &visio->ubi.navis);
        break;
    case VISIO_ZEPPELINUS:
        zeppelinus_reddere(fenestra, &visio->ubi.zeppelinus);
        break;
    }
}

void visio_in_ison(FILE *f, const visio_t *v)
{
    switch (v->qui) {
    case VISIO_TORUS:      torus_in_ison(f, &v->ubi.torus);           break;
    case VISIO_NAVIS:      navis_in_ison(f, &v->ubi.navis);           break;
    case VISIO_ZEPPELINUS: zeppelinus_in_ison(f, &v->ubi.zeppelinus); break;
    }
}

/* ================================================================
 * ISON lector
 * ================================================================ */


visio_t *visio_ex_ison(const char *ison)
{
    /* detege genus ex clavibus praesentibus */
    visualis_t g;
    char *tmp;

    if ((tmp = ison_da_crudum(ison, "toriculus")))
        { free(tmp); g = VISIO_TORUS; }
    else if ((tmp = ison_da_crudum(ison, "naviculus")))
        { free(tmp); g = VISIO_NAVIS; }
    else if ((tmp = ison_da_crudum(ison, "zeppelinulus")))
        { free(tmp); g = VISIO_ZEPPELINUS; }
    else
        return NULL;

    visio_t *v = calloc(1, sizeof(visio_t));
    if (!v) return NULL;
    v->qui = g;

    switch (g) {
    case VISIO_TORUS:      torus_ex_ison(&v->ubi.torus, ison);           break;
    case VISIO_NAVIS:      navis_ex_ison(&v->ubi.navis, ison);           break;
    case VISIO_ZEPPELINUS: zeppelinus_ex_ison(&v->ubi.zeppelinus, ison); break;
    }
    return v;
}
