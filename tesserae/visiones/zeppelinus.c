#include "../visio_communia.h"
/* ================================================================
 * zeppelinus — corpus ellipsoidale cum gondola
 *
 * Ray-sphere intersection analyticum in spatio transformato.
 * ================================================================ */

/* ray-ellipsoid intersection: ellipsoid (x/a)^2 + (y/b)^2 + (z/c)^2 = 1 */
int ellipsoid_ictus(
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
int arca_ictus(
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

void zeppelinus_reddere(unsigned char *fenestra, const zeppelinus_t *z)
{
    double ratio = z->res.ratio > 0 ? z->res.ratio : 4.0;
    double incl  = z->res.inclinatio;
    double azim  = z->res.azimuthus;

    color_t c_inv = z->res.involucrum;
    if (c_inv.r == 0 && c_inv.g == 0 && c_inv.b == 0)
        c_inv = (color_t){0.60, 0.62, 0.65, 1.0};

    color_t c_gon = z->res.gondola;
    if (c_gon.r == 0 && c_gon.g == 0 && c_gon.b == 0)
        c_gon = (color_t){0.25, 0.18, 0.12, 1.0};

    double pin = z->res.pinnae > 0 ? z->res.pinnae : 0.4;

    /* lux */
    double la = z->res.lux_angulus;
    double le = z->res.lux_elevatio != 0 ? z->res.lux_elevatio : 0.6;
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
    double elev     = z->res.elevatio != 0 ? z->res.elevatio : 0.15;
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

void zeppelinus_ex_ison(zeppelinus_t *v, const char *ison)
{
    v->pro.semen = (unsigned)ison_da_n(ison, "visiuncula.semen", 42);
    v->res.ratio        = ison_da_f(ison, "zeppelinulus.ratio", 4.0);
    v->res.inclinatio   = ison_da_f(ison, "zeppelinulus.inclinatio", 0.0);
    v->res.azimuthus    = ison_da_f(ison, "zeppelinulus.azimuthus", 0.0);
    v->res.lux_angulus  = ison_da_f(ison, "zeppelinulus.lux_angulus", 0.8);
    v->res.lux_elevatio = ison_da_f(ison, "zeppelinulus.lux_elevatio", 0.6);
    v->res.involucrum   = (color_t){
        ison_da_f(ison, "zeppelinulus.involucrum.r", 0.60),
        ison_da_f(ison, "zeppelinulus.involucrum.g", 0.62),
        ison_da_f(ison, "zeppelinulus.involucrum.b", 0.65),
        1.0
    };
    v->res.gondola = (color_t){
        ison_da_f(ison, "zeppelinulus.gondola.r", 0.25),
        ison_da_f(ison, "zeppelinulus.gondola.g", 0.18),
        ison_da_f(ison, "zeppelinulus.gondola.b", 0.12),
        1.0
    };
    v->res.elevatio  = ison_da_f(ison, "zeppelinulus.elevatio", 0.15);
    v->res.fenestrae = ison_da_f(ison, "zeppelinulus.fenestrae", 6.0);
    v->res.pinnae    = ison_da_f(ison, "zeppelinulus.pinnae", 0.4);
}

void zeppelinus_in_ison(FILE *f, const zeppelinus_t *s)
{
    fprintf(f, "{\"visiuncula\": {\"semen\": %u}", s->pro.semen);
    fprintf(f, ", \"zeppelinulus\": {\"ratio\": %.6f, \"inclinatio\": %.6f, \"azimuthus\": %.6f, \"lux_angulus\": %.6f, \"lux_elevatio\": %.6f, \"involucrum\": {\"r\": %.6f, \"g\": %.6f, \"b\": %.6f}, \"gondola\": {\"r\": %.6f, \"g\": %.6f, \"b\": %.6f}, \"elevatio\": %.6f, \"fenestrae\": %.6f, \"pinnae\": %.6f}}",
        s->res.ratio, s->res.inclinatio, s->res.azimuthus,
        s->res.lux_angulus, s->res.lux_elevatio,
        s->res.involucrum.r, s->res.involucrum.g, s->res.involucrum.b,
        s->res.gondola.r, s->res.gondola.g, s->res.gondola.b,
        s->res.elevatio, s->res.fenestrae, s->res.pinnae);
}
