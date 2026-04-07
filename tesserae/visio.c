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

#define FEN VISIO_FENESTRA
#define SEMI (FEN / 2)

/* ================================================================
 * torus — per codicem helvea existentem
 * ================================================================ */

static void torus_reddere(unsigned char *fenestra, const visio_torus_t *t)
{
    int gu   = t->gradus_u > 0 ? t->gradus_u : 800;
    int gv   = t->gradus_v > 0 ? t->gradus_v : 400;
    double R = t->radius_maior > 0 ? t->radius_maior : HELVEA_RADIUS_MAIOR;
    double r = t->radius_minor > 0 ? t->radius_minor : HELVEA_RADIUS_MINOR;

    /* strata corrugationis */
    helvea_strata = t->strata;

    /* thema */
    int th = t->thema;
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

    helvea_superficiem_computare(puncta, normae, gu, gv, R, r, t->methodus);

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
    double dist = t->distantia > 0 ? t->distantia : 3.5;
    double elev = t->elevatio;
    double azim = t->azimuthus;
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

/* ================================================================
 * navis ardens — burning ship fractal
 *
 * z_{n+1} = (|Re(z_n)| + i|Im(z_n)|)^2 + c
 * ================================================================ */

static void navis_reddere(unsigned char *fenestra, const visio_navis_t *n)
{
    double cx  = n->centrum_re;
    double cy  = n->centrum_im;
    double amp = n->amplitudo > 0 ? n->amplitudo : 0.02;
    int    max = n->iterationes > 0 ? n->iterationes : 1000;
    double cyc = n->color_cyclus > 0 ? n->color_cyclus : 8.0;
    double pha = n->color_phase;
    double sat = n->saturatio > 0 ? n->saturatio : 1.0;

    double x0  = cx - amp * 0.5;
    double y0  = cy - amp * 0.5;
    double pas = amp / (double)FEN;

    for (int py = 0; py < FEN; py++) {
        double ci = y0 + (double)py * pas;
        for (int px = 0; px < FEN; px++) {
            double cr = x0 + (double)px * pas;

            double zr = 0.0, zi = 0.0;
            int iter  = 0;
            while (iter < max) {
                double azr = fabs(zr);
                double azi = fabs(zi);
                double zr2 = azr * azr - azi * azi + cr;
                double zi2 = 2.0 * azr * azi + ci;
                zr         = zr2;
                zi         = zi2;
                double r2  = zr * zr + zi * zi;
                if (r2 > 256.0 || r2 != r2)
                    break;
                iter++;
            }

            int di = (py * FEN + px) * 4;
            if (iter >= max) {
                /* intra fractale — nigrum */
                fenestra[di + 0] = 0;
                fenestra[di + 1] = 0;
                fenestra[di + 2] = 0;
                fenestra[di + 3] = 255;
            } else {
                /* smooth coloring */
                double r2 = zr * zr + zi * zi;
                if (r2 < 1.0)
                    r2 = 1.0;
                double lr = log2(r2);
                if (lr < 1.0)
                    lr = 1.0;
                double mu = (double)iter - log2(lr) + 4.0;
                double t  = mu / cyc + pha;

                /* palette bichroma: caeruleum profundum ↔ ignis aureus */
                double s = fmod(t, 1.0);
                if (s < 0.0)
                    s += 1.0;
                double r, g, b;
                if (s < 0.15) {
                    /* nigrum → caeruleum profundum */
                    double q = s / 0.15;
                    r        = 0.02 * q;
                    g        = 0.04 * q;
                    b        = 0.18 * q;
                } else if (s < 0.4) {
                    /* caeruleum profundum → caeruleum lucidum */
                    double q = (s - 0.15) / 0.25;
                    r        = 0.02 + 0.08 * q;
                    g        = 0.04 + 0.30 * q;
                    b        = 0.18 + 0.55 * q;
                } else if (s < 0.55) {
                    /* caeruleum → aurantium (transitus) */
                    double q = (s - 0.4) / 0.15;
                    r        = 0.10 + 0.70 * q;
                    g        = 0.34 - 0.04 * q;
                    b        = 0.73 - 0.68 * q;
                } else if (s < 0.8) {
                    /* aurantium → flavum lucidum */
                    double q = (s - 0.55) / 0.25;
                    r        = 0.80 + 0.20 * q;
                    g        = 0.30 + 0.50 * q;
                    b        = 0.05 + 0.05 * q;
                } else {
                    /* flavum → album → nigrum (reditus) */
                    double q = (s - 0.8) / 0.2;
                    r        = 1.00 - 0.85 * q;
                    g        = 0.80 - 0.70 * q;
                    b        = 0.10 - 0.05 * q;
                }
                r *= sat;
                g *= sat;
                b *= sat;

                fenestra[di + 0] = gamma_corrigere(r);
                fenestra[di + 1] = gamma_corrigere(g);
                fenestra[di + 2] = gamma_corrigere(b);
                fenestra[di + 3] = 255;
            }
        }
    }
}

/* ================================================================
 * zeppelinus — corpus ellipsoidale cum gondola
 *
 * Ray-sphere intersection analyticum in spatio transformato.
 * ================================================================ */

/* ray-ellipsoid intersection: ellipsoid (x/a)^2 + (y/b)^2 + (z/c)^2 = 1 */
static int ellipsoid_ictus(
    vec3_t orig, vec3_t dir,
    double a, double b, double c,
    double *t_res, vec3_t *norm_res
) {
    /* scala in spatium unitarium */
    double ox = orig.x / a, oy = orig.y / b, oz = orig.z / c;
    double dx = dir.x / a,  dy = dir.y / b,  dz = dir.z / c;

    double A = dx * dx + dy * dy + dz * dz;
    double B = 2.0 * (ox * dx + oy * dy + oz * dz);
    double C = ox * ox + oy * oy + oz * oz - 1.0;

    double disc = B * B - 4.0 * A * C;
    if (disc < 0.0)
        return 0;

    double sq = sqrt(disc);
    double t1 = (-B - sq) / (2.0 * A);
    double t2 = (-B + sq) / (2.0 * A);

    double t = t1 > 0.001 ? t1 : (t2 > 0.001 ? t2 : -1.0);
    if (t < 0.0)
        return 0;

    *t_res   = t;
    vec3_t p = summa(orig, multiplicare(dir, t));
    *norm_res = normalizare(
        vec3(
            2.0 * p.x / (a * a),
            2.0 * p.y / (b * b),
            2.0 * p.z / (c * c)
        )
    );
    return 1;
}

/* ray-box intersection (gondola) */
static int arca_ictus(
    vec3_t orig, vec3_t dir,
    vec3_t bmin, vec3_t bmax,
    double *t_res, vec3_t *norm_res
) {
    double tmin     = -1e30, tmax = 1e30;
    vec3_t norm_min = vec3(0, 0, 0);

    double inv, t1, t2;
    /* x */
    if (fabs(dir.x) > 1e-12) {
        inv = 1.0 / dir.x;
        t1  = (bmin.x - orig.x) * inv;
        t2  = (bmax.x - orig.x) * inv;
        if (t1 > t2) {
            double tmp = t1;
            t1         = t2;
            t2         = tmp;
        }
        if (t1 > tmin) {
            tmin     = t1;
            norm_min = vec3(dir.x > 0 ? -1 : 1, 0, 0);
        }
        if (t2 < tmax)
            tmax = t2;
    }
    /* y */
    if (fabs(dir.y) > 1e-12) {
        inv = 1.0 / dir.y;
        t1  = (bmin.y - orig.y) * inv;
        t2  = (bmax.y - orig.y) * inv;
        if (t1 > t2) {
            double tmp = t1;
            t1         = t2;
            t2         = tmp;
        }
        if (t1 > tmin) {
            tmin     = t1;
            norm_min = vec3(0, dir.y > 0 ? -1 : 1, 0);
        }
        if (t2 < tmax)
            tmax = t2;
    }
    /* z */
    if (fabs(dir.z) > 1e-12) {
        inv = 1.0 / dir.z;
        t1  = (bmin.z - orig.z) * inv;
        t2  = (bmax.z - orig.z) * inv;
        if (t1 > t2) {
            double tmp = t1;
            t1         = t2;
            t2         = tmp;
        }
        if (t1 > tmin) {
            tmin     = t1;
            norm_min = vec3(0, 0, dir.z > 0 ? -1 : 1);
        }
        if (t2 < tmax)
            tmax = t2;
    }

    if (tmin > tmax || tmax < 0.001)
        return 0;
    double t = tmin > 0.001 ? tmin : tmax;
    if (t < 0.001)
        return 0;

    *t_res    = t;
    *norm_res = norm_min;
    return 1;
}

static void zeppelinus_reddere(unsigned char *fenestra, const visio_zeppelinus_t *z)
{
    double ratio = z->ratio > 0 ? z->ratio : 4.0;
    double incl  = z->inclinatio;
    double azim  = z->azimuthus;

    color_t c_inv = z->involucrum;
    if (c_inv.r == 0 && c_inv.g == 0 && c_inv.b == 0)
        c_inv = (color_t){0.60, 0.62, 0.65, 1.0};

    color_t c_gon = z->gondola;
    if (c_gon.r == 0 && c_gon.g == 0 && c_gon.b == 0)
        c_gon = (color_t){0.25, 0.18, 0.12, 1.0};

    double pin = z->pinnae > 0 ? z->pinnae : 0.4;

    /* lux */
    double la = z->lux_angulus;
    double le = z->lux_elevatio != 0 ? z->lux_elevatio : 0.6;
    vec3_t lux = normalizare(
        vec3(
            cos(le) * cos(la),
            cos(le) * sin(la),
            sin(le)
        )
    );

    /* dimensiones */
    double a  = ratio * 0.5; /* semi-axis longus (x) */
    double bc = 0.5;         /* semi-axes (y, z) */

    /* gondola sub corpore */
    double g_lat = a * 0.35;
    double g_alt = bc * 0.25;
    double g_dep = bc * 0.3;
    vec3_t gmin  = vec3(-g_lat, -g_dep, -bc - g_alt * 2.0);
    vec3_t gmax  = vec3( g_lat,  g_dep, -bc);

    /* pinnae (tres, posteriores) */
    double p_off = a * 0.78;

    /* camera — semper centrata, FOV ex ratio ut fenestram impleat */
    double elev     = z->elevatio != 0 ? z->elevatio : 0.15;
    double cam_dist = a * 2.5;
    vec3_t cam_pos = vec3(
        -cam_dist * cos(elev),
        0.0,
        cam_dist * sin(elev)
    );
    vec3_t scopus   = vec3(0, 0, 0);
    vec3_t cam_ante = normalizare(differentia(scopus, cam_pos));
    vec3_t cam_dext = normalizare(productum_vectoriale(cam_ante, vec3(0, 0, 1)));
    vec3_t cam_surs = productum_vectoriale(cam_dext, cam_ante);

    /* FOV automatica: a/cam_dist ut corpus fenestram impleat */
    double fov = (a * 1.15) / cam_dist;

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double u = ((double)px / FEN - 0.5) * 2.0;
            double v = ((double)py / FEN - 0.5) * 2.0;

            vec3_t dir = normalizare(
                summa(
                    summa(
                        cam_ante,
                        multiplicare(cam_dext, u * fov)
                    ),
                    multiplicare(cam_surs, -v * fov)
                )
            );

            /* rotare radium per inclinationem et azimuthum (inversam) */
            vec3_t ro = differentia(cam_pos, scopus);
            ro = rotare_z(ro, -azim);
            ro = rotare_y(ro, -incl);
            vec3_t orig_r = summa(scopus, ro);

            dir = rotare_z(dir, -azim);
            dir = rotare_y(dir, -incl);

            double t_best = 1e30;
            vec3_t n_best = vec3(0, 0, 0);
            color_t c_best = {0, 0, 0, 0};
            int ictum = 0;

            /* corpus principale (ellipsoid) */
            double t_e;
            vec3_t n_e;
            if (ellipsoid_ictus(orig_r, dir, a, bc, bc, &t_e, &n_e)) {
                if (t_e < t_best) {
                    t_best = t_e;
                    n_best = n_e;
                    c_best = c_inv;
                    ictum  = 1;
                }
            }

            /* gondola (arca) */
            double t_g;
            vec3_t n_g;
            if (arca_ictus(orig_r, dir, gmin, gmax, &t_g, &n_g)) {
                if (t_g < t_best) {
                    t_best = t_g;
                    n_best = n_g;
                    c_best = c_gon;
                    ictum  = 1;
                }
            }

            /* pinnae — tres ellipsoidae planae ad caudam */
            for (int fi = 0; fi < 3; fi++) {
                double ang  = (double)fi * DUO_PI / 3.0;
                double fy   = sin(ang) * pin * bc * 1.5;
                double fz   = cos(ang) * pin * bc * 1.5;
                vec3_t fo   = vec3(p_off, fy, fz);
                vec3_t fdir = differentia(orig_r, fo);

                double t_f;
                vec3_t n_f;
                if (
                    ellipsoid_ictus(
                        fdir, dir,
                        a * 0.2, bc * pin * 0.8, bc * pin * 0.05,
                        &t_f, &n_f
                    )
                ) {
                    if (t_f + 0.0 < t_best) {
                        /* recalcula t in spatio originali */
                        t_best = t_f;
                        n_best = n_f;
                        c_best = c_inv;
                        ictum  = 1;
                    }
                }
            }

            int di = (py * FEN + px) * 4;
            if (!ictum) {
                /* caelum — gradiens simplex */
                double sky_t     = 0.5 + v * 0.5;
                double sr        = 0.35 + 0.50 * sky_t;
                double sg        = 0.55 + 0.35 * sky_t;
                double sb        = 0.80 + 0.15 * sky_t;
                fenestra[di + 0] = gamma_corrigere(sr);
                fenestra[di + 1] = gamma_corrigere(sg);
                fenestra[di + 2] = gamma_corrigere(sb);
                fenestra[di + 3] = 255;
            } else {
                /* Phong */
                double ndl = productum_scalare(n_best, lux);
                if (ndl < 0.0)
                    ndl = 0.0;

                vec3_t h    =
                    normalizare(summa(lux, normalizare(differentia(cam_pos, summa(orig_r, multiplicare(dir, t_best))))));
                double spec = productum_scalare(n_best, h);
                if (spec < 0.0)
                    spec = 0.0;
                spec = pow(spec, 40.0);

                double diff = 0.15 + 0.70 * ndl;
                double r    = c_best.r * diff + spec * 0.3;
                double g    = c_best.g * diff + spec * 0.3;
                double b    = c_best.b * diff + spec * 0.3;

                fenestra[di + 0] = gamma_corrigere(r);
                fenestra[di + 1] = gamma_corrigere(g);
                fenestra[di + 2] = gamma_corrigere(b);
                fenestra[di + 3] = 255;
            }
        }
    }
}

