#define FEN VISIO_FENESTRA
#define SEMI (FEN / 2)

/* ================================================================
 * torus — per codicem helvea existentem
 * ================================================================ */

static void torus_reddere(unsigned char *fenestra, const torus_t *t)
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

