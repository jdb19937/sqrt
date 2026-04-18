/*
 * campus.c — campus stellarum, compositio et redditio
 */

#include "campus.h"
#include "caela.h"
#include "vectores.h"
#include "tessera.h"
#include "instrumentum.h"
#include "aspectus.h"
#include "perceptus.h"
#include <ison/ison.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEN SIDUS_FENESTRA
#define SEMI (FEN / 2)

/* ================================================================
 * campus stellarum
 * ================================================================ */

campus_t *campus_creare(int latitudo, int altitudo)
{
    campus_t *c = (campus_t *)malloc(sizeof(campus_t));
    c->latitudo = latitudo;
    c->altitudo = altitudo;
    c->pixels   = (unsigned char *)calloc((size_t)latitudo * altitudo * 3, 1);
    return c;
}

void campus_destruere(campus_t *c)
{
    if (c) {
        free(c->pixels);
        free(c);
    }
}

void campus_pixel_scribere(
    campus_t *c, int x, int y,
    unsigned char r, unsigned char g, unsigned char b
) {
    x       = ((x % c->latitudo) + c->latitudo) % c->latitudo;
    y       = ((y % c->altitudo) + c->altitudo) % c->altitudo;
    int idx = (y * c->latitudo + x) * 3;

    int cr = c->pixels[idx + 0] + r;
    if (cr > 255)
        cr = 255;
    int cg = c->pixels[idx + 1] + g;
    if (cg > 255)
        cg = 255;
    int cb = c->pixels[idx + 2] + b;
    if (cb > 255)
        cb = 255;
    c->pixels[idx + 0] = (unsigned char)cr;
    c->pixels[idx + 1] = (unsigned char)cg;
    c->pixels[idx + 2] = (unsigned char)cb;
}

void sidus_in_campum(
    campus_t *c, int cx, int cy,
    const unsigned char *fenestra
) {
    for (int dy = 0; dy < FEN; dy++) {
        for (int dx = 0; dx < FEN; dx++) {
            int fi = (dy * FEN + dx) * 4;
            unsigned char a = fenestra[fi + 3];
            if (a == 0)
                continue;

            double af = a / 255.0;
            int px    = cx + dx - SEMI;
            int py    = cy + dy - SEMI;

            px = ((px % c->latitudo) + c->latitudo) % c->latitudo;
            py = ((py % c->altitudo) + c->altitudo) % c->altitudo;

            int ci = (py * c->latitudo + px) * 3;
            int r  = (int)(c->pixels[ci + 0] + fenestra[fi + 0] * af);
            int g  = (int)(c->pixels[ci + 1] + fenestra[fi + 1] * af);
            int b  = (int)(c->pixels[ci + 2] + fenestra[fi + 2] * af);
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            c->pixels[ci + 0] = (unsigned char)r;
            c->pixels[ci + 1] = (unsigned char)g;
            c->pixels[ci + 2] = (unsigned char)b;
        }
    }
}

int campus_regio_vacua(const campus_t *c, int cx, int cy, int radius)
{
    for (int d = 1; d <= radius; d++) {
        int puncta[8][2] = {
            {cx + d, cy}, {cx - d, cy}, {cx, cy + d}, {cx, cy - d},
            {cx + d, cy + d}, {cx - d, cy - d},
            {cx + d, cy - d}, {cx - d, cy + d}
        };
        for (int k = 0; k < 8; k++) {
            int px  = ((puncta[k][0] % c->latitudo) + c->latitudo) % c->latitudo;
            int py  = ((puncta[k][1] % c->altitudo) + c->altitudo) % c->altitudo;
            int idx = (py * c->latitudo + px) * 3;
            int lum = c->pixels[idx] + c->pixels[idx + 1] + c->pixels[idx + 2];
            if (lum > 30)
                return 0;
        }
    }
    return 1;
}

