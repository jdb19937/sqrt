/*
 * torus_specta.c — spectator interactivus tori Hevea per phantasma (PFR)
 *
 * Torum planum corrugatum in fenestra PFR reddit.
 * Usitor camerae rotationem et conspectum clavibus regit.
 *
 * Claves:
 *   Sagittae sin/dex    — azimuthum camerae mutare
 *   Sagittae sur/deor   — elevationem camerae mutare
 *   W / S               — propinquare / recedere
 *   X / Y / Z           — axem rotationis automaticae eligere
 *   Spatium              — rotationem automaticam sistere / resumere
 *   +/- (vel =/-)       — celeritatem rotationis mutare
 *   Tab                  — thema proximum
 *   1/2                  — radium maiorem minuere / augere
 *   3/4                  — radium minorem minuere / augere
 *   Rota muris           — propinquare / recedere
 *   C                    — inscriptionem incipere / finire (→ MP4)
 *   L                    — unum cyclum inscribere (→ MP4 + GIF)
 *   R                    — restituere
 *   Q / Escape           — exire
 */

#include "helvea.h"
#include "instrumentum.h"
#include "sidus.h"
#include "campus.h"
#include "ison.h"

#include "phantasma.h"
#include "computo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define LATITUDO_IMG  1024
#define ALTITUDO_IMG  1024

/* ================================================================
 * catalogum siderum — ex ISONL extractum
 * ================================================================ */

#define SIDERA_MAX 200000

typedef struct {
    int    x, y;
    double temperatura;
    double magnitudo;
    char   genus[24];
} sidus_registrum_t;

static sidus_registrum_t sidera[SIDERA_MAX];
static int numerus_siderum = 0;

static void catalogum_linea(const char *linea, void *ctx)
{
    (void)ctx;
    char *sidus_raw = ison_da_crudum(linea, "sidus");
    if (!sidus_raw)
        return;
    if (numerus_siderum >= SIDERA_MAX) {
        free(sidus_raw);
        return;
    }

    sidus_registrum_t *s = &sidera[numerus_siderum];
    s->x           = (int)ison_da_n(sidus_raw, "x", 0);
    s->y           = (int)ison_da_n(sidus_raw, "y", 0);
    s->temperatura = ison_da_f(sidus_raw, "temperatura", 5000);
    s->magnitudo   = ison_da_f(sidus_raw, "magnitudo", 6.0);

    char *g = ison_da_chordam(sidus_raw, "genus");
    if (g) {
        strncpy(s->genus, g, 23);
        s->genus[23] = '\0';
        free(g);
    } else
        strcpy(s->genus, "sequentia");

    free(sidus_raw);
    numerus_siderum++;
}

/* indices 256 siderum lucidissimorum (magnitudo minima = lucidissimum) */
static int sidera_lucida[256];
static int numerus_lucidorum = 0;

static int cmp_magnitudo(const void *a, const void *b)
{
    double ma = sidera[*(const int *)a].magnitudo;
    double mb = sidera[*(const int *)b].magnitudo;
    return (ma < mb) ? -1 : (ma > mb) ? 1 : 0;
}

static void sidera_lucida_computare(void)
{
    int *indices = (int *)malloc((size_t)numerus_siderum * sizeof(int));
    if (!indices) {
        numerus_lucidorum = 0;
        return;
    }
    for (int i = 0; i < numerus_siderum; i++)
        indices[i] = i;
    qsort(indices, (size_t)numerus_siderum, sizeof(int), cmp_magnitudo);
    numerus_lucidorum = numerus_siderum < 256 ? numerus_siderum : 256;
    for (int i = 0; i < numerus_lucidorum; i++)
        sidera_lucida[i] = indices[i];
    free(indices);
}

static void catalogum_legere(const char *via_isonl)
{
    numerus_siderum = 0;
    char *isonl     = ison_lege_plicam(via_isonl);
    if (!isonl)
        return;
    ison_pro_quaque_linea_s(isonl, catalogum_linea, NULL);
    free(isonl);
    sidera_lucida_computare();
    fprintf(
        stderr, "Catalogum: %d sidera, %d lucida.\n",
        numerus_siderum, numerus_lucidorum
    );
}

/* inveni sidus proximum inter 100 lucidissima, intra radium 40px */
static int inveni_sidus(int cx, int cy, int cam_lat, int cam_alt)
{
    int optimus  = -1;
    int dist_min = 999999999;

    for (int i = 0; i < numerus_lucidorum; i++) {
        int si = sidera_lucida[i];
        int dx = sidera[si].x - cx;
        int dy = sidera[si].y - cy;
        if (dx >  cam_lat / 2)
            dx -= cam_lat;
        if (dx < -cam_lat / 2)
            dx += cam_lat;
        if (dy >  cam_alt / 2)
            dy -= cam_alt;
        if (dy < -cam_alt / 2)
            dy += cam_alt;
        int d2 = dx * dx + dy * dy;
        if (d2 < 40 * 40 && d2 < dist_min) {
            dist_min = d2;
            optimus  = si;
        }
    }
    return optimus;
}

/* inveni cosidus — sidus lucidum propinquum */
static int inveni_cosidus(int prim, int cam_lat, int cam_alt)
{
    sidus_registrum_t *p = &sidera[prim];
    int optimus  = -1;
    int dist_min = 999999999;

    for (int i = 0; i < numerus_siderum; i++) {
        if (i == prim)
            continue;
        if (sidera[i].magnitudo > 3.0)
            continue;

        int dx = sidera[i].x - p->x;
        int dy = sidera[i].y - p->y;
        if (dx >  cam_lat / 2)
            dx -= cam_lat;
        if (dx < -cam_lat / 2)
            dx += cam_lat;
        if (dy >  cam_alt / 2)
            dy -= cam_alt;
        if (dy < -cam_alt / 2)
            dy += cam_alt;
        int d2 = dx * dx + dy * dy;
        if (d2 < 20 * 20 && d2 < dist_min) {
            dist_min = d2;
            optimus  = i;
        }
    }
    return optimus;
}

/* ================================================================
 * curvatura — animatio warp
 * ================================================================ */

#define CURV_TABULAE 36

