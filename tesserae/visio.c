/*
 * visio.c — visiones proximae, renderer
 *
 * Torus per helvea, navis ardens per escape-time,
 * zeppelinus per raymarching analyticum.
 */

#include "visio.h"
#include "../pictura.h"

#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visiones/torus.c"
#include "visiones/navis.c"
#include "visiones/zeppelinus.c"

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

/* ================================================================
 * ISON lector
 * ================================================================ */


visio_t *visio_ex_ison(const char *ison)
{
    /* detege genus ex clavibus praesentibus */
    char *tmp;
    if ((tmp = ison_da_crudum(ison, "toriculus")))
        { free(tmp); return torus_ex_ison(ison); }
    if ((tmp = ison_da_crudum(ison, "naviculus")))
        { free(tmp); return navis_ex_ison(ison); }
    if ((tmp = ison_da_crudum(ison, "zeppelinulus")))
        { free(tmp); return zeppelinus_ex_ison(ison); }
    return NULL;
}