void planeta_in_campum(
    campus_t *c, int cx, int cy,
    const unsigned char *fenestra, double scala
) {
    int dim = (int)(PLANETA_FENESTRA * scala);
    if (dim < 2)
        return;
    int semi = dim / 2;

    for (int dy = 0; dy < dim; dy++) {
        for (int dx = 0; dx < dim; dx++) {
            int sx = (int)(dx / scala);
            int sy = (int)(dy / scala);
            if (sx >= PLANETA_FENESTRA)
                sx = PLANETA_FENESTRA - 1;
            if (sy >= PLANETA_FENESTRA)
                sy = PLANETA_FENESTRA - 1;

            int fi = (sy * PLANETA_FENESTRA + sx) * 4;
            unsigned char a = fenestra[fi + 3];
            if (a == 0)
                continue;

            double af = a / 255.0;
            int px    = cx + dx - semi;
            int py    = cy + dy - semi;

            px = ((px % c->latitudo) + c->latitudo) % c->latitudo;
            py = ((py % c->altitudo) + c->altitudo) % c->altitudo;

            int ci = (py * c->latitudo + px) * 3;
            c->pixels[ci + 0] = (unsigned char)(fenestra[fi + 0] * af + c->pixels[ci + 0] * (1.0 - af));
            c->pixels[ci + 1] = (unsigned char)(fenestra[fi + 1] * af + c->pixels[ci + 1] * (1.0 - af));
            c->pixels[ci + 2] = (unsigned char)(fenestra[fi + 2] * af + c->pixels[ci + 2] * (1.0 - af));
        }
    }
}

/* ================================================================
 * Via Lactea
 * ================================================================ */

typedef struct {
    double galaxia_glow, galaxia_rift, inclinatio;
    int    galaxia_nebulae;
} via_lactea_ctx_t;

static void viam_lacteam_reddere(campus_t *c, const via_lactea_ctx_t *m)
{
    if (m->galaxia_glow < 0.001)
        return;

    double inc       = m->inclinatio;
    unsigned int sem = 7919;

    for (int y = 0; y < c->altitudo; y++) {
        for (int x = 0; x < c->latitudo; x++) {
            double tx       = (double)x / c->latitudo;
            double ty       = (double)y / c->altitudo;
            double y_fascia = 0.5 + (inc / 3.0) * sin(DUO_PI * tx);
            double d        = ty - y_fascia;
            if (d > 0.5)
                d -= 1.0;
            if (d < -0.5)
                d += 1.0;

            double band   = exp(-d * d / (0.06 * 0.06));
            double dx_nuc = tx - 0.65;
            if (dx_nuc > 0.5)
                dx_nuc -= 1.0;
            if (dx_nuc < -0.5)
                dx_nuc += 1.0;
            double nucleus = exp(-(dx_nuc * dx_nuc * 4.0 + d * d) / (0.04 * 0.04));
            band += nucleus * 0.6;

            sem ^= sem << 13;
            sem ^= sem >> 17;
            sem ^= sem << 5;
            double r1 = (double)(sem & 0xFF) / 255.0;
            sem ^= sem << 13;
            sem ^= sem >> 17;
            sem ^= sem << 5;
            double r2     = (double)(sem & 0xFF) / 255.0;
            double rumore = r1 * 0.6 + r2 * 0.4;

            double f = band * (0.04 + rumore * 0.03) * m->galaxia_glow;
            if (f < 0.003)
                continue;

            campus_pixel_scribere(
                c, x, y,
                (unsigned char)(f * 220), (unsigned char)(f * 200), (unsigned char)(f * 170)
            );
        }
    }

    if (m->galaxia_rift > 0.001) {
        sem = 4217;
        for (int y = 0; y < c->altitudo; y++) {
            for (int x = 0; x < c->latitudo; x++) {
                double tx       = (double)x / c->latitudo;
                double ty       = (double)y / c->altitudo;
                double y_fascia = 0.5 + (inc / 3.0) * sin(DUO_PI * tx);
                double d        = ty - y_fascia;
                if (d > 0.5)
                    d -= 1.0;
                if (d < -0.5)
                    d += 1.0;
                if (fabs(d) > 0.08)
                    continue;
                double along = tx;

                sem ^= sem << 13;
                sem ^= sem >> 17;
                sem ^= sem << 5;
                double rumore = (double)(sem & 0xFF) / 255.0;

                double rift = 0.0;
                rift += 0.5 * sin(along * 25.0 + 1.3) * exp(-d * d / (0.015 * 0.015));
                rift += 0.3 * sin(along * 40.0 + 2.7) * exp(-d * d / (0.010 * 0.010));
                rift += 0.2 * rumore * exp(-d * d / (0.02 * 0.02));
                if (rift < 0.15)
                    continue;
                double obscura = rift * 0.5 * m->galaxia_rift;
                if (obscura > 0.8)
                    obscura = 0.8;

                int px  = ((x % c->latitudo) + c->latitudo) % c->latitudo;
                int py  = ((y % c->altitudo) + c->altitudo) % c->altitudo;
                int idx = (py * c->latitudo + px) * 3;
                c->pixels[idx + 0] = (unsigned char)(c->pixels[idx + 0] * (1.0 - obscura));
                c->pixels[idx + 1] = (unsigned char)(c->pixels[idx + 1] * (1.0 - obscura));
                c->pixels[idx + 2] = (unsigned char)(c->pixels[idx + 2] * (1.0 - obscura));
            }
        }
    }

    sem = 1571;
    for (int i = 0; i < m->galaxia_nebulae; i++) {
        sem ^= sem << 13;
        sem ^= sem >> 17;
        sem ^= sem << 5;
        double nx = (double)(sem & 0xFFFFF) / (double)0x100000;
        sem ^= sem << 13;
        sem ^= sem >> 17;
        sem ^= sem << 5;
        double ny    = (double)(sem & 0xFFFFF) / (double)0x100000;
        double gx    = nx * c->latitudo;
        double y_neb = 0.5 + (inc / 3.0) * sin(DUO_PI * nx);
        double gy    = (y_neb + (ny - 0.5) * 0.08) * c->altitudo;
        sem ^= sem << 13;
        sem ^= sem >> 17;
        sem ^= sem << 5;
        double radius = 8 + (double)(sem & 0x1F);
        unsigned char nr, ng, nb;
        if (i % 3 == 0)      {
            nr = 40;
            ng = 15;
            nb = 15;
        }else if (i % 3 == 1) {
            nr = 12;
            ng = 30;
            nb = 20;
        }else                  {
            nr = 25;
            ng = 20;
            nb = 30;
        }
        for (int dy = -(int)radius; dy <= (int)radius; dy++) {
            for (int dx = -(int)radius; dx <= (int)radius; dx++) {
                double d2 = (double)(dx * dx + dy * dy);
                double f  = exp(-d2 / (radius * radius * 0.4));
                if (f < 0.05)
                    continue;
                campus_pixel_scribere(
                    c, (int)gx + dx, (int)gy + dy,
                    (unsigned char)(nr * f), (unsigned char)(ng * f), (unsigned char)(nb * f)
                );
            }
        }
    }
}

