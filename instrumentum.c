/*
 * instrumentum.c — instrumenta optica, effectus atmosphaerici
 *
 * Post-processatio campi stellarum: visio, scintillatio, refractio,
 * caeli lumen, florescentia, acuitas, aberratio, distorsio,
 * saturatio, vignetta, fenestra.
 */

#include "instrumentum.h"
#include "sidus.h"
#include "campus.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * generans numerum pseudo-aleatorium (copia ex astra.c)
 * ================================================================ */

static unsigned int semen_g = 1;

/* ================================================================
 * effectus post-processationis
 * ================================================================ */

static void pp_visio(astra_campus_t *c, double radius,
                     unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    int r = (int)(radius * 3.0);
    if (r < 1) r = 1;
    if (r > 30) r = 30;

    /* nucleus Gaussianus normalizatus */
    double *kern = (double *)malloc((size_t)(r * 2 + 1) * sizeof(double));
    double sum_k = 0;
    for (int i = -r; i <= r; i++) {
        double t = (double)i / radius;
        kern[i + r] = exp(-t * t * 0.5);
        sum_k += kern[i + r];
    }
    for (int i = 0; i <= 2 * r; i++) kern[i] /= sum_k;

    /* transitus horizontalis — toroidale */
    memcpy(copia, px, (size_t)L * A * 3);
    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            double sr = 0, sg = 0, sb = 0;
            for (int k = -r; k <= r; k++) {
                int sx = ((x + k) % L + L) % L;
                int si = (y * L + sx) * 3;
                double w = kern[k + r];
                sr += copia[si + 0] * w;
                sg += copia[si + 1] * w;
                sb += copia[si + 2] * w;
            }
            int di = (y * L + x) * 3;
            px[di + 0] = (unsigned char)(sr + 0.5);
            px[di + 1] = (unsigned char)(sg + 0.5);
            px[di + 2] = (unsigned char)(sb + 0.5);
        }
    }

    /* transitus verticalis — toroidale */
    memcpy(copia, px, (size_t)L * A * 3);
    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            double sr = 0, sg = 0, sb = 0;
            for (int k = -r; k <= r; k++) {
                int sy = ((y + k) % A + A) % A;
                int si = (sy * L + x) * 3;
                double w = kern[k + r];
                sr += copia[si + 0] * w;
                sg += copia[si + 1] * w;
                sb += copia[si + 2] * w;
            }
            int di = (y * L + x) * 3;
            px[di + 0] = (unsigned char)(sr + 0.5);
            px[di + 1] = (unsigned char)(sg + 0.5);
            px[di + 2] = (unsigned char)(sb + 0.5);
        }
    }
    free(kern);
}

/* scintillatio — variatio pseudo-aleatoriae intensitatis per pixel.
 * Tatarskii (1961): σ²_I ∝ C_n² ∫ Φ_n(κ) dκ.
 * simplificamus ad rumorem multiplicativum. */
static void pp_scintillatio(astra_campus_t *c, double amplitudo)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    unsigned int sem = semen_g ^ 31337;
    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            int idx = (y * L + x) * 3;
            int lum = px[idx] + px[idx + 1] + px[idx + 2];
            if (lum < 15) continue;
            sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
            double noise = ((double)(sem & 0xFFFF) / 32768.0) - 1.0;
            double factor = 1.0 + amplitudo * noise;
            if (factor < 0.0) factor = 0.0;
            for (int ch = 0; ch < 3; ch++) {
                int v = (int)(px[idx + ch] * factor + 0.5);
                if (v > 255) v = 255;
                px[idx + ch] = (unsigned char)v;
            }
        }
    }
}

/* refractio — dislocatio spatiosa localis atmosphaerica.
 * quisque pixel movetur in directione pseudo-aleatoriae per
 * distantiam parvam. coherentia spatiosa per hash de (x/s, y/s)
 * ubi s = scala cellulae turbulentiae (Kolmogorov inner scale).
 * Fried (1966): σ_θ ≈ 0.42 λ/r_0. toroidale involutum. */