static void hsv_ad_rgb(
    double h, double sat, double v,
    unsigned char *r, unsigned char *g, unsigned char *b
) {
    double c  = v * sat;
    double hp = fmod(h * 6.0, 6.0);
    if (hp < 0)
        hp += 6.0;
    double x = c * (1.0 - fabs(fmod(hp, 2.0) - 1.0));
    double m = v - c;
    double r1, g1, b1;
    if (hp < 1) {
        r1 = c;
        g1 = x;
        b1 = 0;
    } else if (hp < 2) {
        r1 = x;
        g1 = c;
        b1 = 0;
    } else if (hp < 3) {
        r1 = 0;
        g1 = c;
        b1 = x;
    } else if (hp < 4) {
        r1 = 0;
        g1 = x;
        b1 = c;
    } else if (hp < 5) {
        r1 = x;
        g1 = 0;
        b1 = c;
    } else {
        r1 = c;
        g1 = 0;
        b1 = x;
    }
    *r = (unsigned char)((r1 + m) * 255);
    *g = (unsigned char)((g1 + m) * 255);
    *b = (unsigned char)((b1 + m) * 255);
}

static unsigned int curv_semen = 1;
static unsigned int curv_alea(void)
{
    curv_semen ^= curv_semen << 13;
    curv_semen ^= curv_semen >> 17;
    curv_semen ^= curv_semen << 5;
    return curv_semen;
}
static double curv_alea_f(void)
{
    return (double)(curv_alea() & 0xFFFFFF) / (double)0x1000000;
}

/* una linea curvatiae — origo prope centrum, directio libera */
typedef struct {
    double ox, oy;       /* origo */
    double dx, dy;       /* directio (normalizata) */
    double longitudo;    /* longitudo maxima */
    double velocitas;    /* multiplicator progressus */
    int    latitudo;     /* pixeles */
    unsigned char r, g, b;
} curv_linea_t;

#define CURV_LINEAE_MAX 300
static curv_linea_t curv_lineae[CURV_LINEAE_MAX];
static int curv_n_linearum = 0;

static void curvatura_lineas_generare(int lat_img, int alt_img)
{
    int cx      = lat_img / 2;
    int cy      = alt_img / 2;
    double diag = sqrt((double)(cx * cx + cy * cy));

    curv_n_linearum = 180 + (int)(curv_alea_f() * 120);
    if (curv_n_linearum > CURV_LINEAE_MAX)
        curv_n_linearum = CURV_LINEAE_MAX;

    for (int i = 0; i < curv_n_linearum; i++) {
        curv_linea_t *l = &curv_lineae[i];

        /* origo: circa centrum, dispersio ~80px */
        l->ox = cx + (curv_alea_f() - 0.5) * 160.0;
        l->oy = cy + (curv_alea_f() - 0.5) * 160.0;

        /* directio: omnino libera */
        double ang = curv_alea_f() * DUO_PI;
        l->dx      = cos(ang);
        l->dy      = sin(ang);

        l->longitudo = diag * (0.6 + curv_alea_f() * 0.8);
        l->velocitas = 0.6 + curv_alea_f() * 0.8;
        l->latitudo  = 1 + (int)(curv_alea_f() * 2.5);

        /* colores: praecipue caerulei/albi/violacei, rari calidi */
        unsigned char cr, cg, cb;
        double h = curv_alea_f();
        if (h < 0.45)
            hsv_ad_rgb(
                0.55 + curv_alea_f() * 0.25,
                0.25 + curv_alea_f() * 0.5,
                0.7 + curv_alea_f() * 0.3, &cr, &cg, &cb
            );
        else if (h < 0.7)
            hsv_ad_rgb(
                0.58 + curv_alea_f() * 0.05,
                0.03 + curv_alea_f() * 0.12,
                0.85 + curv_alea_f() * 0.15, &cr, &cg, &cb
            );
        else if (h < 0.85)
            hsv_ad_rgb(
                0.72 + curv_alea_f() * 0.15,
                0.35 + curv_alea_f() * 0.45,
                0.6 + curv_alea_f() * 0.4, &cr, &cg, &cb
            );
        else
            hsv_ad_rgb(
                curv_alea_f() * 0.08 + 0.95,
                0.5 + curv_alea_f() * 0.4,
                0.6 + curv_alea_f() * 0.4, &cr, &cg, &cb
            );

        l->r = cr;
        l->g = cg;
        l->b = cb;
    }
}

static void curvatura_reddere(tabula_t *t, int tabula)
{
    double p = (double)tabula / CURV_TABULAE;

    /* genera lineas in prima tabula */
    if (tabula == 0) {
        curv_semen = 77701u;
        curvatura_lineas_generare(t->latitudo, t->altitudo);
    }

    for (int i = 0; i < curv_n_linearum; i++) {
        curv_linea_t *l = &curv_lineae[i];
        double prog     = p * l->velocitas;
        double l_cur    = l->longitudo * prog * prog;
        double vis_base = 0.25 + 0.75 * p;

        /* linea ab origine in directionem */
        for (double rr = 0; rr < l_cur; rr += 0.5) {
            double vis = (1.0 - rr / l_cur) * vis_base;

            int bx = (int)(l->ox + rr * l->dx);
            int by = (int)(l->oy + rr * l->dy);

            for (int w = -(l->latitudo / 2); w <= l->latitudo / 2; w++) {
                int px = bx + (int)(w * l->dy);
                int py = by - (int)(w * l->dx);
                if (px < 0 || px >= t->latitudo)
                    continue;
                if (py < 0 || py >= t->altitudo)
                    continue;

                int idx = (py * t->latitudo + px) * 4;
                int nb  = t->imaginis[idx + 0] + (int)(l->b * vis);
                int ng  = t->imaginis[idx + 1] + (int)(l->g * vis);
                int nr  = t->imaginis[idx + 2] + (int)(l->r * vis);
                if (nb > 255)
                    nb = 255;
                if (ng > 255)
                    ng = 255;
                if (nr > 255)
                    nr = 255;
                t->imaginis[idx + 0] = (unsigned char)nb;
                t->imaginis[idx + 1] = (unsigned char)ng;
                t->imaginis[idx + 2] = (unsigned char)nr;
            }
        }
    }

    /* flash album ad finem */
    if (p > 0.7) {
        double album = (p - 0.7) / 0.3;
        album *= album * album;
        unsigned char a = (unsigned char)(album * 255);
        for (int y = 0; y < t->altitudo; y++) {
            for (int x = 0; x < t->latitudo; x++) {
                int idx = (y * t->latitudo + x) * 4;
                int b   = t->imaginis[idx + 0] + a;
                int g   = t->imaginis[idx + 1] + a;
                int r   = t->imaginis[idx + 2] + a;
                if (b > 255)
                    b = 255;
                if (g > 255)
                    g = 255;
                if (r > 255)
                    r = 255;
                t->imaginis[idx + 0] = (unsigned char)b;
                t->imaginis[idx + 1] = (unsigned char)g;
                t->imaginis[idx + 2] = (unsigned char)r;
            }
        }
    }
}
#define GRADUS_U_INIT 300
#define GRADUS_V_INIT 150
#define GRADUS_MIN    40
#define GRADUS_MAX    2000