/* ================================================================
 * campus redditio — quattuor phases
 * ================================================================ */

void campus_sidera_reddere(
    campus_t *c, const caela_t *caela, const instrumentum_t *inst
) {
    int i_spic         = inst->spiculae;
    double i_spic_long = inst->spiculae_long;
    double i_spic_ang  = inst->spiculae_ang;
    double i_halo_r    = inst->halo_radius;
    double i_halo_v    = inst->halo_vis;

    for (int i = 0; i < caela->numerus_siderum; i++) {
        const sidus_caeli_t *sc = &caela->sidera[i];
        double mag = sc->sidus.ubi.sequentia.pro.magnitudo;

        instrumentum_t si = {.saturatio = 1.0};
        if (mag < 1.5 && i_spic > 0) {
            double bright    = 1.5 - mag;
            si.spiculae      = i_spic;
            si.spiculae_long = i_spic_long * bright + 2.0;
            si.spiculae_ang  = i_spic_ang;
            si.halo_radius   = i_halo_r * bright + 1.0;
            si.halo_vis      = i_halo_v * bright;
        } else if (mag < 2.5 && i_halo_r > 0.01) {
            si.halo_radius = i_halo_r * (2.5 - mag) * 0.5;
            si.halo_vis    = i_halo_v * (2.5 - mag) * 0.5;
        }

        unsigned char fen[FEN * FEN * 4];
        sidus_reddere(fen, &sc->sidus, &si);
        sidus_in_campum(c, sc->x, sc->y, fen);
    }
}

void campus_viam_lacteam_reddere(campus_t *c, const caela_t *caela)
{
    via_lactea_ctx_t vl = {
        .galaxia_glow    = caela->galaxia_glow,
        .galaxia_rift    = caela->galaxia_rift,
        .galaxia_nebulae = caela->galaxia_nebulae,
        .inclinatio      = caela->inclinatio_galaxiae
    };
    viam_lacteam_reddere(c, &vl);
}