static void pp_refractio(astra_campus_t *c, double amplitudo,
                         unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    memcpy(copia, px, (size_t)L * A * 3);

    /* scala cellulae — pixeles per cellula turbulentiae.
     * cellulae maiores dant dislocationes cohaerentes (blobby),
     * cellulae minores dant rumorem granularem. */
    double scala = 8.0;
    double inv_s = 1.0 / scala;

    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            /* hash spatiosa cohaerens — interpolata inter cellulas.
             * cellula (gx, gy) dat dislocatio fixa per semen. */
            double fx = x * inv_s, fy = y * inv_s;
            int gx0 = (int)fx, gy0 = (int)fy;
            double tx = fx - gx0, ty = fy - gy0;
            /* smooth interpolatio (Perlin) */
            tx = tx * tx * (3.0 - 2.0 * tx);
            ty = ty * ty * (3.0 - 2.0 * ty);

            /* 4 anguli cellulae — dislocationes per hash */
            double ddx = 0, ddy = 0;
            for (int cy = 0; cy <= 1; cy++) {
                for (int cx = 0; cx <= 1; cx++) {
                    unsigned int h = (unsigned int)(
                        (gx0 + cx) * 73856093u ^ (gy0 + cy) * 19349663u
                        ^ semen_g);
                    h ^= h >> 13; h ^= h << 7; h ^= h >> 17;
                    double hx = ((double)(h & 0xFFFF) / 32768.0) - 1.0;
                    h ^= h << 13; h ^= h >> 7; h ^= h << 17;
                    double hy = ((double)(h & 0xFFFF) / 32768.0) - 1.0;
                    double wx = cx ? tx : (1.0 - tx);
                    double wy = cy ? ty : (1.0 - ty);
                    double w = wx * wy;
                    ddx += hx * w;
                    ddy += hy * w;
                }
            }

            int sx = ((int)(x + ddx * amplitudo + 0.5) % L + L) % L;
            int sy = ((int)(y + ddy * amplitudo + 0.5) % A + A) % A;
            int di = (y * L + x) * 3;
            int si = (sy * L + sx) * 3;
            px[di + 0] = copia[si + 0];
            px[di + 1] = copia[si + 1];
            px[di + 2] = copia[si + 2];
        }
    }
}

/* caeli lumen — glow additivum uniformem (pollutio luminosa).
 * color calidus aurantiacus ex sodii lampade (Garstang 1986)
 * cum componente caerulea ex dispersione Rayleigh. */
static void pp_caeli_lumen(astra_campus_t *c, double intensitas)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    /* NaD 589 nm: aurantiacum calidum */
    int gr = (int)(intensitas * 45);
    int gg = (int)(intensitas * 35);
    int gb = (int)(intensitas * 22);
    for (int i = 0; i < L * A; i++) {
        int idx = i * 3;
        int r = px[idx + 0] + gr; if (r > 255) r = 255;
        int g = px[idx + 1] + gg; if (g > 255) g = 255;
        int b = px[idx + 2] + gb; if (b > 255) b = 255;
        px[idx + 0] = (unsigned char)r;
        px[idx + 1] = (unsigned char)g;
        px[idx + 2] = (unsigned char)b;
    }
}

/* florescentia — bloom: regiones lucidae halo diffusum emittunt.
 * extractio supra limen, blur Gaussianus, additio.
 * Janesick (2001): CCD blooming ex saturatione pixelorum. */