/* themata et illuminatio thematica nunc in helvea.h/c */

/* ================================================================
 * post-effecta
 * ================================================================ */

static unsigned int semen_pfx = 12345;
static inline unsigned int aleatorium(void)
{
    semen_pfx ^= semen_pfx << 13;
    semen_pfx ^= semen_pfx >> 17;
    semen_pfx ^= semen_pfx << 5;
    return semen_pfx;
}

static void pfx_applicare(tabula_t *t, int pfx, int posteriza_niv)
{
    if (pfx == HELVEA_PFX_NULLUS)
        return;

    size_t n_pix = (size_t)t->latitudo * t->altitudo;

    unsigned char *limbus = NULL;
    if (pfx & HELVEA_PFX_LINEAE) {
        limbus = (unsigned char *)calloc(n_pix, 1);
        for (int y = 1; y < t->altitudo - 1; y++) {
            for (int x = 1; x < t->latitudo - 1; x++) {
                int c    = y * t->latitudo + x;
                double d = t->profunditatis[c];
                if (d > 1e20)
                    continue;

                double dl = t->profunditatis[c - 1];
                double dr = t->profunditatis[c + 1];
                double du = t->profunditatis[c - t->latitudo];
                double dd = t->profunditatis[c + t->latitudo];

                if (dl > 1e20 || dr > 1e20 || du > 1e20 || dd > 1e20) {
                    limbus[c] = 255;
                    continue;
                }

                double gx = fabs(dr - dl);
                double gy = fabs(dd - du);
                double g  = (gx + gy) * 25.0;
                if (g > 1.0)
                    g = 1.0;
                limbus[c] = (unsigned char)(g * 255.0);
            }
        }
    }

    for (size_t i = 0; i < n_pix; i++) {
        size_t base = i * 4;
        double b    = t->imaginis[base + 0] / 255.0;
        double g    = t->imaginis[base + 1] / 255.0;
        double r    = t->imaginis[base + 2] / 255.0;

        if ((pfx & HELVEA_PFX_POSTERIZA) && posteriza_niv > 0) {
            double nf = (double)posteriza_niv;
            r         = floor(r * nf + 0.5) / nf;
            g         = floor(g * nf + 0.5) / nf;
            b         = floor(b * nf + 0.5) / nf;
        }

        if (pfx & HELVEA_PFX_GRANUM) {
            double rumore = ((double)(aleatorium() & 0xFF) / 255.0 - 0.5) * 0.08;
            r += rumore;
            g += rumore;
            b += rumore;
        }

        if (pfx & HELVEA_PFX_NIGRESCO) {
            int px         = (int)(i % (size_t)t->latitudo);
            int py         = (int)(i / (size_t)t->latitudo);
            double dx      = (px - t->latitudo * 0.5) / (t->latitudo * 0.5);
            double dy      = (py - t->altitudo * 0.5) / (t->altitudo * 0.5);
            double dist    = dx * dx + dy * dy;
            double obscura = 1.0 - dist * 0.45;
            if (obscura < 0.2)
                obscura = 0.2;
            r *= obscura;
            g *= obscura;
            b *= obscura;
        }

        if (limbus && limbus[i] > 0) {
            double lf = limbus[i] / 255.0;
            r *= (1.0 - lf);
            g *= (1.0 - lf);
            b *= (1.0 - lf);
        }

        if (r < 0)
            r = 0;
        if (r > 1)
            r = 1;
        if (g < 0)
            g = 0;
        if (g > 1)
            g = 1;
        if (b < 0)
            b = 0;
        if (b > 1)
            b = 1;
        t->imaginis[base + 0] = (unsigned char)(b * 255.0);
        t->imaginis[base + 1] = (unsigned char)(g * 255.0);
        t->imaginis[base + 2] = (unsigned char)(r * 255.0);
    }

    free(limbus);
}

/* rotatio N vectorum per matricem rotationis (CPU BLAS).
 * axis: 0=x, 1=y, 2=z. Matrices R^T pro GEMM: result = src × R^T. */
static void vertices_rotare(
    vec3_t *dest, vec3_t *fons,
    size_t n, int axis, double angulus
) {
    double ca = cos(angulus), sa = sin(angulus);
    double r[9];

    switch (axis) {
    case 0: /* R_x^T */
        r[0] = 1;
        r[1] = 0;
        r[2] = 0;
        r[3] = 0;
        r[4] = ca;
        r[5] = sa;
        r[6] = 0;
        r[7] = -sa;
        r[8] = ca;
        break;
    case 1: /* R_y^T */
        r[0] = ca;
        r[1] = 0;
        r[2] = -sa;
        r[3] = 0;
        r[4] = 1;
        r[5] = 0;
        r[6] = sa;
        r[7] = 0;
        r[8] = ca;
        break;
    default: /* R_z^T */
        r[0] = ca;
        r[1] = sa;
        r[2] = 0;
        r[3] = -sa;
        r[4] = ca;
        r[5] = 0;
        r[6] = 0;
        r[7] = 0;
        r[8] = 1;
        break;
    }

    pfr_matrix_d_t rot = {3,      3, r,              NULL};
    pfr_matrix_d_t src = {(int)n, 3, (double *)fons, NULL};
    pfr_matrix_d_t dst = {(int)n, 3, (double *)dest, NULL};
    pfr_matmat_d(&dst, &src, &rot);
}