/* inspicit an bbox planetae viewport tangat (toroidale) */
static int planeta_in_conspectu(
    const planeta_caeli_t *pc, int lat, int alt,
    int vx, int vy, int vw, int vh
) {
    if (vw <= 0 || vh <= 0)
        return 1; /* nullum viewport = omnia */
    int dim  = (int)(PLANETA_FENESTRA * pc->scala);
    int semi = dim / 2;

    /* distantia toroidalis minima inter centrum planetae et centrum viewport */
    int cx  = (int)pc->x, cy = (int)pc->y;
    int vcx = vx + vw / 2, vcy = vy + vh / 2;

    int dx = abs(cx - vcx);
    if (dx > lat / 2)
        dx = lat - dx;
    int dy = abs(cy - vcy);
    if (dy > alt / 2)
        dy = alt - dy;

    return dx < (semi + vw / 2 + 1) && dy < (semi + vh / 2 + 1);
}

void campus_planetas_reddere(campus_t *c, const caela_t *caela)
{
    campus_planetas_reddere_in_conspectu(c, caela, 0, 0, c->latitudo, c->altitudo);
}

typedef struct {
    int    index;
    double z;
} planeta_z_ordo_t;

static int cmp_planeta_z(const void *a, const void *b)
{
    double za = ((const planeta_z_ordo_t *)a)->z;
    double zb = ((const planeta_z_ordo_t *)b)->z;
    return (za < zb) ? -1 : (za > zb) ? 1 : 0;
}

void campus_planetas_reddere_in_conspectu(
    campus_t *c, const caela_t *caela,
    int vx, int vy, int vw, int vh
) {
    int n = caela->numerus_planetarum;
    if (n <= 0)
        return;

    /* ordina per z (longinquiores primo) */
    planeta_z_ordo_t *ordo = malloc((size_t)n * sizeof(planeta_z_ordo_t));
    if (!ordo)
        return;

    for (int i = 0; i < n; i++) {
        ordo[i].index = i;
        ordo[i].z     = caela->planetae[i].z;
    }
    qsort(ordo, (size_t)n, sizeof(planeta_z_ordo_t), cmp_planeta_z);

    size_t fen_sz = (size_t)PLANETA_FENESTRA * PLANETA_FENESTRA * 4;

    for (int j = 0; j < n; j++) {
        planeta_caeli_t *pc = &caela->planetae[ordo[j].index];
        if (!planeta_in_conspectu(pc, c->latitudo, c->altitudo, vx, vy, vw, vh))
            continue;

        /* cache corpus planetae (sine illuminatione) */
        if (!pc->fenestra_cacata) {
            pc->fenestra_cacata = (unsigned char *)calloc(fen_sz, 1);
            if (pc->fenestra_cacata)
                planeta_reddere(pc->fenestra_cacata, &pc->planeta);
        }
        if (!pc->fenestra_cacata)
            continue;

        /* copia cacatam, applica illuminationem (non soles/nebulae) et perceptum, compone */
        unsigned char *pfen = (unsigned char *)malloc(fen_sz);
        if (pfen) {
            memcpy(pfen, pc->fenestra_cacata, fen_sz);
            if (pc->planeta.qui != PLANETA_SOL && pc->planeta.qui != PLANETA_NEBULA)
                planeta_illuminationem_applicare(pfen, pc->planeta.ubi.saxosum.pro.radius, &pc->perceptus);
            planeta_perceptum_applicare(pfen, &pc->perceptus);
            planeta_in_campum(c, (int)pc->x, (int)pc->y, pfen, pc->scala);
            free(pfen);
        }
    }
    free(ordo);
}

/* ================================================================
 * campus_ex_caela — reddit campum ex caela et instrumento
 * ================================================================ */

campus_t *campus_ex_caela(
    const caela_t *caela,
    const instrumentum_t *inst
) {
    campus_t *c = campus_creare(caela->latitudo, caela->altitudo);
    if (!c)
        return NULL;

    campus_sidera_reddere(c, caela, inst);
    campus_viam_lacteam_reddere(c, caela);
    campus_planetas_reddere(c, caela);
    campus_post_processare(c, inst);
    return c;
}
