#include "../visio_communia.h"

/* ================================================================
 * torus — per codicem helvea existentem
 * ================================================================ */

void torus_reddere(unsigned char *fenestra, const torus_t *t)
{
    int gu   = t->res.gradus_u > 0 ? t->res.gradus_u : 800;
    int gv   = t->res.gradus_v > 0 ? t->res.gradus_v : 400;
    double R = t->res.radius_maior > 0 ? t->res.radius_maior : HELVEA_RADIUS_MAIOR;
    double r = t->res.radius_minor > 0 ? t->res.radius_minor : HELVEA_RADIUS_MINOR;

    /* strata corrugationis */
    helvea_strata = t->res.strata;

    /* thema */
    int th = t->res.thema;
    if (th < 0 || th >= helvea_numerus_thematum)
        th = 0;
    helvea_index_thematis = th;

    /* superficiem praecomputare */
    size_t n_vert  = (size_t)(gu + 1) * (gv + 1);
    vec3_t *puncta = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *normae = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    if (!puncta || !normae) {
        free(puncta);
        free(normae);
        return;
    }

    helvea_superficiem_computare(puncta, normae, gu, gv, R, r, t->res.methodus);

    /* tabula imaginis */
    size_t n_pix = (size_t)FEN * FEN;
    tabula_t tab;
    tab.latitudo      = FEN;
    tab.altitudo      = FEN;
    tab.bytes_pixel   = 3;
    tab.imaginis      = (unsigned char *)calloc(n_pix * 3, 1);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));
    if (!tab.imaginis || !tab.profunditatis) {
        free(puncta);
        free(normae);
        free(tab.imaginis);
        free(tab.profunditatis);
        return;
    }
    tabulam_purgare(&tab);

    /* camera */
    double dist = t->res.distantia > 0 ? t->res.distantia : 3.5;
    double elev = t->res.elevatio;
    double azim = t->res.azimuthus;
    vec3_t pos = vec3(
        dist * cos(elev) * cos(azim),
        dist * cos(elev) * sin(azim),
        dist * sin(elev)
    );
    vec3_t scopus = vec3(0.0, 0.0, -0.05);
    camera_t cam  = cameram_constituere(pos, scopus);

    /* scaenam reddere */
    scaenam_reddere(
        &tab, puncta, normae, gu, gv,
        &cam, helvea_illuminare_thema, pixel_rgb
    );

    /* RGB → RGBA convertere */
    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            int si = (y * FEN + x) * 3;
            int di = (y * FEN + x) * 4;
            unsigned char cr = tab.imaginis[si + 0];
            unsigned char cg = tab.imaginis[si + 1];
            unsigned char cb = tab.imaginis[si + 2];
            fenestra[di + 0] = cr;
            fenestra[di + 1] = cg;
            fenestra[di + 2] = cb;
            /* alpha: opacum si non nigrum */
            fenestra[di + 3] = (cr || cg || cb) ? 255 : 0;
        }
    }

    free(puncta);
    free(normae);
    free(tab.imaginis);
    free(tab.profunditatis);
}

void torus_ex_ison(torus_t *v, const char *ison)
{
    v->pro.semen = (unsigned)ison_da_n(ison, "visiuncula.semen", 42);

    char *m = ison_da_chordam(ison, "toriculus.methodus");
    if (!m) return;
    if (strcmp(m, "borrelli") == 0)
        v->res.methodus = HELVEA_BORRELLI;
    else if (strcmp(m, "borrelli_t") == 0)
        v->res.methodus = HELVEA_BORRELLI_T;
    else if (strcmp(m, "planus") == 0)
        v->res.methodus = HELVEA_PLANUS;
    else { free(m); return; }
    free(m);

    v->res.thema        = (int)ison_da_n(ison, "toriculus.thema", 0);
    v->res.strata       = (int)ison_da_n(ison, "toriculus.strata", 0);
    v->res.radius_maior = ison_da_f(ison, "toriculus.radius_maior", HELVEA_RADIUS_MAIOR);
    v->res.radius_minor = ison_da_f(ison, "toriculus.radius_minor", HELVEA_RADIUS_MINOR);
    v->res.gradus_u     = (int)ison_da_n(ison, "toriculus.gradus_u", 800);
    v->res.gradus_v     = (int)ison_da_n(ison, "toriculus.gradus_v", 400);
    v->res.distantia    = ison_da_f(ison, "toriculus.distantia", 3.5);
    v->res.elevatio     = ison_da_f(ison, "toriculus.elevatio", 0.55);
    v->res.azimuthus    = ison_da_f(ison, "toriculus.azimuthus", -0.65);
}

void torus_in_ison(FILE *f, const torus_t *s)
{
    fprintf(f, "{\"visiuncula\": {\"semen\": %u}", s->pro.semen);

    const char *m;
    switch (s->res.methodus) {
    case HELVEA_BORRELLI:   m = "borrelli";   break;
    case HELVEA_BORRELLI_T: m = "borrelli_t"; break;
    case HELVEA_PLANUS:     m = "planus";     break;
    default:                m = "borrelli";   break;
    }

    fprintf(f, ", \"toriculus\": {\"methodus\": \"%s\", \"thema\": %d, \"strata\": %d, \"radius_maior\": %.6f, \"radius_minor\": %.6f, \"gradus_u\": %d, \"gradus_v\": %d, \"distantia\": %.6f, \"elevatio\": %.6f, \"azimuthus\": %.6f}}",
        m, s->res.thema, s->res.strata,
        s->res.radius_maior, s->res.radius_minor,
        s->res.gradus_u, s->res.gradus_v,
        s->res.distantia, s->res.elevatio, s->res.azimuthus);
}