/* ================================================================
 * principium
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    fprintf(stderr, "Superficiem computans...\n");

    double radius_maior = HELVEA_RADIUS_MAIOR;
    double radius_minor = HELVEA_RADIUS_MINOR;
    helvea_methodus_t methodus = HELVEA_BORRELLI;
    helvea_strata = 5;
    int gradus = 6;
    #define GRADUS_U(g) (10 * (1 << (g)))
    #define GRADUS_V(g) (5 * (1 << (g)))

    size_t n_vert       = (size_t)(GRADUS_U(gradus) + 1) * (GRADUS_V(gradus) + 1);
    vec3_t *puncta_orig = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *normae_orig = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *puncta_rot  = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *normae_rot  = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    int gradus_mutatus  = 0;

    if (!puncta_orig || !normae_orig || !puncta_rot || !normae_rot) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    helvea_superficiem_computare(
        puncta_orig, normae_orig,
        GRADUS_U(gradus), GRADUS_V(gradus),
        radius_maior, radius_minor, methodus
    );
    int superficies_obsoleta = 0;
    fprintf(stderr, "Superficies parata: %zu vertices.\n", n_vert);

    /* campum stellarum ex ISONL reddere */
    const char *via_isonl = "caelae/terra.isonl";
    const char *via_instr = "instrumenta/oculus.ison";
    fprintf(stderr, "Campum stellarum reddens: %s + %s\n", via_isonl, via_instr);
    campus_t *campus = campus_ex_isonl_reddere(via_isonl, via_instr);
    if (!campus) {
        fprintf(stderr, "ERROR: campus stellarum reddere non possum!\n");
        return 1;
    }
    fprintf(stderr, "Campus stellarum paratus.\n");

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    tabula_t tab;
    tab.latitudo      = LATITUDO_IMG;
    tab.altitudo      = ALTITUDO_IMG;
    tab.bytes_pixel   = 4;
    tab.imaginis      = (unsigned char *)malloc(n_pix * 4);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (pfr_initia(PFR_INITIA_VIDEO) != 0) {
        fprintf(stderr, "ERROR pfr_initia: %s\n", pfr_erratum());
        return 1;
    }

    pfr_fenestra_t *fenestra = pfr_fenestram_crea(
        "Torus Hevea — Spectator",
        PFR_POS_MEDIUM, PFR_POS_MEDIUM,
        LATITUDO_IMG, ALTITUDO_IMG, 0
    );
    if (!fenestra) {
        fprintf(stderr, "ERROR fenestra: %s\n", pfr_erratum());
        pfr_fini();
        return 1;
    }

    pfr_pictor_t *pictor = pfr_pictorem_crea(
        fenestra, -1, PFR_PICTOR_CELER | PFR_PICTOR_SYNC
    );
    if (!pictor) {
        fprintf(stderr, "ERROR pictor: %s\n", pfr_erratum());
        pfr_fenestram_destrue(fenestra);
        pfr_fini();
        return 1;
    }

    pfr_textura_t *textura = pfr_texturam_crea(
        pictor, PFR_PIXEL_ARGB8888, PFR_TEXTURA_FLUENS,
        LATITUDO_IMG, ALTITUDO_IMG
    );
    if (!textura) {
        fprintf(stderr, "ERROR textura: %s\n", pfr_erratum());
        pfr_pictorem_destrue(pictor);
        pfr_fenestram_destrue(fenestra);
        pfr_fini();
        return 1;
    }

    /* positio in toro T² — duo anguli libere volventes */
    double theta       = 0.6;    /* toroidalis (horizontalis) */
    double phi         = 0.4;    /* poloidalis (verticalis) */
    double distantia   = 6.5;
    double angulus_rot  = 0.0;
    double celeritas    = 0.36;
    int    axis_index   = 1;
    int    rotatio_activa = 1;

    const char *nomina_axium[3] = { "X", "Y", "Z" };

    fprintf(stderr, "Spectator paratus. Claves:\n");
    fprintf(stderr, "  Sagittae   — azimuthum et elevationem\n");
    fprintf(stderr, "  W/S        — propinquare / recedere\n");
    fprintf(stderr, "  X/Y/Z      — axem rotationis eligere\n");
    fprintf(stderr, "  Spatium    — rotationem sistere / resumere\n");
    fprintf(stderr, "  +/-        — celeritatem mutare\n");
    fprintf(stderr, "  Tab        — thema proximum\n");
    fprintf(stderr, "  M          — methodum superficiei mutare\n");
    fprintf(stderr, "  1/2        — radium maiorem (R=%.2f)\n", radius_maior);
    fprintf(stderr, "  3/4        — radium minorem (r=%.2f)\n", radius_minor);
    fprintf(stderr, "  G          — imaginem unam capere (GIF)\n");
    fprintf(stderr, "  C          — inscriptionem incipere / finire (MP4)\n");
    fprintf(stderr, "  L          — unum cyclum rotationis inscribere (MP4 + GIF)\n");
    fprintf(stderr, "  R          — restituere\n");
    fprintf(stderr, "  Q/Escape   — exire\n");
    fprintf(stderr, "Thema [0]: %s\n", helvea_themata[helvea_index_thematis].nomen);
    fprintf(stderr, "Methodus [0]: %s\n", helvea_nomina_methodorum[methodus]);

    /* inscriptio (recording) — directe per phantasma inscriptores */
    pfr_mp4_t *inscr_mp4 = NULL;
    int inscr_tabula     = 0;

    /* orbita inscriptio — unus cyclus exactus */
    pfr_mp4_t *orbita_mp4 = NULL;
    pfr_gif_t *orbita_gif = NULL;
    int orbita_tabula = 0;
    double orbita_angulus_init = 0;
    char orbita_nomen[256] = {0};

    /* catalogum siderum legere */
    catalogum_legere(via_isonl);

    /* curvatura (warp) status */
    int    curvatura_activa = 0;
    int    curv_tabula      = 0;
    pid_t  curv_pid         = 0;
    int    curv_generatum   = 0;

    /* indicium status in terminali */
    char status_nuntius[128] = "";
    int tabulae_fps = 0;
    pfr_u32 tempus_fps = pfr_tempus();

    int currit = 1;
    pfr_u32 tempus_prius = pfr_tempus();

    while (currit) {
        pfr_u32 tempus_nunc = pfr_tempus();
        double dt = (double)(tempus_nunc - tempus_prius) / 1000.0;
        tempus_prius = tempus_nunc;

        pfr_eventus_t eventus;
        while (pfr_eventum_lege(&eventus)) {
            if (eventus.typus == PFR_EXITUS) {
                currit = 0;
            } else if (eventus.typus == PFR_ROTA_MURIS) {
                distantia -= eventus.rota.y * 0.25;
                if (distantia < 1.8)
                    distantia = 1.8;
                if (distantia > 8.0)
                    distantia = 8.0;
                if (orbita_mp4) {
                    pfr_mp4_fini(orbita_mp4);
                    pfr_gif_fini(orbita_gif);
                    orbita_mp4 = NULL;
                    orbita_gif = NULL;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Orbita cancellata"
                    );
                }
            } else if (
                eventus.typus == PFR_MURIS_INF
                && eventus.muris.plectrum == 1
                && !curvatura_activa
            ) {
                /* click sinister — naviga ad sidus */
                int mx = eventus.muris.x;
                int my = eventus.muris.y;

                /* solum si click in fundo (non in toro) */
                int idx_click = my * LATITUDO_IMG + mx;
                if (
                    mx >= 0 && mx < LATITUDO_IMG
                    && my >= 0 && my < ALTITUDO_IMG
                    && tab.profunditatis[idx_click] > 1e20
                    && numerus_siderum > 0
                ) {
                    /* campus coordinatae */
                    int delta_cx = (int)(theta / DUO_PI * campus->latitudo);
                    int delta_cy = (int)(phi   / DUO_PI * campus->altitudo);
                    int campus_cx = (
                        (mx + delta_cx) % campus->latitudo
                        + campus->latitudo
                    ) % campus->latitudo;
                    int campus_cy = (
                        (my + delta_cy) % campus->altitudo
                        + campus->altitudo
                    ) % campus->altitudo;

                    int si = inveni_sidus(
                        campus_cx, campus_cy,
                        campus->latitudo, campus->altitudo
                    );
                    if (si >= 0) {
                        sidus_registrum_t *s = &sidera[si];
                        unsigned int semen_novum =
                            (unsigned int)(
                                s->x * 7919 + s->y * 6271
                                + (int)(s->temperatura)
                            );

                        /* cosidus? */
                        int ci = inveni_cosidus(
                            si, campus->latitudo, campus->altitudo
                        );
                        int habet_co = (ci >= 0 && (semen_novum & 0xFF) < 25);

                        /* scribe sidus ad /tmp */
                        {
                            FILE *fp = fopen("/tmp/sqrt_sidus.ison", "w");
                            if (fp) {
                                fprintf(
                                    fp,
                                    "{\n"
                                    "  \"genus\": \"%s\",\n"
                                    "  \"temperatura\": %.0f\n"
                                    "}\n",
                                    s->genus, s->temperatura
                                );
                                fclose(fp);
                            }
                        }
                        if (habet_co) {
                            FILE *fp = fopen("/tmp/sqrt_cosidus.ison", "w");
                            if (fp) {
                                fprintf(
                                    fp,
                                    "{\n"
                                    "  \"genus\": \"%s\",\n"
                                    "  \"temperatura\": %.0f\n"
                                    "}\n",
                                    sidera[ci].genus, sidera[ci].temperatura
                                );
                                fclose(fp);
                            }
                        }

                        /* fork: genera + caele in fundo */
                        curv_pid = fork();
                        if (curv_pid == 0) {
                            char mandatum[512];
                            if (habet_co)
                                snprintf(
                                    mandatum, sizeof(mandatum),
                                    "./genera /tmp/sqrt_sidus.ison"
                                    " /tmp/sqrt_cosidus.ison %u"
                                    " > /tmp/sqrt_campus.ison"
                                    " && ./caele /tmp/sqrt_campus.ison"
                                    " > /tmp/sqrt_caelae.isonl",
                                    semen_novum
                                );
                            else
                                snprintf(
                                    mandatum, sizeof(mandatum),
                                    "./genera /tmp/sqrt_sidus.ison %u"
                                    " > /tmp/sqrt_campus.ison"
                                    " && ./caele /tmp/sqrt_campus.ison"
                                    " > /tmp/sqrt_caelae.isonl",
                                    semen_novum
                                );
                            _exit(system(mandatum));
                        }

                        curvatura_activa = 1;
                        curv_tabula      = 0;
                        curv_generatum   = 0;

                        snprintf(
                            status_nuntius, sizeof(status_nuntius),
                            "★ CURVATURA: %s T=%.0fK %s",
                            s->genus, s->temperatura,
                            habet_co ? "(binarium)" : ""
                        );
                    }
                }
            } else if (eventus.typus == PFR_CLAVIS_INF) {
                switch (eventus.clavis.signum.symbolum) {
                case PFR_CL_EFFUGIUM:
                case 'q':
                    currit = 0;
                    break;
                case PFR_CL_SINISTRUM:
                case PFR_CL_DEXTRUM:
                case PFR_CL_SURSUM:
                case PFR_CL_DEORSUM:
                    if (eventus.clavis.signum.symbolum == PFR_CL_SINISTRUM)
                        theta -= 0.08;
                    if (eventus.clavis.signum.symbolum == PFR_CL_DEXTRUM)
                        theta += 0.08;
                    if (eventus.clavis.signum.symbolum == PFR_CL_SURSUM)
                        phi   += 0.06;
                    if (eventus.clavis.signum.symbolum == PFR_CL_DEORSUM)
                        phi   -= 0.06;
                    if (orbita_mp4) {
                        pfr_mp4_fini(orbita_mp4);
                        pfr_gif_fini(orbita_gif);
                        orbita_mp4 = NULL;
                        orbita_gif = NULL;
                        snprintf(
                            status_nuntius, sizeof(status_nuntius),
                            "Orbita cancellata"
                        );
                    }
                    break;
                case 'w':
                    distantia -= 0.15;
                    if (distantia < 1.8)
                        distantia = 1.8;
                    break;
                case 's':
                    distantia += 0.15;
                    if (distantia > 8.0)
                        distantia = 8.0;
                    break;
                case 'x':
                    axis_index = 0;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Axis: %s", nomina_axium[axis_index]
                    );
                    break;
                case 'y':
                    axis_index = 1;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Axis: %s", nomina_axium[axis_index]
                    );
                    break;
                case 'z':
                    axis_index = 2;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Axis: %s", nomina_axium[axis_index]
                    );
                    break;
                case PFR_CL_SPATIUM:
                    rotatio_activa = !rotatio_activa;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Rotatio: %s",
                        rotatio_activa ? "activa" : "sistita"
                    );
                    break;
                case PFR_CL_AEQUALE:
                case PFR_CL_PLUS:
                    celeritas *= 1.3;
                    if (celeritas > 6.0)
                        celeritas = 6.0;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Celeritas: %.2f rad/s", celeritas
                    );
                    break;
                case PFR_CL_MINUS:
                    celeritas /= 1.3;
                    if (celeritas < 0.05)
                        celeritas = 0.05;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Celeritas: %.2f rad/s", celeritas
                    );
                    break;
                case PFR_CL_1:
                    radius_maior -= 0.05;
                    if (radius_maior < 0.3)
                        radius_maior = 0.3;
                    superficies_obsoleta = 1;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "R=%.2f r=%.2f", radius_maior, radius_minor
                    );
                    break;
                case PFR_CL_2:
                    radius_maior += 0.05;
                    if (radius_maior > 3.0)
                        radius_maior = 3.0;
                    superficies_obsoleta = 1;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "R=%.2f r=%.2f", radius_maior, radius_minor
                    );
                    break;
                case PFR_CL_3:
                    radius_minor -= 0.02;
                    if (radius_minor < 0.08)
                        radius_minor = 0.08;
                    superficies_obsoleta = 1;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "R=%.2f r=%.2f", radius_maior, radius_minor
                    );
                    break;
                case PFR_CL_4:
                    radius_minor += 0.02;
                    if (radius_minor > 1.5)
                        radius_minor = 1.5;
                    superficies_obsoleta = 1;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "R=%.2f r=%.2f", radius_maior, radius_minor
                    );
                    break;
                case PFR_CL_TABULA:
                    helvea_index_thematis = (helvea_index_thematis + 1) % helvea_numerus_thematum;
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Thema [%d]: %s",
                        helvea_index_thematis,
                        helvea_themata[helvea_index_thematis].nomen
                    );
                    break;
                case 'm':
                    methodus = (methodus + 1) % HELVEA_NUMERUS_METHODORUM;
                    superficies_obsoleta = 1;
                    break;
                case PFR_CL_5:
                    helvea_strata--;
                    if (helvea_strata < 1)
                        helvea_strata = 1;
                    superficies_obsoleta = 1;
                    break;
                case PFR_CL_6:
                    helvea_strata++;
                    if (helvea_strata > HELVEA_STRATA_MAX)
                        helvea_strata = HELVEA_STRATA_MAX;
                    superficies_obsoleta = 1;
                    break;
                case PFR_CL_7:
                    if (gradus > 1) {
                        gradus--;
                        gradus_mutatus       = 1;
                        superficies_obsoleta = 1;
                    }
                    break;
                case PFR_CL_8:
                    if (gradus < 7) {
                        gradus++;
                        gradus_mutatus       = 1;
                        superficies_obsoleta = 1;
                    }
                    break;
                case 'c':
                    if (!inscr_mp4) {
                        /* incipe inscriptionem */
                        time_t nunc = time(NULL);
                        struct tm *tm = localtime(&nunc);
                        char via[256];
                        snprintf(
                            via, sizeof(via),
                            "caelae/inscr_%04d%02d%02d_%02d%02d%02d.mp4",
                            tm->tm_year + 1900, tm->tm_mon + 1,
                            tm->tm_mday, tm->tm_hour,
                            tm->tm_min, tm->tm_sec
                        );
                        inscr_mp4    = pfr_mp4_initia(via, LATITUDO_IMG, ALTITUDO_IMG, 30);
                        inscr_tabula = 0;
                        snprintf(
                            status_nuntius, sizeof(status_nuntius),
                            "● INSCRIPTIO: %s", via
                        );
                    } else {
                        snprintf(
                            status_nuntius, sizeof(status_nuntius),
                            "■ INSCRIPTIO FINITA: %d tabulae",
                            inscr_tabula
                        );
                        pfr_mp4_fini(inscr_mp4);
                        inscr_mp4 = NULL;
                    }
                    break;
                case 'g': {
                        time_t nunc = time(NULL);
                        struct tm *tm = localtime(&nunc);
                        char via[256];
                        snprintf(
                            via, sizeof(via),
                            "caelae/imago_%04d%02d%02d_%02d%02d%02d.gif",
                            tm->tm_year + 1900, tm->tm_mon + 1,
                            tm->tm_mday, tm->tm_hour,
                            tm->tm_min, tm->tm_sec
                        );
                        pfr_gif_t *g = pfr_gif_initia(via, LATITUDO_IMG, ALTITUDO_IMG, 0, 1);
                        if (g) {
                            uint32_t *inv = (uint32_t *)malloc(
                                (size_t)LATITUDO_IMG * ALTITUDO_IMG * 4
                            );
                            if (inv) {
                                const uint32_t *src = (const uint32_t *)tab.imaginis;
                                for (int iy = 0; iy < ALTITUDO_IMG; iy++)
                                    memcpy(
                                        inv + (size_t)(ALTITUDO_IMG - 1 - iy) * LATITUDO_IMG,
                                        src + (size_t)iy * LATITUDO_IMG,
                                        (size_t)LATITUDO_IMG * 4
                                    );
                                pfr_gif_tabulam_adde(g, inv);
                                free(inv);
                            }
                            pfr_gif_fini(g);
                            snprintf(
                                status_nuntius, sizeof(status_nuntius),
                                "■ IMAGO: %s", via
                            );
                        }
                        break;
                    }
                case 'l':
                    if (!orbita_mp4 && rotatio_activa) {
                        time_t nunc = time(NULL);
                        struct tm *tm = localtime(&nunc);
                        snprintf(
                            orbita_nomen, sizeof(orbita_nomen),
                            "caelae/orbita_%04d%02d%02d_%02d%02d%02d",
                            tm->tm_year + 1900, tm->tm_mon + 1,
                            tm->tm_mday, tm->tm_hour,
                            tm->tm_min, tm->tm_sec
                        );
                        char via_mp4[280];
                        char via_gif[280];
                        snprintf(via_mp4, sizeof(via_mp4), "%s.mp4", orbita_nomen);
                        snprintf(via_gif, sizeof(via_gif), "%s.gif", orbita_nomen);
                        orbita_mp4 = pfr_mp4_initia(
                            via_mp4,
                            LATITUDO_IMG, ALTITUDO_IMG, 30
                        );
                        orbita_gif = pfr_gif_initia(
                            via_gif,
                            LATITUDO_IMG, ALTITUDO_IMG, 3, 2
                        );
                        orbita_tabula       = 0;
                        orbita_angulus_init = angulus_rot;
                        snprintf(
                            status_nuntius, sizeof(status_nuntius),
                            "◎ ORBITA: %s (unus cyclus)",
                            orbita_nomen
                        );
                    }
                    break;
                case 'r':
                    theta     = 0.6;
                    phi       = 0.4;
                    distantia = 3.2;
                    angulus_rot = 0.0;
                    celeritas  = 0.12;
                    axis_index = 2;
                    rotatio_activa = 1;
                    if (
                        radius_maior != HELVEA_RADIUS_MAIOR ||
                        radius_minor != HELVEA_RADIUS_MINOR ||
                        methodus != HELVEA_BORRELLI
                    ) {
                        radius_maior = HELVEA_RADIUS_MAIOR;
                        radius_minor = HELVEA_RADIUS_MINOR;
                        methodus = HELVEA_BORRELLI;
                        superficies_obsoleta = 1;
                    }
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Conspectus restitutus"
                    );
                    break;
                default:
                    break;
                }
            }
        }

        const pfr_u8 *status_clavium = pfr_claves_status(NULL);
        int perturbatum = 0;
        if (status_clavium[PFR_SC_SINISTRUM]) {
            theta -= 1.5 * dt;
            perturbatum = 1;
        }
        if (status_clavium[PFR_SC_DEXTRUM])   {
            theta += 1.5 * dt;
            perturbatum = 1;
        }
        if (status_clavium[PFR_SC_SURSUM])    {
            phi   += 1.0 * dt;
            perturbatum = 1;
        }
        if (status_clavium[PFR_SC_DEORSUM])   {
            phi   -= 1.0 * dt;
            perturbatum = 1;
        }
        if (status_clavium[PFR_SC_W])         {
            distantia -= 2.0 * dt;
            perturbatum = 1;
        }
        if (status_clavium[PFR_SC_S])         {
            distantia += 2.0 * dt;
            perturbatum = 1;
        }
        if (perturbatum && orbita_mp4) {
            pfr_mp4_fini(orbita_mp4);
            pfr_gif_fini(orbita_gif);
            orbita_mp4 = NULL;
            orbita_gif = NULL;
            fprintf(stderr, "  orbita cancellata\n");
        }
        if (distantia < 1.8)
            distantia = 1.8;
        if (distantia > 8.0)
            distantia = 8.0;

        if (rotatio_activa)
            angulus_rot += celeritas * dt;

        if (gradus_mutatus) {
            n_vert         = (size_t)(GRADUS_U(gradus) + 1) * (GRADUS_V(gradus) + 1);
            puncta_orig    = (vec3_t *)realloc(puncta_orig, n_vert * sizeof(vec3_t));
            normae_orig    = (vec3_t *)realloc(normae_orig, n_vert * sizeof(vec3_t));
            puncta_rot     = (vec3_t *)realloc(puncta_rot,  n_vert * sizeof(vec3_t));
            normae_rot     = (vec3_t *)realloc(normae_rot,  n_vert * sizeof(vec3_t));
            gradus_mutatus = 0;
        }

        if (superficies_obsoleta) {
            helvea_superficiem_computare(
                puncta_orig, normae_orig,
                GRADUS_U(gradus), GRADUS_V(gradus),
                radius_maior, radius_minor, methodus
            );
            superficies_obsoleta = 0;
        }

        vertices_rotare(puncta_rot, puncta_orig, n_vert, axis_index, angulus_rot);
        vertices_rotare(normae_rot, normae_orig, n_vert, axis_index, angulus_rot);

        /* camera: positio ex coordinatis toroidalibus (theta, phi).
         * theta = horizontalis, phi = verticalis. ambo libere volvuntur.
         * camera orbitat in plano inclinato per phi. */
        vec3_t pos_cam = vec3(
            distantia * cos(theta),
            distantia * sin(theta),
            0.0
        );
        pos_cam       = rotare_x(pos_cam, phi);
        vec3_t scopus = vec3(0.0, 0.0, 0.0);

        /* sursum: perpendiculare ad orbitam, rotatum per phi */
        vec3_t sursum_cam = rotare_x(vec3(0.0, 0.0, 1.0), phi);
        camera_t cam;
        cam.positio = pos_cam;
        cam.ante    = normalizare(differentia(scopus, pos_cam));
        cam.dextrum = normalizare(productum_vectoriale(cam.ante, sursum_cam));
        cam.sursum  = productum_vectoriale(cam.dextrum, cam.ante);
        cam.focalis = 1.6;

        /* fundum stellarum — toroidaliter volvitur cum (theta, phi) */
        tabulam_purgare(&tab);
        int delta_x = (int)(theta / DUO_PI * campus->latitudo);
        int delta_y = (int)(phi   / DUO_PI * campus->altitudo);
        fundum_implere(
            &tab, campus->pixels,
            campus->latitudo, campus->altitudo,
            delta_x, delta_y
        );

        scaenam_reddere(
            &tab, puncta_rot, normae_rot,
            GRADUS_U(gradus), GRADUS_V(gradus), &cam,
            helvea_illuminare_thema, pixel_bgra
        );

        pfx_applicare(
            &tab, helvea_themata[helvea_index_thematis].pfx,
            helvea_themata[helvea_index_thematis].posteriza_niv
        );

        /* curvatura animatio */
        if (curvatura_activa) {
            /* inspice si generatio finita */
            if (!curv_generatum && curv_pid > 0) {
                int wstat;
                if (waitpid(curv_pid, &wstat, WNOHANG) > 0)
                    curv_generatum = 1;
            }

            curvatura_reddere(&tab, curv_tabula);
            curv_tabula++;

            /* fini curvatura cum animatio et generatio ambae perfectae */
            if (curv_tabula >= CURV_TABULAE && curv_generatum) {
                campus_destruere(campus);
                campus = campus_ex_isonl_reddere(
                    "/tmp/sqrt_caelae.isonl", via_instr
                );
                if (campus) {
                    catalogum_legere("/tmp/sqrt_caelae.isonl");
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "Systema novum: %d sidera", numerus_siderum
                    );
                } else {
                    /* fallback: restitue campus originale */
                    campus = campus_ex_isonl_reddere(via_isonl, via_instr);
                    snprintf(
                        status_nuntius, sizeof(status_nuntius),
                        "ERRATUM: campus reddere non possum"
                    );
                }
                curvatura_activa = 0;
                curv_pid         = 0;
            }
        }

        pfr_texturam_renova(textura, NULL, tab.imaginis, LATITUDO_IMG * 4);
        pfr_purga(pictor);
        pfr_texturam_pinge(pictor, textura, NULL, NULL);

        /* cursor muris — figitur ad sidus proximum */
        {
            int mx, my;
            pfr_muris_positio(&mx, &my);
            if (
                mx >= 0 && mx < LATITUDO_IMG
                && my >= 0 && my < ALTITUDO_IMG
                && !curvatura_activa
                && numerus_siderum > 0
            ) {
                /* converte ad campus coordinatas */
                int dcx = (int)(theta / DUO_PI * campus->latitudo);
                int dcy = (int)(phi   / DUO_PI * campus->altitudo);
                int ccx = (
                    (mx + dcx) % campus->latitudo
                    + campus->latitudo
                ) % campus->latitudo;
                int ccy = (
                    (my + dcy) % campus->altitudo
                    + campus->altitudo
                ) % campus->altitudo;

                int si = inveni_sidus(
                    ccx, ccy,
                    campus->latitudo, campus->altitudo
                );
                if (si >= 0) {
                    /* campus → screen */
                    int sx = (
                        (sidera[si].x - dcx) % campus->latitudo +
                        campus->latitudo
                    ) % campus->latitudo;
                    int sy = (
                        (sidera[si].y - dcy) % campus->altitudo +
                        campus->altitudo
                    ) % campus->altitudo;

                    if (
                        sx >= 0 && sx < LATITUDO_IMG &&
                        sy >= 0 && sy < ALTITUDO_IMG
                    ) {
                        /* crux */
                        pfr_colorem_pone(pictor, 120, 255, 120, 200);
                        for (int i = -10; i <= 10; i++) {
                            if (i >= -2 && i <= 2)
                                continue;
                            pfr_punctum_pinge(pictor, sx + i, sy);
                            pfr_punctum_pinge(pictor, sx, sy + i);
                        }
                        /* circulus */
                        for (int a = 0; a < 40; a++) {
                            double ang = (double)a / 40.0 * DUO_PI;
                            pfr_punctum_pinge(
                                pictor,
                                sx + (int)(14.0 * cos(ang)),
                                sy + (int)(14.0 * sin(ang))
                            );
                        }
                    }
                }
            }
        }

        /* indicium inscriptionis — circulus ruber */
        if (inscr_mp4) {
            pfr_colorem_pone(pictor, 220, 30, 30, 255);
            for (int dy = -5; dy <= 5; dy++)
                for (int dx = -5; dx <= 5; dx++)
                    if (dx * dx + dy * dy <= 25)
                        pfr_punctum_pinge(pictor, 20 + dx, 20 + dy);
        }

        /* indicium orbitae — circulus caeruleus */
        if (orbita_mp4) {
            pfr_colorem_pone(pictor, 30, 120, 220, 255);
            for (int dy = -5; dy <= 5; dy++)
                for (int dx = -5; dx <= 5; dx++)
                    if (dx * dx + dy * dy <= 25)
                        pfr_punctum_pinge(
                            pictor, inscr_mp4 ? 40 : 20,
                            20 + dy + dx * 0
                        );
            /* arcum progressus reddere */
            double prog = angulus_rot - orbita_angulus_init;
            if (prog < 0)
                prog = -prog;
            double frac = prog / DUO_PI;
            if (frac > 1.0)
                frac = 1.0;
            int arc_px = (int)(frac * (LATITUDO_IMG - 40));
            pfr_colorem_pone(pictor, 30, 120, 220, 255);
            pfr_rectum_t bar = {20, ALTITUDO_IMG - 8, arc_px, 4};
            pfr_rectum_imple(pictor, &bar);
        }

        pfr_praesenta(pictor);

        /* tabulam inscribere si inscribimus */
        if (inscr_mp4) {
            pfr_mp4_tabulam_adde(inscr_mp4, (const uint32_t *)tab.imaginis);
            inscr_tabula++;
        }
        if (orbita_mp4) {
            pfr_mp4_tabulam_adde(orbita_mp4, (const uint32_t *)tab.imaginis);
            pfr_gif_tabulam_adde(orbita_gif, (const uint32_t *)tab.imaginis);
            orbita_tabula++;
        }

        /* orbita: fini post unum cyclum (2π) */
        if (orbita_mp4) {
            double progressus = angulus_rot - orbita_angulus_init;
            if (progressus < 0)
                progressus = -progressus;
            if (progressus >= DUO_PI) {
                snprintf(
                    status_nuntius, sizeof(status_nuntius),
                    "◎ ORBITA PERFECTA: %d tabulae",
                    orbita_tabula
                );
                pfr_mp4_fini(orbita_mp4);
                pfr_gif_fini(orbita_gif);
                orbita_mp4 = NULL;
                orbita_gif = NULL;
            }
        }

        /* indicium FPS in terminali */
        tabulae_fps++;
        pfr_u32 tempus_nunc_fps = pfr_tempus();
        pfr_u32 intervallum_fps = tempus_nunc_fps - tempus_fps;
        if (intervallum_fps >= 1000) {
            double fps = tabulae_fps * 1000.0 / intervallum_fps;
            fprintf(
                stderr, "\r\033[K%3.0f fps | %-12s | %-10s s%d g%d | %s | %4.2f rad/s",
                fps,
                helvea_themata[helvea_index_thematis].nomen,
                helvea_nomina_methodorum[methodus],
                helvea_strata,
                gradus,
                nomina_axium[axis_index],
                celeritas
            );
            if (status_nuntius[0]) {
                fprintf(stderr, " | %s", status_nuntius);
                status_nuntius[0] = '\0';
            }
            tabulae_fps = 0;
            tempus_fps  = tempus_nunc_fps;
        }
    }

    fprintf(stderr, "\r\033[K");

    /* inscriptiones finire */
    if (inscr_mp4) {
        fprintf(stderr, "■ INSCRIPTIO FINITA: %d tabulae\n", inscr_tabula);
        pfr_mp4_fini(inscr_mp4);
    }
    if (orbita_mp4) {
        fprintf(stderr, "Orbita cancellata (exit)\n");
        pfr_mp4_fini(orbita_mp4);
        pfr_gif_fini(orbita_gif);
    }

    campus_destruere(campus);
    free(puncta_orig);
    free(normae_orig);
    free(puncta_rot);
    free(normae_rot);
    free(tab.imaginis);
    free(tab.profunditatis);

    pfr_texturam_destrue(textura);
    pfr_pictorem_destrue(pictor);
    pfr_fenestram_destrue(fenestra);
    pfr_fini();

    fprintf(stderr, "Vale.\n");
    return 0;
}