static void pp_florescentia(astra_campus_t *c, double radius,
                            unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    size_t n = (size_t)L * A * 3;

    /* extractio pixelorum lucidorum in copia */
    for (size_t i = 0; i < (size_t)L * A; i++) {
        int idx = (int)i * 3;
        int lum = px[idx] + px[idx + 1] + px[idx + 2];
        if (lum > 180) {
            double f = (double)(lum - 180) / (765.0 - 180.0);
            copia[idx + 0] = (unsigned char)(px[idx + 0] * f);
            copia[idx + 1] = (unsigned char)(px[idx + 1] * f);
            copia[idx + 2] = (unsigned char)(px[idx + 2] * f);
        } else {
            copia[idx + 0] = 0;
            copia[idx + 1] = 0;
            copia[idx + 2] = 0;
        }
    }

    /* blur copia — simplificatus: iterationes box blur (Central Limit).
     * tres iterationes box blur approximant Gaussianam (Wells 1986). */
    unsigned char *temp = (unsigned char *)malloc(n);
    int box_r = (int)(radius + 0.5);
    if (box_r < 1) box_r = 1;
    if (box_r > 30) box_r = 30;

    for (int iter = 0; iter < 3; iter++) {
        /* horizontale — toroidale */
        memcpy(temp, copia, n);
        for (int y = 0; y < A; y++) {
            int sr = 0, sg = 0, sb = 0;
            int w = 2 * box_r + 1;
            /* initia summa */
            for (int k = -box_r; k <= box_r; k++) {
                int sx = ((k) % L + L) % L;
                int si = (y * L + sx) * 3;
                sr += temp[si]; sg += temp[si + 1]; sb += temp[si + 2];
            }
            for (int x = 0; x < L; x++) {
                int di = (y * L + x) * 3;
                copia[di + 0] = (unsigned char)(sr / w);
                copia[di + 1] = (unsigned char)(sg / w);
                copia[di + 2] = (unsigned char)(sb / w);
                /* glide: adde dextrum, remove sinistrum */
                int add_x = ((x + box_r + 1) % L + L) % L;
                int rem_x = ((x - box_r) % L + L) % L;
                int ai = (y * L + add_x) * 3;
                int ri = (y * L + rem_x) * 3;
                sr += temp[ai] - temp[ri];
                sg += temp[ai + 1] - temp[ri + 1];
                sb += temp[ai + 2] - temp[ri + 2];
            }
        }
        /* verticale — toroidale */
        memcpy(temp, copia, n);
        for (int x = 0; x < L; x++) {
            int sr = 0, sg = 0, sb = 0;
            int w = 2 * box_r + 1;
            for (int k = -box_r; k <= box_r; k++) {
                int sy = ((k) % A + A) % A;
                int si = (sy * L + x) * 3;
                sr += temp[si]; sg += temp[si + 1]; sb += temp[si + 2];
            }
            for (int y = 0; y < A; y++) {
                int di = (y * L + x) * 3;
                copia[di + 0] = (unsigned char)(sr / w);
                copia[di + 1] = (unsigned char)(sg / w);
                copia[di + 2] = (unsigned char)(sb / w);
                int add_y = ((y + box_r + 1) % A + A) % A;
                int rem_y = ((y - box_r) % A + A) % A;
                int ai = (add_y * L + x) * 3;
                int ri = (rem_y * L + x) * 3;
                sr += temp[ai] - temp[ri];
                sg += temp[ai + 1] - temp[ri + 1];
                sb += temp[ai + 2] - temp[ri + 2];
            }
        }
    }
    free(temp);

    /* additio bloom ad imaginem */
    for (size_t i = 0; i < (size_t)L * A; i++) {
        int idx = (int)i * 3;
        for (int ch = 0; ch < 3; ch++) {
            int v = px[idx + ch] + copia[idx + ch];
            if (v > 255) v = 255;
            px[idx + ch] = (unsigned char)v;
        }
    }
}

/* acuitas — unsharp masking (Malin 1977).
 * I_acuta = I + α(I - I_blur). α > 0 acuit, α < 0 lenit. */
static void pp_acuitas(astra_campus_t *c, double factor,
                       unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    memcpy(copia, px, (size_t)L * A * 3);

    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            int idx = (y * L + x) * 3;
            /* 3×3 media — toroidale */
            for (int ch = 0; ch < 3; ch++) {
                int sum = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int sx = ((x + dx) % L + L) % L;
                        int sy = ((y + dy) % A + A) % A;
                        sum += copia[(sy * L + sx) * 3 + ch];
                    }
                }
                double avg = sum / 9.0;
                double orig = copia[idx + ch];
                double v = orig + factor * (orig - avg);
                if (v < 0) v = 0; if (v > 255) v = 255;
                px[idx + ch] = (unsigned char)(v + 0.5);
            }
        }
    }
}

/* aberratio chromatica — separatio canalium R et B radialis.
 * Cauchy: n(λ) = A + B/λ². Toroidale involutum. */
