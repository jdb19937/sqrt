/*
 * sidus_communia.h — functiones communes pro redditione siderum
 *
 * Includitur a sidera ut separatim compilari possint.
 */

#ifndef SIDUS_COMMUNIA_H
#define SIDUS_COMMUNIA_H

#include "sidus.h"
#include "../instrumentum.h"
#include <ison/ison.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PI_GRAECUM 3.14159265358979323846
#define DUO_PI     (2.0 * PI_GRAECUM)

#define FEN SIDUS_FENESTRA
#define SEMI (FEN / 2)

/* ================================================================
 * generans numerum pseudo-aleatorium
 * ================================================================ */

extern unsigned int sidus_semen_g;

static inline unsigned int sidus_alea(void)
{
    sidus_semen_g ^= sidus_semen_g << 13;
    sidus_semen_g ^= sidus_semen_g >> 17;
    sidus_semen_g ^= sidus_semen_g << 5;
    return sidus_semen_g;
}

/* [0, 1) */
static inline double sidus_alea_f(void)
{
    return (double)(sidus_alea() & 0xFFFFFF) / (double)0x1000000;
}

/* Gaussiana per Box-Muller */
static inline double sidus_alea_gauss(void)
{
    double u1 = sidus_alea_f() + 1e-30;
    double u2 = sidus_alea_f();
    return sqrt(-2.0 * log(u1)) * cos(DUO_PI * u2);
}

/* ================================================================
 * sidus reddere — functiones auxiliares
 * ================================================================ */

/* punctum Gaussianum in fenestra scribere */
static inline void fen_punctum(
    unsigned char *fen, double cx, double cy,
    double radius, color_t col, double intensitas
) {
    int r0 = (int)(cy - radius * 3) - 1;
    int r1 = (int)(cy + radius * 3) + 2;
    int c0 = (int)(cx - radius * 3) - 1;
    int c1 = (int)(cx + radius * 3) + 2;
    if (r0 < 0)
        r0 = 0;
    if (r1 >= FEN)
        r1 = FEN - 1;
    if (c0 < 0)
        c0 = 0;
    if (c1 >= FEN)
        c1 = FEN - 1;

    double inv_r2 = 1.0 / (radius * radius + 0.01);

    for (int y = r0; y <= r1; y++) {
        for (int x = c0; x <= c1; x++) {
            double dx = x - cx, dy = y - cy;
            double d2 = dx * dx + dy * dy;
            double f  = intensitas * exp(-d2 * inv_r2 * 0.5);
            if (f < 0.002)
                continue;

            int idx = (y * FEN + x) * 4;
            int r   = (int)(fen[idx + 0] + col.r * f * 255);
            int g   = (int)(fen[idx + 1] + col.g * f * 255);
            int b   = (int)(fen[idx + 2] + col.b * f * 255);
            int a   = (int)(fen[idx + 3] + f * 255);
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            if (a > 255)
                a = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = (unsigned char)a;
        }
    }
}

/*
 * Spicula diffractionis — ex theoria Fraunhofer.
 */
static inline void fen_spicula(
    unsigned char *fen, double cx, double cy,
    double angulus, double longitudo, double latitudo,
    color_t col, double intensitas
) {
    double dx = cos(angulus);
    double dy = sin(angulus);
    int n     = (int)(longitudo * 3);
    if (n < 6)
        n = 6;

    for (int i = -n; i <= n; i++) {
        double t    = (double)i / (double)n;
        double dist = fabs(t) * longitudo;

        double env  = intensitas / (1.0 + dist * 0.4);
        double airy = 1.0 + 0.3 * cos(dist * 2.5);
        double f    = env * airy;
        if (f < 0.002)
            continue;

        double px = cx + t * longitudo * dx;
        double py = cy + t * longitudo * dy;
        fen_punctum(fen, px, py, latitudo, col, f);
    }
}

#endif /* SIDUS_COMMUNIA_H */
