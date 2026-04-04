/*
 * pictura.c — camera, tabula imaginis, rasterizatio
 *
 * Implementationes genericas reddendi: camera perspectiva,
 * framebuffer cum z-buffer, rasterizatio triangulorum.
 */

#include "pictura.h"

/* ================================================================
 * camera perspectiva
 * ================================================================ */

camera_t cameram_constituere(vec3_t positio, vec3_t scopus)
{
    camera_t cam;
    cam.positio = positio;
    cam.ante    = normalizare(differentia(scopus, positio));

    vec3_t mundi_sursum = vec3(0.0, 0.0, 1.0);
    cam.dextrum = normalizare(productum_vectoriale(cam.ante, mundi_sursum));
    cam.sursum  = productum_vectoriale(cam.dextrum, cam.ante);
    cam.focalis = 1.6;
    return cam;
}

int pictura_proicere(
    const camera_t *cam, vec3_t p,
    double *scr_x, double *scr_y, double *prof,
    int latitudo, int altitudo
) {
    vec3_t d = differentia(p, cam->positio);
    double z = productum_scalare(d, cam->ante);
    if (z < 0.05)
        return 0;

    double f     = cam->focalis / z;
    double scala = altitudo * 0.5;

    *scr_x = latitudo * 0.5 + productum_scalare(d, cam->dextrum) * f * scala;
    *scr_y = altitudo * 0.5 - productum_scalare(d, cam->sursum)  * f * scala;
    *prof  = z;
    return 1;
}

/* ================================================================
 * tabula imaginis
 * ================================================================ */

void tabulam_purgare(tabula_t *t)
{
    size_t n_pix = (size_t)t->latitudo * t->altitudo;

    for (size_t i = 0; i < n_pix; i++)
        t->profunditatis[i] = 1e30;

    if (t->bytes_pixel == 4) {
        for (size_t i = 0; i < n_pix; i++) {
            size_t base = i * 4;
            t->imaginis[base + 0] = 14;   /* B */
            t->imaginis[base + 1] = 8;    /* G */
            t->imaginis[base + 2] = 8;    /* R */
            t->imaginis[base + 3] = 255;  /* A */
        }
    } else {
        for (size_t i = 0; i < n_pix; i++) {
            size_t base = i * 3;
            t->imaginis[base + 0] = 8;
            t->imaginis[base + 1] = 8;
            t->imaginis[base + 2] = 14;
        }
    }
}

void pixel_rgb(
    tabula_t *t, int x, int y,
    double prof, color_t c
) {
    if (x < 0 || x >= t->latitudo || y < 0 || y >= t->altitudo)
        return;
    int idx = y * t->latitudo + x;
    if (prof >= t->profunditatis[idx])
        return;
    t->profunditatis[idx] = prof;

    int base = idx * 3;
    t->imaginis[base + 0] = gamma_corrigere(c.r);
    t->imaginis[base + 1] = gamma_corrigere(c.g);
    t->imaginis[base + 2] = gamma_corrigere(c.b);
}

void pixel_bgra(
    tabula_t *t, int x, int y,
    double prof, color_t c
) {
    if (x < 0 || x >= t->latitudo || y < 0 || y >= t->altitudo)
        return;
    int idx = y * t->latitudo + x;
    if (prof >= t->profunditatis[idx])
        return;
    t->profunditatis[idx] = prof;

    int base = idx * 4;
    t->imaginis[base + 0] = gamma_corrigere(c.b);
    t->imaginis[base + 1] = gamma_corrigere(c.g);
    t->imaginis[base + 2] = gamma_corrigere(c.r);
    t->imaginis[base + 3] = 255;
}

/* ================================================================
 * fundum bitmap toroidale
 * ================================================================ */

void fundum_implere(
    tabula_t *t,
    const unsigned char *bitmap,
    int bm_latitudo, int bm_altitudo,
    int delta_x, int delta_y
) {
    /* delta in [0, bm) normalizare */
    delta_x = ((delta_x % bm_latitudo) + bm_latitudo) % bm_latitudo;
    delta_y = ((delta_y % bm_altitudo) + bm_altitudo) % bm_altitudo;

    for (int y = 0; y < t->altitudo; y++) {
        int by = (y + delta_y) % bm_altitudo;
        for (int x = 0; x < t->latitudo; x++) {
            int bx = (x + delta_x) % bm_latitudo;
            int bm_idx = (by * bm_latitudo + bx) * 3;
            unsigned char r = bitmap[bm_idx + 0];
            unsigned char g = bitmap[bm_idx + 1];
            unsigned char b = bitmap[bm_idx + 2];

            size_t idx = (size_t)y * t->latitudo + x;
            if (t->bytes_pixel == 4) {
                size_t base = idx * 4;
                t->imaginis[base + 0] = b;
                t->imaginis[base + 1] = g;
                t->imaginis[base + 2] = r;
                t->imaginis[base + 3] = 255;
            } else {
                size_t base = idx * 3;
                t->imaginis[base + 0] = r;
                t->imaginis[base + 1] = g;
                t->imaginis[base + 2] = b;
            }
        }
    }
}