static void pp_aberratio(astra_campus_t *c, double aberratio,
                         unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    memcpy(copia, px, (size_t)L * A * 3);

    double cx = L * 0.5, cy = A * 0.5;
    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            double dx = x - cx, dy = y - cy;
            double dist = sqrt(dx * dx + dy * dy);
            if (dist < 1.0) continue;
            double nx = dx / dist, ny = dy / dist;
            double shift_r = aberratio * dist / (L * 0.5);
            int rx = ((int)(x + nx * shift_r) % L + L) % L;
            int ry = ((int)(y + ny * shift_r) % A + A) % A;
            int bx = ((int)(x - nx * shift_r) % L + L) % L;
            int by = ((int)(y - ny * shift_r) % A + A) % A;
            int idx = (y * L + x) * 3;
            px[idx + 0] = copia[(ry * L + rx) * 3 + 0];
            px[idx + 2] = copia[(by * L + bx) * 3 + 2];
        }
    }
}

/* distorsio — lens gravitationalis / deformatio barilis toroidalis.
 * Distorsio barilis: r' = r(1 + k·r²) (Brown 1966).
 * In contextu cosmologico: gravitational lensing per massam
 * punctualem dat deflectionem α = 4GM/(c²b) (Einstein 1915,
 * "Die Feldgleichungen der Gravitation"). Pro lente debili
 * (weak lensing, Bartelmann & Schneider 2001, "Weak gravitational
 * lensing"), deformatio imaginis per tensorem shear γ describitur:
 *   convergentia κ = Σ/Σ_cr (densitas superficialis / critica)
 *   shear γ = γ₁ + iγ₂ (deformatio elliptica)
 * hic simplificamus ad distorsionem radialem symmetricam.
 * omnes coordinatae toroidaliter involutae. */
static void pp_distorsio(astra_campus_t *c, double coeff,
                         unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    memcpy(copia, px, (size_t)L * A * 3);

    double cx = L * 0.5, cy = A * 0.5;
    double r_max = sqrt(cx * cx + cy * cy);
    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            double dx = x - cx, dy = y - cy;
            double r = sqrt(dx * dx + dy * dy) / r_max;
            double r_dist = r * (1.0 + coeff * r * r);
            /* coordinatae fontis — toroidale */
            int sx = ((int)(cx + dx * r_dist / (r + 1e-10) + 0.5) % L + L) % L;
            int sy = ((int)(cy + dy * r_dist / (r + 1e-10) + 0.5) % A + A) % A;
            int di = (y * L + x) * 3;
            int si = (sy * L + sx) * 3;
            px[di + 0] = copia[si + 0];
            px[di + 1] = copia[si + 1];
            px[di + 2] = copia[si + 2];
        }
    }
}

/* saturatio coloris — CIE 1931 luminantia preservata. */
static void pp_saturatio(astra_campus_t *c, double saturatio)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    for (int i = 0; i < L * A; i++) {
        int idx = i * 3;
        double r = px[idx + 0] / 255.0;
        double g = px[idx + 1] / 255.0;
        double b = px[idx + 2] / 255.0;
        double lum = 0.2126 * r + 0.7152 * g + 0.0722 * b;
        r = lum + (r - lum) * saturatio;
        g = lum + (g - lum) * saturatio;
        b = lum + (b - lum) * saturatio;
        if (r < 0) r = 0; if (r > 1) r = 1;
        if (g < 0) g = 0; if (g > 1) g = 1;
        if (b < 0) b = 0; if (b > 1) b = 1;
        px[idx + 0] = (unsigned char)(r * 255);
        px[idx + 1] = (unsigned char)(g * 255);
        px[idx + 2] = (unsigned char)(b * 255);
    }
}

/* vignetta — obscuratio marginalis cos⁴(θ) (Slater 1959).
 * distantia toroidalis a centro imaginis. */
static void pp_vignetta(astra_campus_t *c, double fortitudo)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    double cx = L * 0.5, cy = A * 0.5;
    double r_max = sqrt(cx * cx + cy * cy);
    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            double dx = x - cx, dy = y - cy;
            double d = sqrt(dx * dx + dy * dy) / r_max;
            double f = 1.0 - fortitudo * d * d;
            if (f < 0) f = 0;
            int idx = (y * L + x) * 3;
            px[idx + 0] = (unsigned char)(px[idx + 0] * f);
            px[idx + 1] = (unsigned char)(px[idx + 1] * f);
            px[idx + 2] = (unsigned char)(px[idx + 2] * f);
        }
    }
}

