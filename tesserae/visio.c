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

static color_t color_ex_ison(
    const ison_par_t *pp, int n,
    const char *praefixum_r, const char *praefixum_g,
    const char *praefixum_b,
    double def_r, double def_g, double def_b
) {
    return (color_t){
        ison_pares_f(pp, n, praefixum_r, def_r),
        ison_pares_f(pp, n, praefixum_g, def_g),
        ison_pares_f(pp, n, praefixum_b, def_b),
        1.0
    };
}

visio_t *visio_ex_ison(const char *ison)
{
    ison_par_t pp[64];
    int n = ison_lege(ison, pp, 64);
    if (n <= 0)
        return NULL;

    const char *g = ison_pares_s(pp, n, "genus");

    if (strcmp(g, "torus") == 0) {
        visio_t *v = (visio_t *)calloc(1, sizeof(visio_t));
        if (!v)
            return NULL;

        v->qui = VISIO_TORUS;
        v->ubi.torus.pro.semen = (unsigned)ison_pares_n(pp, n, "semen", 42);

        const char *m = ison_pares_s(pp, n, "methodus");
        if (strcmp(m, "borrelli_t") == 0)
            v->ubi.torus.res.methodus = HELVEA_BORRELLI_T;
        else if (strcmp(m, "planus") == 0)
            v->ubi.torus.res.methodus = HELVEA_PLANUS;
        else
            v->ubi.torus.res.methodus = HELVEA_BORRELLI;

        v->ubi.torus.res.thema        = (int)ison_pares_n(pp, n, "thema", 0);
        v->ubi.torus.res.strata       = (int)ison_pares_n(pp, n, "strata", 0);
        v->ubi.torus.res.radius_maior = ison_pares_f(pp, n, "radius_maior", HELVEA_RADIUS_MAIOR);
        v->ubi.torus.res.radius_minor = ison_pares_f(pp, n, "radius_minor", HELVEA_RADIUS_MINOR);
        v->ubi.torus.res.gradus_u     = (int)ison_pares_n(pp, n, "gradus_u", 800);
        v->ubi.torus.res.gradus_v     = (int)ison_pares_n(pp, n, "gradus_v", 400);
        v->ubi.torus.res.distantia    = ison_pares_f(pp, n, "distantia", 3.5);
        v->ubi.torus.res.elevatio     = ison_pares_f(pp, n, "elevatio", 0.55);
        v->ubi.torus.res.azimuthus    = ison_pares_f(pp, n, "azimuthus", -0.65);

        return v;
    }

    if (strcmp(g, "navis") == 0) {
        visio_t *v = (visio_t *)calloc(1, sizeof(visio_t));
        if (!v)
            return NULL;

        v->qui = VISIO_NAVIS;
        v->ubi.navis.pro.semen = (unsigned)ison_pares_n(pp, n, "semen", 42);

        v->ubi.navis.res.centrum_re   = ison_pares_f(pp, n, "centrum_re", -1.7557);
        v->ubi.navis.res.centrum_im   = ison_pares_f(pp, n, "centrum_im", -0.0175);
        v->ubi.navis.res.amplitudo    = ison_pares_f(pp, n, "amplitudo", 0.08);
        v->ubi.navis.res.iterationes  = (int)ison_pares_n(pp, n, "iterationes", 1000);
        v->ubi.navis.res.color_cyclus = ison_pares_f(pp, n, "color_cyclus", 8.0);
        v->ubi.navis.res.color_phase  = ison_pares_f(pp, n, "color_phase", 0.0);
        v->ubi.navis.res.saturatio    = ison_pares_f(pp, n, "saturatio", 1.0);

        return v;
    }

    if (strcmp(g, "zeppelinus") == 0) {
        visio_t *v = (visio_t *)calloc(1, sizeof(visio_t));
        if (!v)
            return NULL;

        v->qui = VISIO_ZEPPELINUS;
        v->ubi.zeppelinus.pro.semen = (unsigned)ison_pares_n(pp, n, "semen", 42);

        v->ubi.zeppelinus.res.ratio        = ison_pares_f(pp, n, "ratio", 4.0);
        v->ubi.zeppelinus.res.inclinatio   = ison_pares_f(pp, n, "inclinatio", 0.0);
        v->ubi.zeppelinus.res.azimuthus    = ison_pares_f(pp, n, "azimuthus", 0.0);
        v->ubi.zeppelinus.res.lux_angulus  = ison_pares_f(pp, n, "lux_angulus", 0.8);
        v->ubi.zeppelinus.res.lux_elevatio = ison_pares_f(pp, n, "lux_elevatio", 0.6);
        v->ubi.zeppelinus.res.involucrum   = color_ex_ison(
            pp, n, "involucrum_r", "involucrum_g", "involucrum_b",
            0.60, 0.62, 0.65
        );
        v->ubi.zeppelinus.res.gondola = color_ex_ison(
            pp, n, "gondola_r", "gondola_g", "gondola_b",
            0.25, 0.18, 0.12
        );
        v->ubi.zeppelinus.res.elevatio  = ison_pares_f(pp, n, "elevatio", 0.15);
        v->ubi.zeppelinus.res.fenestrae = ison_pares_f(pp, n, "fenestrae", 6.0);
        v->ubi.zeppelinus.res.pinnae    = ison_pares_f(pp, n, "pinnae", 0.4);

        return v;
    }

    return NULL;
}