/* ================================================================
 * rasterizatio triangulorum
 * ================================================================ */

void triangulum_reddere(
    tabula_t *t,
    double sx0, double sy0, double sz0, vec3_t p0, vec3_t n0,
    double sx1, double sy1, double sz1, vec3_t p1, vec3_t n1,
    double sx2, double sy2, double sz2, vec3_t p2, vec3_t n2,
    vec3_t oculus,
    illuminare_fn illum_fn,
    pixel_fn pixel_fn
) {
    double area = (sx1 - sx0) * (sy2 - sy0) - (sx2 - sx0) * (sy1 - sy0);
    if (fabs(area) < 0.5)
        return;
    /* backface culling */
    if (area > 0.0)
        return;
    double inv_area = 1.0 / area;

    int min_x = (int)floor(fmin(fmin(sx0, sx1), sx2));
    int max_x = (int)ceil (fmax(fmax(sx0, sx1), sx2));
    int min_y = (int)floor(fmin(fmin(sy0, sy1), sy2));
    int max_y = (int)ceil (fmax(fmax(sy0, sy1), sy2));

    if (min_x < 0)
        min_x = 0;
    if (max_x >= t->latitudo)
        max_x = t->latitudo - 1;
    if (min_y < 0)
        min_y = 0;
    if (max_y >= t->altitudo)
        max_y = t->altitudo - 1;

    for (int y = min_y; y <= max_y; y++) {
        double py = y + 0.5;
        for (int x = min_x; x <= max_x; x++) {
            double px = x + 0.5;

            double w0 = ((sx1 - px) * (sy2 - py) - (sx2 - px) * (sy1 - py))
            * inv_area;
            double w1 = ((sx2 - px) * (sy0 - py) - (sx0 - px) * (sy2 - py))
            * inv_area;
            double w2 = 1.0 - w0 - w1;

            if (w0 < -0.001 || w1 < -0.001 || w2 < -0.001)
                continue;

            double z = w0 * sz0 + w1 * sz1 + w2 * sz2;

            vec3_t punct = summa(
                summa(
                    multiplicare(p0, w0),
                    multiplicare(p1, w1)
                ),
                multiplicare(p2, w2)
            );
            vec3_t norm  = normalizare(
                summa(
                    summa(
                        multiplicare(n0, w0),
                        multiplicare(n1, w1)
                    ),
                    multiplicare(n2, w2)
                )
            );

            color_t c = illum_fn(punct, norm, oculus);
            pixel_fn(t, x, y, z, c);
        }
    }
}

/* ================================================================
 * scaenam reddere
 * ================================================================ */

void scaenam_reddere(
    tabula_t *t,
    const vec3_t *puncta, const vec3_t *normae,
    int gradus_u, int gradus_v,
    const camera_t *cam,
    illuminare_fn illum_fn,
    pixel_fn pixel_fn
) {
    for (int i = 0; i < gradus_u; i++) {
        for (int j = 0; j < gradus_v; j++) {
            size_t vi[4] = {
                (size_t)i       * (gradus_v + 1) + j,
                (size_t)(i + 1) * (gradus_v + 1) + j,
                (size_t)(i + 1) * (gradus_v + 1) + (j + 1),
                (size_t)i       * (gradus_v + 1) + (j + 1)
            };

            double sx[4], sy[4], sz[4];
            int omnes_visibiles = 1;
            for (int k = 0; k < 4; k++) {
                if (
                    !pictura_proicere(
                        cam, puncta[vi[k]],
                        &sx[k], &sy[k], &sz[k],
                        t->latitudo, t->altitudo
                    )
                ) {
                    omnes_visibiles = 0;
                    break;
                }
            }
            if (!omnes_visibiles)
                continue;

            triangulum_reddere(
                t,
                sx[0], sy[0], sz[0], puncta[vi[0]], normae[vi[0]],
                sx[1], sy[1], sz[1], puncta[vi[1]], normae[vi[1]],
                sx[2], sy[2], sz[2], puncta[vi[2]], normae[vi[2]],
                cam->positio, illum_fn, pixel_fn
            );

            triangulum_reddere(
                t,
                sx[0], sy[0], sz[0], puncta[vi[0]], normae[vi[0]],
                sx[2], sy[2], sz[2], puncta[vi[2]], normae[vi[2]],
                sx[3], sy[3], sz[3], puncta[vi[3]], normae[vi[3]],
                cam->positio, illum_fn, pixel_fn
            );
        }
    }
}