/* ================================================================
 * dispatch
 * ================================================================ */

void visio_reddere(unsigned char *fenestra, const visio_t *visio)
{
    switch (visio->p.genus) {
    case VISIO_TORUS:
        torus_reddere(fenestra, &visio->g.torus);
        break;
    case VISIO_NAVIS:
        navis_reddere(fenestra, &visio->g.navis);
        break;
    case VISIO_ZEPPELINUS:
        zeppelinus_reddere(fenestra, &visio->g.zeppelinus);
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
        visio_torus_t *t = (visio_torus_t *)calloc(1, sizeof(visio_torus_t));
        if (!t)
            return NULL;

        t->basis.genus = VISIO_TORUS;
        t->basis.semen = (unsigned)ison_pares_n(pp, n, "semen", 42);

        const char *m = ison_pares_s(pp, n, "methodus");
        if (strcmp(m, "borrelli_t") == 0)
            t->methodus = HELVEA_BORRELLI_T;
        else if (strcmp(m, "planus") == 0)
            t->methodus = HELVEA_PLANUS;
        else
            t->methodus = HELVEA_BORRELLI;

        t->thema        = (int)ison_pares_n(pp, n, "thema", 0);
        t->strata       = (int)ison_pares_n(pp, n, "strata", 0);
        t->radius_maior = ison_pares_f(pp, n, "radius_maior", HELVEA_RADIUS_MAIOR);
        t->radius_minor = ison_pares_f(pp, n, "radius_minor", HELVEA_RADIUS_MINOR);
        t->gradus_u     = (int)ison_pares_n(pp, n, "gradus_u", 800);
        t->gradus_v     = (int)ison_pares_n(pp, n, "gradus_v", 400);
        t->distantia    = ison_pares_f(pp, n, "distantia", 3.5);
        t->elevatio     = ison_pares_f(pp, n, "elevatio", 0.55);
        t->azimuthus    = ison_pares_f(pp, n, "azimuthus", -0.65);

        return (visio_t *)t;
    }

    if (strcmp(g, "navis") == 0) {
        visio_navis_t *nv = (visio_navis_t *)calloc(1, sizeof(visio_navis_t));
        if (!nv)
            return NULL;

        nv->basis.genus = VISIO_NAVIS;
        nv->basis.semen = (unsigned)ison_pares_n(pp, n, "semen", 42);

        nv->centrum_re   = ison_pares_f(pp, n, "centrum_re", -1.7557);
        nv->centrum_im   = ison_pares_f(pp, n, "centrum_im", -0.0175);
        nv->amplitudo    = ison_pares_f(pp, n, "amplitudo", 0.08);
        nv->iterationes  = (int)ison_pares_n(pp, n, "iterationes", 1000);
        nv->color_cyclus = ison_pares_f(pp, n, "color_cyclus", 8.0);
        nv->color_phase  = ison_pares_f(pp, n, "color_phase", 0.0);
        nv->saturatio    = ison_pares_f(pp, n, "saturatio", 1.0);

        return (visio_t *)nv;
    }

    if (strcmp(g, "zeppelinus") == 0) {
        visio_zeppelinus_t *z = (visio_zeppelinus_t *)calloc(
            1, sizeof(visio_zeppelinus_t)
        );
        if (!z)
            return NULL;

        z->basis.genus = VISIO_ZEPPELINUS;
        z->basis.semen = (unsigned)ison_pares_n(pp, n, "semen", 42);

        z->ratio        = ison_pares_f(pp, n, "ratio", 4.0);
        z->inclinatio   = ison_pares_f(pp, n, "inclinatio", 0.0);
        z->azimuthus    = ison_pares_f(pp, n, "azimuthus", 0.0);
        z->lux_angulus  = ison_pares_f(pp, n, "lux_angulus", 0.8);
        z->lux_elevatio = ison_pares_f(pp, n, "lux_elevatio", 0.6);
        z->involucrum   = color_ex_ison(
            pp, n, "involucrum_r", "involucrum_g", "involucrum_b",
            0.60, 0.62, 0.65
        );
        z->gondola = color_ex_ison(
            pp, n, "gondola_r", "gondola_g", "gondola_b",
            0.25, 0.18, 0.12
        );
        z->elevatio  = ison_pares_f(pp, n, "elevatio", 0.15);
        z->fenestrae = ison_pares_f(pp, n, "fenestrae", 6.0);
        z->pinnae    = ison_pares_f(pp, n, "pinnae", 0.4);

        return (visio_t *)z;
    }

    return NULL;
}
