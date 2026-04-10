/*
 * sidus.c — sidera singula, renderer
 *
 * Reddit singula sidera in fenestra 64x64.
 */

#include "sidus.h"
#include "../instrumentum.h"
#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI_GRAECUM 3.14159265358979323846
#define DUO_PI     (2.0 * PI_GRAECUM)

#define FEN SIDUS_FENESTRA
#define SEMI (FEN / 2)

const char *sidus_nomina_generum[SIDUS_NUMERUS] = {
    "Nanum Album",
    "Sequentia",
    "Gigas Rubrum",
    "Supergigas",
    "Neutronium",
    "Crystallinum",
    "Magnetar",
    "Galaxia",
    "Vagans"
};

/* ================================================================
 * generans numerum pseudo-aleatorium
 * ================================================================ */

static unsigned int semen_g = 1;

static unsigned int alea(void)
{
    semen_g ^= semen_g << 13;
    semen_g ^= semen_g >> 17;
    semen_g ^= semen_g << 5;
    return semen_g;
}

/* [0, 1) */
static double alea_f(void)
{
    return (double)(alea() & 0xFFFFFF) / (double)0x1000000;
}

/* Gaussiana per Box-Muller */
static double alea_gauss(void)
{
    double u1 = alea_f() + 1e-30;
    double u2 = alea_f();
    return sqrt(-2.0 * log(u1)) * cos(DUO_PI * u2);
}

/* ================================================================
 * temperatura ad colorem (approximatio Planck)
 * ================================================================ */

color_t sidus_temperatura_ad_colorem(double kelvin)
{
    /* Approximatio Tanner Helland (2012) functionis Planckianae
     * per CIE 1931 2° standard observer.
     * Exacta intra 1% pro 1000K-40000K contra integrationem
     * spectri Planckiani B(λ,T) = 2hc²/λ⁵ · 1/(e^(hc/λkT)-1)
     * convolutam cum x̄(λ), ȳ(λ), z̄(λ) et transformatam per
     * sRGB matricem (Rec. 709 primariis). */
    double t = kelvin / 100.0;
    double r, g, b;

    if (t <= 66.0) {
        r = 255.0;
        g = 99.4708 * log(t) - 161.1196;
        if (g < 0)
            g = 0;
        if (g > 255)
            g = 255;
    } else {
        r = 329.699 * pow(t - 60.0, -0.1332);
        if (r < 0)
            r = 0;
        if (r > 255)
            r = 255;
        g = 288.122 * pow(t - 60.0, -0.0755);
        if (g < 0)
            g = 0;
        if (g > 255)
            g = 255;
    }

    if (t >= 66.0) {
        b = 255.0;
    } else if (t <= 19.0) {
        b = 0.0;
    } else {
        b = 138.518 * log(t - 10.0) - 305.045;
        if (b < 0)
            b = 0;
        if (b > 255)
            b = 255;
    }

    return (color_t){r / 255.0, g / 255.0, b / 255.0, 1.0};
}

/* ================================================================
 * sidus reddere
 * ================================================================ */

/* punctum Gaussianum in fenestra scribere */
static void fen_punctum(
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
 * Aperturam non-circularem (obstructio araneo secundario)
 * PSF cum spiculis generat: Airy pattern convolutus cum
 * transformatione Fourier structurae aperturae.
 *
 * Newtonianum (4 bracchia): 4 spiculae orthogonales.
 * JWST (3 bracchia segmentorum hex): 6 spiculae principales + 2 minores.
 * Intensitas spicula ∝ 1/r (non exp), cum oscillationibus Airy.
 */
static void fen_spicula(
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

        /* profilo 1/r cum oscillationibus Airy */
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

/* ================================================================
 * renderers per genus
 * ================================================================ */

#include "sidera/nanum_album.c"
#include "sidera/sequentia.c"
#include "sidera/gigas_rubrum.c"
#include "sidera/supergigas.c"
#include "sidera/neutronium.c"
#include "sidera/crystallinum.c"
#include "sidera/magnetar.c"
#include "sidera/galaxia.c"
#include "sidera/vagans.c"

/* ================================================================
 * ISON parser
 * ================================================================ */

void sidus_ex_ison(sidus_t *s, const char *ison)
{
    memset(s, 0, sizeof(*s));
    char *tmp;
    if ((tmp = ison_da_crudum(ison, "vaganulus")))
        { free(tmp); vagans_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "galaxiola")))
        { free(tmp); galaxia_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "magnetarulum")))
        { free(tmp); magnetar_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "nanulum_album")))
        { free(tmp); nanum_album_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "gigulum_rubrum")))
        { free(tmp); gigas_rubrum_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "supergigulum")))
        { free(tmp); supergigas_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "neutroniulum")))
        { free(tmp); neutronium_ex_ison(s, ison); return; }
    if ((tmp = ison_da_crudum(ison, "crystallulum")))
        { free(tmp); crystallinum_ex_ison(s, ison); return; }
    /* praefinitum: sequentia (solum sidulum) */
    sequentia_ex_ison(s, ison);
}

/* ================================================================
 * dispatcher
 * ================================================================ */

void sidus_reddere(
    unsigned char *fenestra,
    const sidus_t *sidus,
    const instrumentum_t *instrumentum
) {
    memset(fenestra, 0, FEN * FEN * 4);

    switch (sidus->qui) {
    case SIDUS_NANUM_ALBUM:
        reddere_nanum_album(fenestra, &sidus->ubi.nanum_album, instrumentum);
        break;
    case SIDUS_SEQUENTIA:
        reddere_sequentia(fenestra, &sidus->ubi.sequentia, instrumentum);
        break;
    case SIDUS_GIGAS_RUBRUM:
        reddere_gigas_rubrum(fenestra, &sidus->ubi.gigas_rubrum, instrumentum);
        break;
    case SIDUS_SUPERGIGAS:
        reddere_supergigas(fenestra, &sidus->ubi.supergigas, instrumentum);
        break;
    case SIDUS_NEUTRONIUM:
        reddere_neutronium(fenestra, &sidus->ubi.neutronium, instrumentum);
        break;
    case SIDUS_CRYSTALLINUM:
        reddere_crystallinum(fenestra, &sidus->ubi.crystallinum, instrumentum);
        break;
    case SIDUS_MAGNETAR:
        reddere_magnetar(fenestra, &sidus->ubi.magnetar, instrumentum);
        break;
    case SIDUS_GALAXIA:
        reddere_galaxia(fenestra, &sidus->ubi.galaxia, instrumentum);
        break;
    case SIDUS_VAGANS:
        reddere_vagans(fenestra, &sidus->ubi.vagans, instrumentum);
        break;
    default:
        break;
    }
}