/* fenestra — lens toroidalis: distorsio ex topologia tori.
 *
 * Torus planus est quadratum cum marginibus oppositis identificatis.
 * Duo puncta singularia in proiectione existunt:
 *   (1) centrum quadrati — punctum maximae regularitatis,
 *       hic lens convexa levis (inflatio) applicatur.
 *   (2) anguli quadrati — omnes quattuor idem punctum in toro,
 *       ubi curvatura Gaussiana (in immersione Nash-Kuiper)
 *       concentratur. hic lens convergens applicatur.
 *
 * Physice motivatum: in immersione C1 tori plani (Borrelli+ 2012,
 * "Flat tori in three-dimensional space and convex integration"),
 * corrugationes altae frequentiae curvatura localem enormiter
 * augent — praesertim circa puncta ubi directiones corrugationum
 * concurrunt. effectus opticus: lux per superficiem corrugatum
 * transiens refringitur (quasi vitrum corrugatum).
 *
 * in topologia: metrica plana in toro per chartam (R²/Z²) dat
 * trivialem — sed immersio in R³ necessarie curvatura extrinsecam
 * introducit. Weyl (1916, "Über die Bestimmung einer geschlossenen
 * konvexen Fläche durch ihr Linienelement") demonstravit connexionem
 * inter curvatura intrinsecam et extrinsecam. Nash (1956) et
 * Kuiper (1955) existentiam immersionis probaverunt sed
 * constructio explicita solum per convexam integrationem
 * (Conti, De Lellis & Székelyhidi 2012) inventa.
 *
 * fenestra = 0: nulla. >0: fortitudo effectus. */
static void pp_fenestra(astra_campus_t *c, double fortitudo,
                        unsigned char *copia)
{
    int L = c->latitudo, A = c->altitudo;
    unsigned char *px = c->pixels;
    memcpy(copia, px, (size_t)L * A * 3);

    double cx = L * 0.5, cy = A * 0.5;
    /* semi normalizata — ellipsis pro rectangulo */
    double norm_x = cx, norm_y = cy;

    for (int y = 0; y < A; y++) {
        for (int x = 0; x < L; x++) {
            /* coordinatae normalizatae [-1, 1] */
            double dx = (x - cx) / norm_x;
            double dy = (y - cy) / norm_y;

            /* lens 1: inflatio centralis (bubbling).
             * Gaussiana lata — maxima in centro, evanescens ad margines */
            double r_c2 = dx * dx + dy * dy;
            double d_centro = -fortitudo * 0.15 * exp(-r_c2 * 1.5);

            /* lens 2: contractio ad angulos (omnes 4 = idem punctum in toro).
             * distantia toroidalis ab angulo proximo */
            double ax = fabs(dx), ay = fabs(dy);
            double r_a2 = (1.0 - ax) * (1.0 - ax) + (1.0 - ay) * (1.0 - ay);
            double d_anguli = fortitudo * 0.12 * exp(-r_a2 * 2.5);

            /* lens 3: levis ondulatio ad margines laterales (ubi
             * margines oppositi identificantur in toro) */
            double edge_x = exp(-ax * ax * 8.0) * (1.0 - ay * ay);
            double edge_y = exp(-ay * ay * 8.0) * (1.0 - ax * ax);
            double d_marginis = fortitudo * 0.04 * (edge_x + edge_y);

            double dt = d_centro + d_anguli + d_marginis;

            /* coordinatae fontis — toroidale */
            double sx_f = x + dx * dt * norm_x;
            double sy_f = y + dy * dt * norm_y;
            int sx = ((int)(sx_f + 0.5) % L + L) % L;
            int sy = ((int)(sy_f + 0.5) % A + A) % A;

            int di = (y * L + x) * 3;
            int si = (sy * L + sx) * 3;
            px[di + 0] = copia[si + 0];
            px[di + 1] = copia[si + 1];
            px[di + 2] = copia[si + 2];
        }
    }
}

