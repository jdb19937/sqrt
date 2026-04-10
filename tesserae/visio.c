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
    char *g = ison_da_chordam(ison, "qui");
    if (!g)
        return NULL;

    if (strcmp(g, "torus") == 0) {
        free(g);
        visio_t *v = (visio_t *)calloc(1, sizeof(visio_t));
        if (!v)
            return NULL;

        v->qui = VISIO_TORUS;
        v->ubi.torus.pro.semen = (unsigned)ison_da_n(ison, "ubi.pro.semen", 42);

        char *m = ison_da_chordam(ison, "ubi.res.methodus");
        if (m && strcmp(m, "borrelli_t") == 0)
            v->ubi.torus.res.methodus = HELVEA_BORRELLI_T;
        else if (m && strcmp(m, "planus") == 0)
            v->ubi.torus.res.methodus = HELVEA_PLANUS;
        else
            v->ubi.torus.res.methodus = HELVEA_BORRELLI;
        free(m);

        v->ubi.torus.res.thema        = (int)ison_da_n(ison, "ubi.res.thema", 0);
        v->ubi.torus.res.strata       = (int)ison_da_n(ison, "ubi.res.strata", 0);
        v->ubi.torus.res.radius_maior = ison_da_f(ison, "ubi.res.radius_maior", HELVEA_RADIUS_MAIOR);
        v->ubi.torus.res.radius_minor = ison_da_f(ison, "ubi.res.radius_minor", HELVEA_RADIUS_MINOR);
        v->ubi.torus.res.gradus_u     = (int)ison_da_n(ison, "ubi.res.gradus_u", 800);
        v->ubi.torus.res.gradus_v     = (int)ison_da_n(ison, "ubi.res.gradus_v", 400);
        v->ubi.torus.res.distantia    = ison_da_f(ison, "ubi.res.distantia", 3.5);
        v->ubi.torus.res.elevatio     = ison_da_f(ison, "ubi.res.elevatio", 0.55);
        v->ubi.torus.res.azimuthus    = ison_da_f(ison, "ubi.res.azimuthus", -0.65);

        return v;
    }

    if (strcmp(g, "navis") == 0) {
        free(g);
        visio_t *v = (visio_t *)calloc(1, sizeof(visio_t));
        if (!v)
            return NULL;

        v->qui = VISIO_NAVIS;
        v->ubi.navis.pro.semen = (unsigned)ison_da_n(ison, "ubi.pro.semen", 42);

        v->ubi.navis.res.centrum_re   = ison_da_f(ison, "ubi.res.centrum_re", -1.7557);
        v->ubi.navis.res.centrum_im   = ison_da_f(ison, "ubi.res.centrum_im", -0.0175);
        v->ubi.navis.res.amplitudo    = ison_da_f(ison, "ubi.res.amplitudo", 0.08);
        v->ubi.navis.res.iterationes  = (int)ison_da_n(ison, "ubi.res.iterationes", 1000);
        v->ubi.navis.res.color_cyclus = ison_da_f(ison, "ubi.res.color_cyclus", 8.0);
        v->ubi.navis.res.color_phase  = ison_da_f(ison, "ubi.res.color_phase", 0.0);
        v->ubi.navis.res.saturatio    = ison_da_f(ison, "ubi.res.saturatio", 1.0);

        return v;
    }

    if (strcmp(g, "zeppelinus") == 0) {
        free(g);
        visio_t *v = (visio_t *)calloc(1, sizeof(visio_t));
        if (!v)
            return NULL;

        v->qui = VISIO_ZEPPELINUS;
        v->ubi.zeppelinus.pro.semen = (unsigned)ison_da_n(ison, "ubi.pro.semen", 42);

        v->ubi.zeppelinus.res.ratio        = ison_da_f(ison, "ubi.res.ratio", 4.0);
        v->ubi.zeppelinus.res.inclinatio   = ison_da_f(ison, "ubi.res.inclinatio", 0.0);
        v->ubi.zeppelinus.res.azimuthus    = ison_da_f(ison, "ubi.res.azimuthus", 0.0);
        v->ubi.zeppelinus.res.lux_angulus  = ison_da_f(ison, "ubi.res.lux_angulus", 0.8);
        v->ubi.zeppelinus.res.lux_elevatio = ison_da_f(ison, "ubi.res.lux_elevatio", 0.6);
        v->ubi.zeppelinus.res.involucrum   = (color_t){
            ison_da_f(ison, "ubi.res.involucrum_r", 0.60),
            ison_da_f(ison, "ubi.res.involucrum_g", 0.62),
            ison_da_f(ison, "ubi.res.involucrum_b", 0.65),
            1.0
        };
        v->ubi.zeppelinus.res.gondola = (color_t){
            ison_da_f(ison, "ubi.res.gondola_r", 0.25),
            ison_da_f(ison, "ubi.res.gondola_g", 0.18),
            ison_da_f(ison, "ubi.res.gondola_b", 0.12),
            1.0
        };
        v->ubi.zeppelinus.res.elevatio  = ison_da_f(ison, "ubi.res.elevatio", 0.15);
        v->ubi.zeppelinus.res.fenestrae = ison_da_f(ison, "ubi.res.fenestrae", 6.0);
        v->ubi.zeppelinus.res.pinnae    = ison_da_f(ison, "ubi.res.pinnae", 0.4);

        return v;
    }

    free(g);
    return NULL;
}