/* pipeline post-processationis — omnes effectus in ordine physico */
void isonl_post_processare(astra_campus_t *c,
                            const astra_instrumentum_t *inst)
{
    size_t n = (size_t)c->latitudo * c->altitudo * 3;

    /* alveus communis pro effectibus qui copiam requirunt */
    int opus_copia = (inst->visio > 0.1 || inst->refractio > 0.1
        || inst->florescentia > 0.1
        || (inst->acuitas > 0.01 || inst->acuitas < -0.01)
        || inst->aberratio > 0.1
        || (inst->distorsio > 0.001 || inst->distorsio < -0.001)
        || inst->fenestra > 0.001);
    unsigned char *copia = opus_copia ? (unsigned char *)malloc(n) : NULL;

    if (inst->visio > 0.1)
        pp_visio(c, inst->visio, copia);
    if (inst->scintillatio > 0.001)
        pp_scintillatio(c, inst->scintillatio);
    if (inst->refractio > 0.1)
        pp_refractio(c, inst->refractio, copia);
    if (inst->caeli_lumen > 0.001)
        pp_caeli_lumen(c, inst->caeli_lumen);
    if (inst->florescentia > 0.1)
        pp_florescentia(c, inst->florescentia, copia);
    if (inst->acuitas > 0.01 || inst->acuitas < -0.01)
        pp_acuitas(c, inst->acuitas, copia);
    if (inst->aberratio > 0.1)
        pp_aberratio(c, inst->aberratio, copia);
    if (inst->distorsio > 0.001 || inst->distorsio < -0.001)
        pp_distorsio(c, inst->distorsio, copia);
    if (inst->saturatio > 1.001 || inst->saturatio < 0.999)
        pp_saturatio(c, inst->saturatio);
    if (inst->vignetta > 0.001)
        pp_vignetta(c, inst->vignetta);
    if (inst->fenestra > 0.001)
        pp_fenestra(c, inst->fenestra, copia);

    free(copia);
}

/* ================================================================
 * animatio — effectus dynamici per tabulam
 *
 * Atmosphaera non est statica: cellulae turbulentiae moventur
 * per ventum (Taylor "frozen turbulence" hypothesis, 1938):
 * v_wind ~5-15 m/s ad altitudinem turbulentem ~10 km.
 * Tempus coherentiae (Greenwood 1977): τ_0 = 0.314 r_0/v_wind
 * typice 2-20 ms. Ergo in quaque tabula (33 ms ad 30 fps),
 * pattern refractiones et scintillationis mutatur complete.
 *
 * Translatio campi similis rotationem Terrae: 15°/h = 0.25°/min.
 * In campo 360°, una revolutio = 24h = 86400 tabulae ad 1/s.
 * ================================================================ */

astra_campus_t *astra_tabulam_dynamicam(const astra_campus_t *basis,
                                         const astra_instrumentum_t *inst,
                                         int tabula,
                                         int scala,
                                         double dx, double dy)
{
    int L = basis->latitudo, A = basis->altitudo;
    int oL = L / scala, oA = A / scala;
    if (oL < 1) oL = 1;
    if (oA < 1) oA = 1;

    astra_campus_t *out = astra_campum_creare(oL, oA);
    int off_x = (int)(dx + 0.5);
    int off_y = (int)(dy + 0.5);

    for (int y = 0; y < oA; y++) {
        for (int x = 0; x < oL; x++) {
            /* supersampla per scala×scala et media */
            int sr = 0, sg = 0, sb = 0;
            for (int sy = 0; sy < scala; sy++) {
                for (int sx = 0; sx < scala; sx++) {
                    int bx = ((x * scala + sx + off_x) % L + L) % L;
                    int by = ((y * scala + sy + off_y) % A + A) % A;
                    int bi = (by * L + bx) * 3;
                    sr += basis->pixels[bi + 0];
                    sg += basis->pixels[bi + 1];
                    sb += basis->pixels[bi + 2];
                }
            }
            int n = scala * scala;
            int oi = (y * oL + x) * 3;
            out->pixels[oi + 0] = (unsigned char)(sr / n);
            out->pixels[oi + 1] = (unsigned char)(sg / n);
            out->pixels[oi + 2] = (unsigned char)(sb / n);
        }
    }

    /* effectus dynamici — semen mutatur per tabulam */
    astra_instrumentum_t dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.saturatio = 1.0;

    /* solum effectus temporales: scintillatio et refractio.
     * semen diversum per tabulam dat pattern novum. */
    if (inst->scintillatio > 0.001)
        dyn.scintillatio = inst->scintillatio;
    if (inst->refractio > 0.1)
        dyn.refractio = inst->refractio / scala;

    /* semen temporale — mutamus semen_g globalem ante effectus */
    semen_g = (unsigned int)(tabula * 73856093u + 42);

    isonl_post_processare(out, &dyn);

    return out;
}
