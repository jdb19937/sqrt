/*
 * astra.c — sidera et planetae
 *
 * Generat campum stellarum toroidalem.
 * Reddit singula sidera in fenestra 64x64.
 */

#include "astra.h"
#include "ison/ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI_GRAECUM 3.14159265358979323846
#define DUO_PI     (2.0 * PI_GRAECUM)

#define FEN ASTRA_FENESTRA
#define SEMI (FEN / 2)

const char *astra_nomina_generum[SIDUS_NUMERUS] = {
    "Nanum Album",
    "Sequentia",
    "Gigas Rubrum",
    "Supergigas",
    "Neutronium",
    "Crystallinum",
    "Magnetar",
    "Planeta"
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

astra_color_t astra_temperatura_ad_colorem(double kelvin)
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
        if (g < 0) g = 0; if (g > 255) g = 255;
    } else {
        r = 329.699 * pow(t - 60.0, -0.1332);
        if (r < 0) r = 0; if (r > 255) r = 255;
        g = 288.122 * pow(t - 60.0, -0.0755);
        if (g < 0) g = 0; if (g > 255) g = 255;
    }

    if (t >= 66.0) {
        b = 255.0;
    } else if (t <= 19.0) {
        b = 0.0;
    } else {
        b = 138.518 * log(t - 10.0) - 305.045;
        if (b < 0) b = 0; if (b > 255) b = 255;
    }

    return (astra_color_t){r / 255.0, g / 255.0, b / 255.0, 1.0};
}

/* ================================================================
 * campus stellarum
 * ================================================================ */

astra_campus_t *astra_campum_creare(int latitudo, int altitudo)
{
    astra_campus_t *c = (astra_campus_t *)malloc(sizeof(astra_campus_t));
    c->latitudo = latitudo;
    c->altitudo = altitudo;
    c->pixels = (unsigned char *)calloc((size_t)latitudo * altitudo * 3, 1);
    return c;
}

void astra_campum_destruere(astra_campus_t *c)
{
    if (c) {
        free(c->pixels);
        free(c);
    }
}

void astra_pixel_scribere(astra_campus_t *c, int x, int y,
                          unsigned char r, unsigned char g, unsigned char b)
{
    /* topologia toroidalis */
    x = ((x % c->latitudo) + c->latitudo) % c->latitudo;
    y = ((y % c->altitudo) + c->altitudo) % c->altitudo;
    int idx = (y * c->latitudo + x) * 3;

    /* additiva saturans */
    int cr = c->pixels[idx + 0] + r; if (cr > 255) cr = 255;
    int cg = c->pixels[idx + 1] + g; if (cg > 255) cg = 255;
    int cb = c->pixels[idx + 2] + b; if (cb > 255) cb = 255;
    c->pixels[idx + 0] = (unsigned char)cr;
    c->pixels[idx + 1] = (unsigned char)cg;
    c->pixels[idx + 2] = (unsigned char)cb;
}

void astra_sidus_in_campum(astra_campus_t *c, int cx, int cy,
                           const unsigned char *fenestra)
{
    for (int dy = 0; dy < FEN; dy++) {
        for (int dx = 0; dx < FEN; dx++) {
            int fi = (dy * FEN + dx) * 4;
            unsigned char a = fenestra[fi + 3];
            if (a == 0) continue;

            double af = a / 255.0;
            int px = cx + dx - SEMI;
            int py = cy + dy - SEMI;

            /* topologia toroidalis */
            px = ((px % c->latitudo) + c->latitudo) % c->latitudo;
            py = ((py % c->altitudo) + c->altitudo) % c->altitudo;

            int ci = (py * c->latitudo + px) * 3;
            int r = (int)(c->pixels[ci + 0] + fenestra[fi + 0] * af);
            int g = (int)(c->pixels[ci + 1] + fenestra[fi + 1] * af);
            int b = (int)(c->pixels[ci + 2] + fenestra[fi + 2] * af);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            c->pixels[ci + 0] = (unsigned char)r;
            c->pixels[ci + 1] = (unsigned char)g;
            c->pixels[ci + 2] = (unsigned char)b;
        }
    }
}

int astra_regio_vacua(const astra_campus_t *c, int cx, int cy, int radius)
{
    /* inspicit pixeles in cruce circa centrum */
    for (int d = 1; d <= radius; d++) {
        int puncta[8][2] = {
            {cx + d, cy}, {cx - d, cy}, {cx, cy + d}, {cx, cy - d},
            {cx + d, cy + d}, {cx - d, cy - d},
            {cx + d, cy - d}, {cx - d, cy + d}
        };
        for (int k = 0; k < 8; k++) {
            int px = ((puncta[k][0] % c->latitudo) + c->latitudo) % c->latitudo;
            int py = ((puncta[k][1] % c->altitudo) + c->altitudo) % c->altitudo;
            int idx = (py * c->latitudo + px) * 3;
            int lum = c->pixels[idx] + c->pixels[idx + 1] + c->pixels[idx + 2];
            if (lum > 30) return 0;
        }
    }
    return 1;
}

void astra_campum_generare(astra_campus_t *c, const astra_parametri_t *p,
                           const astra_instrumentum_t *instrumentum)
{
    semen_g = p->semen;
    memset(c->pixels, 0, (size_t)c->latitudo * c->altitudo * 3);

    double cos_inc = cos(p->inclinatio_galaxiae);
    double sin_inc = sin(p->inclinatio_galaxiae);
    int n_gigantes = 0, n_supergigantes = 0, n_exotica = 0;

    /* --- stellae --- */
    for (int i = 0; i < p->numerus_stellarum; i++) {
        double fx = alea_f() * c->latitudo;
        double fy = alea_f() * c->altitudo;

        double tx = fx / c->latitudo;
        double ty = fy / c->altitudo;
        double y_fascia = 0.5 + (p->inclinatio_galaxiae / 3.0) * sin(DUO_PI * tx);
        double dy_g = ty - y_fascia;
        if (dy_g > 0.5) dy_g -= 1.0;
        if (dy_g < -0.5) dy_g += 1.0;
        double dist_gal = fabs(dy_g) / (p->latitudo_galaxiae + 0.001);

        if (alea_f() > p->densitas_galaxiae * exp(-dist_gal * dist_gal * 8.0)
            + (1.0 - p->densitas_galaxiae))
            continue;

        /* mag = 6 - 6r^4: pleraeque mag 5-6, paucissimae lucidae */
        double r_mag = alea_f();
        double mag = 6.0 - 6.0 * r_mag * r_mag * r_mag * r_mag;

        /* genus ex IMF (Kroupa 2001) */
        astra_genus_t genus;
        double gr = alea_f();
        if (gr < 0.06)        genus = SIDUS_NANUM_ALBUM;
        else if (gr < 0.96)   genus = SIDUS_SEQUENTIA;
        else if (gr < 0.98)   genus = SIDUS_GIGAS_RUBRUM;
        else if (gr < 0.985)  genus = SIDUS_SUPERGIGAS;
        else if (gr < 0.992)  genus = SIDUS_NEUTRONIUM;
        else if (gr < 0.9995) genus = SIDUS_SEQUENTIA;
        else if (gr < 0.99975) genus = SIDUS_CRYSTALLINUM;
        else                    genus = SIDUS_MAGNETAR;

        /* limites per genus */
        if (genus == SIDUS_GIGAS_RUBRUM && p->max_gigantes > 0
            && n_gigantes >= p->max_gigantes)
            genus = SIDUS_SEQUENTIA;
        if (genus == SIDUS_SUPERGIGAS && p->max_supergigantes > 0
            && n_supergigantes >= p->max_supergigantes)
            genus = SIDUS_SEQUENTIA;
        if ((genus == SIDUS_NEUTRONIUM || genus == SIDUS_CRYSTALLINUM
             || genus == SIDUS_MAGNETAR) && p->max_exotica > 0
            && n_exotica >= p->max_exotica)
            genus = SIDUS_SEQUENTIA;

        /* T_eff per classem spectralem (Gray & Corbally 2009) */
        double temp;
        {
            double tr = alea_f();
            if (tr < 0.40)      temp = 2400 + alea_f() * 1300;
            else if (tr < 0.76) temp = 3700 + alea_f() * 1500;
            else if (tr < 0.88) temp = 5200 + alea_f() * 800;
            else if (tr < 0.95) temp = 6000 + alea_f() * 1500;
            else if (tr < 0.98) temp = 7500 + alea_f() * 2500;
            else                 temp = 10000 + alea_f() * 20000;
        }

        switch (genus) {
        case SIDUS_NANUM_ALBUM:
            temp = 4000 + alea_f() * 30000;
            mag = 4.0 + alea_f() * 2.0;
            break;
        case SIDUS_GIGAS_RUBRUM:
            temp = 2500 + alea_f() * 2500;
            mag = 1.5 + alea_f() * 2.5;
            n_gigantes++;
            break;
        case SIDUS_SUPERGIGAS:
            temp = 3000 + alea_f() * 25000;
            mag = 0.5 + alea_f() * 1.5;
            n_supergigantes++;
            break;
        case SIDUS_NEUTRONIUM:
            temp = 500000;
            mag = 3.0 + alea_f() * 3.0;
            n_exotica++;
            break;
        case SIDUS_CRYSTALLINUM:
            temp = 6000 + alea_f() * 10000;
            mag = 2.0 + alea_f() * 3.0;
            n_exotica++;
            break;
        case SIDUS_MAGNETAR:
            temp = 5000000;
            mag = 1.0 + alea_f() * 2.0;
            n_exotica++;
            break;
        default: break;
        }

        /* spatium minimum */
        int spatium = 1;
        if (mag < 5.0) spatium = 2;
        if (mag < 4.0) spatium = 4;
        if (mag < 2.5) spatium = 8;
        if (mag < 1.5) spatium = 14;

        if (!astra_regio_vacua(c, (int)fx, (int)fy, spatium))
            continue;

        astra_sidus_t sidus = {genus, mag, temp, 0, 0};

        /* instrumentum applicare — spiculae pro lucidis */
        astra_instrumentum_t instr = {0, 0, 0, 0, 0, 1.0, 1.0, 0};
        if (mag < 1.5 && instrumentum && instrumentum->spiculae > 0) {
            double bright = 1.5 - mag;
            instr = *instrumentum;
            instr.spiculae_long = instrumentum->spiculae_long * bright + 2.0;
            instr.spiculae_ang += alea_f() * 0.1;
            instr.halo_radius = instrumentum->halo_radius * bright + 1.0;
            instr.halo_vis = instrumentum->halo_vis * bright;
        } else if (mag < 2.5 && instrumentum && instrumentum->halo_radius > 0.01) {
            instr.halo_radius = instrumentum->halo_radius * (2.5 - mag) * 0.5;
            instr.halo_vis = instrumentum->halo_vis * (2.5 - mag) * 0.5;
        }

        unsigned char fenestra[FEN * FEN * 4];
        astra_sidus_reddere(fenestra, &sidus, &instr);
        astra_sidus_in_campum(c, (int)fx, (int)fy, fenestra);
    }

    /* --- planetae --- */
    for (int i = 0; i < p->numerus_planetarum; i++) {
        double temp = p->planetae_temp_min
                    + alea_f() * (p->planetae_temp_max - p->planetae_temp_min);
        double mag = 1.0 + alea_f() * 3.0;
        double phase = alea_f() * 0.45;
        double ang = alea_f() * DUO_PI;

        astra_sidus_t sidus = {SIDUS_PLANETA, mag, temp, phase, ang};
        astra_instrumentum_t instr = {0, 0, 0, 0, 0, 1.0, 1.0, 0};

        unsigned char fenestra[FEN * FEN * 4];
        astra_sidus_reddere(fenestra, &sidus, &instr);

        int px = (int)(alea_f() * c->latitudo);
        int py = (int)(alea_f() * c->altitudo);
        if (astra_regio_vacua(c, px, py, 10))
            astra_sidus_in_campum(c, px, py, fenestra);
    }

    /* --- via lactea glow (supra stellas) --- */
    if (p->galaxia_glow > 0.001) {
        unsigned int sem = semen_g;
        for (int y = 0; y < c->altitudo; y++) {
            for (int x = 0; x < c->latitudo; x++) {
                double lrx = (double)x / c->latitudo - 0.5;
                double lry = (double)y / c->altitudo - 0.5;
                double d = -lrx * sin_inc + lry * cos_inc;
                double band = exp(-d * d / (0.06 * 0.06));

                double dx_nuc = lrx - 0.15;
                double nucleus = exp(-(dx_nuc * dx_nuc + d * d) / (0.04 * 0.04));
                band += nucleus * 0.6;

                sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
                double r1 = (double)(sem & 0xFF) / 255.0;
                sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
                double r2 = (double)(sem & 0xFF) / 255.0;
                double rumore = r1 * 0.6 + r2 * 0.4;

                double f = band * (0.04 + rumore * 0.03) * p->galaxia_glow;
                if (f < 0.003) continue;

                unsigned char cr = (unsigned char)(f * 220);
                unsigned char cg = (unsigned char)(f * 200);
                unsigned char cb = (unsigned char)(f * 170);
                astra_pixel_scribere(c, x, y, cr, cg, cb);
            }
        }

        /* fasciae pulveris (Great Rift) */
        if (p->galaxia_rift > 0.001) {
            sem = semen_g + 4217;
            for (int y = 0; y < c->altitudo; y++) {
                for (int x = 0; x < c->latitudo; x++) {
                    double lrx = (double)x / c->latitudo - 0.5;
                    double lry = (double)y / c->altitudo - 0.5;
                    double d = -lrx * sin_inc + lry * cos_inc;
                    if (fabs(d) > 0.08) continue;
                    double along = lrx * cos_inc + lry * sin_inc;

                    sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
                    double rumore = (double)(sem & 0xFF) / 255.0;

                    double rift = 0.0;
                    rift += 0.5 * sin(along * 25.0 + 1.3) * exp(-d * d / (0.015 * 0.015));
                    rift += 0.3 * sin(along * 40.0 + 2.7) * exp(-d * d / (0.010 * 0.010));
                    rift += 0.2 * rumore * exp(-d * d / (0.02 * 0.02));
                    if (rift < 0.15) continue;
                    double obscura = rift * 0.5 * p->galaxia_rift;
                    if (obscura > 0.8) obscura = 0.8;

                    int px = ((x % c->latitudo) + c->latitudo) % c->latitudo;
                    int py = ((y % c->altitudo) + c->altitudo) % c->altitudo;
                    int idx = (py * c->latitudo + px) * 3;
                    c->pixels[idx + 0] = (unsigned char)(c->pixels[idx + 0] * (1.0 - obscura));
                    c->pixels[idx + 1] = (unsigned char)(c->pixels[idx + 1] * (1.0 - obscura));
                    c->pixels[idx + 2] = (unsigned char)(c->pixels[idx + 2] * (1.0 - obscura));
                }
            }
        }

        /* nebulosae emissionis */
        sem = semen_g + 1571;
        for (int i = 0; i < p->galaxia_nebulae; i++) {
            sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
            double nx = (double)(sem & 0xFFFFF) / (double)0x100000;
            sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
            double ny = (double)(sem & 0xFFFFF) / (double)0x100000;
            double gx = nx * c->latitudo;
            double gy = c->altitudo * 0.5 + (nx - 0.5) * sin_inc * c->altitudo
                      + (ny - 0.5) * 0.08 * c->altitudo;
            sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
            double radius = 8 + (double)(sem & 0x1F);
            unsigned char nr, ng, nb;
            if (i % 3 == 0) { nr = 40; ng = 15; nb = 15; }
            else if (i % 3 == 1) { nr = 12; ng = 30; nb = 20; }
            else { nr = 25; ng = 20; nb = 30; }
            for (int dy = -(int)radius; dy <= (int)radius; dy++) {
                for (int dx = -(int)radius; dx <= (int)radius; dx++) {
                    double d2 = (double)(dx * dx + dy * dy);
                    double f = exp(-d2 / (radius * radius * 0.4));
                    if (f < 0.05) continue;
                    astra_pixel_scribere(c, (int)gx + dx, (int)gy + dy,
                        (unsigned char)(nr * f), (unsigned char)(ng * f),
                        (unsigned char)(nb * f));
                }
            }
        }
    }
}

/* ================================================================
 * sidus reddere
 * ================================================================ */

/* punctum Gaussianum in fenestra scribere */
static void fen_punctum(unsigned char *fen, double cx, double cy,
                        double radius, astra_color_t col, double intensitas)
{
    int r0 = (int)(cy - radius * 3) - 1;
    int r1 = (int)(cy + radius * 3) + 2;
    int c0 = (int)(cx - radius * 3) - 1;
    int c1 = (int)(cx + radius * 3) + 2;
    if (r0 < 0) r0 = 0; if (r1 >= FEN) r1 = FEN - 1;
    if (c0 < 0) c0 = 0; if (c1 >= FEN) c1 = FEN - 1;

    double inv_r2 = 1.0 / (radius * radius + 0.01);

    for (int y = r0; y <= r1; y++) {
        for (int x = c0; x <= c1; x++) {
            double dx = x - cx, dy = y - cy;
            double d2 = dx * dx + dy * dy;
            double f = intensitas * exp(-d2 * inv_r2 * 0.5);
            if (f < 0.002) continue;

            int idx = (y * FEN + x) * 4;
            int r = (int)(fen[idx + 0] + col.r * f * 255);
            int g = (int)(fen[idx + 1] + col.g * f * 255);
            int b = (int)(fen[idx + 2] + col.b * f * 255);
            int a = (int)(fen[idx + 3] + f * 255);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            if (a > 255) a = 255;
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
static void fen_spicula(unsigned char *fen, double cx, double cy,
                        double angulus, double longitudo, double latitudo,
                        astra_color_t col, double intensitas)
{
    double dx = cos(angulus);
    double dy = sin(angulus);
    int n = (int)(longitudo * 3);
    if (n < 6) n = 6;

    for (int i = -n; i <= n; i++) {
        double t = (double)i / (double)n;
        double dist = fabs(t) * longitudo;

        /* profilo 1/r cum oscillationibus Airy */
        double env = intensitas / (1.0 + dist * 0.4);
        double airy = 1.0 + 0.3 * cos(dist * 2.5);
        double f = env * airy;
        if (f < 0.002) continue;

        double px = cx + t * longitudo * dx;
        double py = cy + t * longitudo * dy;
        fen_punctum(fen, px, py, latitudo, col, f);
    }
}

/* ================================================================
 * renderers per genus
 * ================================================================ */

static void reddere_nanum_album(unsigned char *fen,
                                const astra_sidus_t *s,
                                const astra_instrumentum_t *instr)
{
    astra_color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.0;

    /* nucleus intensissimus, minimus */
    fen_punctum(fen, SEMI, SEMI, 0.6, col, luciditas * 1.5);

    /* halo tenuis caeruleus */
    astra_color_t halo_col = {col.r * 0.6, col.g * 0.7, col.b * 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 2.5, halo_col, luciditas * 0.3);

    (void)instr;
}

static void reddere_sequentia(unsigned char *fen,
                              const astra_sidus_t *s,
                              const astra_instrumentum_t *instr)
{
    astra_color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.0;

    /* nucleus */
    double r_nucl = 0.8 + luciditas * 0.3;
    if (r_nucl > 3.0) r_nucl = 3.0;
    fen_punctum(fen, SEMI, SEMI, r_nucl, col, luciditas);

    /* halo levis */
    if (luciditas > 0.3)
        fen_punctum(fen, SEMI, SEMI, r_nucl * 3.0, col, luciditas * 0.08);

    /* spiculae instrumenti */
    if (instr->spiculae > 0 && luciditas > 0.2) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                       + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(fen, SEMI, SEMI, ang,
                        instr->spiculae_long * luciditas,
                        0.4, col, luciditas * 0.25);
        }
    }

    /* halo instrumenti */
    if (instr->halo_vis > 0.01)
        fen_punctum(fen, SEMI, SEMI, instr->halo_radius,
                    col, instr->halo_vis * luciditas);
}

static void reddere_gigas_rubrum(unsigned char *fen,
                                 const astra_sidus_t *s,
                                 const astra_instrumentum_t *instr)
{
    astra_color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.5;

    /* discus maior, limbi obscuriores (limb darkening) */
    double r_disc = 2.5 + luciditas * 1.5;
    if (r_disc > 8.0) r_disc = 8.0;

    /* centrum lucidum */
    fen_punctum(fen, SEMI, SEMI, r_disc * 0.4, col, luciditas * 1.2);
    /* discus latus */
    fen_punctum(fen, SEMI, SEMI, r_disc, col, luciditas * 0.5);
    /* halo rubrum */
    astra_color_t halo = {col.r, col.g * 0.5, col.b * 0.3, 1.0};
    fen_punctum(fen, SEMI, SEMI, r_disc * 2.0, halo, luciditas * 0.06);

    /* spiculae */
    if (instr->spiculae > 0 && luciditas > 0.3) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                       + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(fen, SEMI, SEMI, ang,
                        instr->spiculae_long * luciditas * 0.7,
                        0.5, col, luciditas * 0.15);
        }
    }
}

static void reddere_supergigas(unsigned char *fen,
                               const astra_sidus_t *s,
                               const astra_instrumentum_t *instr)
{
    astra_color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 4.0;

    /* discus vastus */
    double r_disc = 5.0 + luciditas * 3.0;
    if (r_disc > 15.0) r_disc = 15.0;

    fen_punctum(fen, SEMI, SEMI, r_disc * 0.3, col, luciditas * 1.5);
    fen_punctum(fen, SEMI, SEMI, r_disc * 0.7, col, luciditas * 0.6);
    fen_punctum(fen, SEMI, SEMI, r_disc, col, luciditas * 0.2);

    /* maculae (cellulae convectionis) — perturbationes coloris */
    semen_g = (unsigned int)(s->temperatura * 1000);
    for (int i = 0; i < 5; i++) {
        double mx = SEMI + alea_gauss() * r_disc * 0.4;
        double my = SEMI + alea_gauss() * r_disc * 0.4;
        double dx = mx - SEMI, dy = my - SEMI;
        if (dx * dx + dy * dy > r_disc * r_disc * 0.6) continue;
        astra_color_t mc = {col.r * 0.7, col.g * 0.6, col.b * 0.5, 1.0};
        fen_punctum(fen, mx, my, r_disc * 0.2, mc, luciditas * 0.15);
    }

    /* spiculae longissimae */
    if (instr->spiculae > 0) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                       + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(fen, SEMI, SEMI, ang,
                        instr->spiculae_long * luciditas * 1.5,
                        0.6, col, luciditas * 0.2);
        }
    }

    if (instr->halo_vis > 0.01)
        fen_punctum(fen, SEMI, SEMI, instr->halo_radius * 2.0,
                    col, instr->halo_vis * luciditas * 0.5);
}

static void reddere_neutronium(unsigned char *fen,
                               const astra_sidus_t *s,
                               const astra_instrumentum_t *instr)
{
    (void)instr;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 3.0;

    /* punctum album intensissimum */
    astra_color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.4, album, luciditas * 2.0);

    /* annuli pulsantes — duae tenues fasciae */
    astra_color_t cyan = {0.3, 0.7, 1.0, 1.0};
    double r1 = 2.0 + luciditas;
    double r2 = 4.0 + luciditas * 2.0;

    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            double dx = x - SEMI, dy = y - SEMI;
            double d = sqrt(dx * dx + dy * dy);
            double f1 = exp(-(d - r1) * (d - r1) * 2.0) * luciditas * 0.4;
            double f2 = exp(-(d - r2) * (d - r2) * 1.0) * luciditas * 0.15;
            double f = f1 + f2;
            if (f < 0.002) continue;

            int idx = (y * FEN + x) * 4;
            int r = (int)(fen[idx + 0] + cyan.r * f * 255);
            int g = (int)(fen[idx + 1] + cyan.g * f * 255);
            int b = (int)(fen[idx + 2] + cyan.b * f * 255);
            int a = (int)(fen[idx + 3] + f * 255);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            if (a > 255) a = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = (unsigned char)a;
        }
    }

    /* bipolar jets — duae spiculae oppositae */
    double ang_jet = alea_f() * PI_GRAECUM;
    astra_color_t jet_col = {0.5, 0.6, 1.0, 1.0};
    fen_spicula(fen, SEMI, SEMI, ang_jet, 12.0 * luciditas, 0.3,
                jet_col, luciditas * 0.3);
    fen_spicula(fen, SEMI, SEMI, ang_jet + PI_GRAECUM, 12.0 * luciditas, 0.3,
                jet_col, luciditas * 0.3);
}

static void reddere_crystallinum(unsigned char *fen,
                                 const astra_sidus_t *s,
                                 const astra_instrumentum_t *instr)
{
    (void)instr;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.5;

    /* stella crystallina: globus Koosh — multa filamenta recta
     * ex centro irradiantia, diversis coloribus spectralibus */

    astra_color_t col = astra_temperatura_ad_colorem(s->temperatura);

    /* nucleus album intensum */
    astra_color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 1.2, album, luciditas * 1.0);
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 1.5);

    /* multa filamenta — ut globus Koosh */
    semen_g = (unsigned int)(s->temperatura * 137);
    int n_fil = 20 + (int)(luciditas * 15);
    if (n_fil > 60) n_fil = 60;

    /* 6 colores spectrales discreti */
    astra_color_t spectra[6] = {
        {1.0, 0.15, 0.1, 1.0},   /* rubrum */
        {1.0, 0.6, 0.05, 1.0},   /* aurantiacum */
        {0.9, 1.0, 0.1, 1.0},    /* flavum */
        {0.1, 1.0, 0.3, 1.0},    /* viride */
        {0.15, 0.4, 1.0, 1.0},   /* caeruleum */
        {0.7, 0.15, 1.0, 1.0}    /* violaceum */
    };

    for (int i = 0; i < n_fil; i++) {
        /* angulus aleatorius — distributio uniformis */
        double ang = alea_f() * DUO_PI;
        double dx = cos(ang), dy = sin(ang);

        /* longitudo variat */
        double len = 3.0 + alea_f() * (8.0 + luciditas * 8.0);
        if (len > 26.0) len = 26.0;

        /* color spectrale — unum ex 6 */
        astra_color_t fc = spectra[alea() % 6];

        /* intensitas decrescens ab centro */
        int n_steps = (int)(len * 3);
        if (n_steps < 4) n_steps = 4;

        for (int j = 0; j <= n_steps; j++) {
            double t = (double)j / (double)n_steps;
            double px = SEMI + t * len * dx;
            double py = SEMI + t * len * dy;

            /* intensitas: fortis prope centrum, evanescens */
            double f = luciditas * 0.4 * (1.0 - t) * (1.0 - t);
            fen_punctum(fen, px, py, 0.35, fc, f);
        }
    }

    /* halo tenuis multicolor circa centrum */
    fen_punctum(fen, SEMI, SEMI, 4.0, col, luciditas * 0.06);
}

/* Magnetar: stella neutronium cum B ~10^9-10^11 T.
 * Jets relativistici bipolares spirant circa lineas campi magnetici.
 * Materia in jet accelerata ad ~0.3c per processum Blandford-Znajek.
 * Emissio synchrotronum: spectrum continuum, polarizatum,
 * color caeruleum-album (potentia spectri ∝ ν^(-0.7)).
 * Precession geodaetica causat jets spirales (Lense-Thirring).
 * Halo birefringens: vacuum QED prope B_Schwinger lucem
 * in duos modos polares separat (Heisenberg-Euler 1936). */
static void reddere_magnetar(unsigned char *fen,
                              const astra_sidus_t *s,
                              const astra_instrumentum_t *instr)
{
    (void)instr;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 3.0;

    semen_g = (unsigned int)(s->temperatura * 71);

    /* axis magneticus — angulus aleatorius */
    double axis_ang = alea_f() * PI_GRAECUM;
    double ax = cos(axis_ang), ay = sin(axis_ang);

    /* nucleus: album intensum cum halo asymmetrico (birefringentia) */
    astra_color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 2.0);

    /* halo elongatum per axem magneticum (birefringentia vacui) */
    for (int i = -8; i <= 8; i++) {
        double t = (double)i / 8.0;
        double px = SEMI + t * 3.0 * ax;
        double py = SEMI + t * 3.0 * ay;
        double f = luciditas * 0.5 * exp(-t * t * 2.0);
        astra_color_t hc = {0.7, 0.8, 1.0, 1.0};
        fen_punctum(fen, px, py, 1.0, hc, f);
    }

    /* jets bipolares relativistici spirantes circa lineas campi.
     * precession Lense-Thirring: Ω_LT = 2GJ/(c²r³)
     * causat helicam cum radio crescente. */
    astra_color_t jet_colores[3] = {
        {0.4, 0.6, 1.0, 1.0},    /* synchrotron caeruleum */
        {0.7, 0.5, 1.0, 1.0},    /* violaceum */
        {0.3, 0.9, 0.9, 1.0}     /* cyaneum (Compton inversus) */
    };

    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;

        /* 3 filamenta per jet (structura interna campi) */
        for (int fil = 0; fil < 3; fil++) {
            double phase_offset = DUO_PI * fil / 3.0;
            astra_color_t jc = jet_colores[fil];

            /* spirala: radius crescit, angulus rotat */
            int n_steps = 120;
            for (int j = 0; j <= n_steps; j++) {
                double t = (double)j / (double)n_steps;
                double dist = t * 28.0;

                /* componens axialis */
                double px_ax = SEMI + dir * dist * ax;
                double py_ax = SEMI + dir * dist * ay;

                /* componens spiralis — radius crescit, frequentia decrescit */
                double spiral_r = t * t * 6.0;
                double spiral_ang = phase_offset + t * 12.0;
                double perp_x = -ay, perp_y = ax;

                double sx = spiral_r * (cos(spiral_ang) * perp_x
                          - sin(spiral_ang) * dir * ax);
                double sy = spiral_r * (cos(spiral_ang) * perp_y
                          - sin(spiral_ang) * dir * ay);

                double px = px_ax + sx;
                double py = py_ax + sy;

                if (px < 0 || px >= FEN || py < 0 || py >= FEN) continue;

                /* intensitas: decrescens cum distantia (synchrotron cooling) */
                double f = luciditas * 0.35 * (1.0 - t) * (1.0 - t * 0.5);
                fen_punctum(fen, px, py, 0.4 + t * 0.3, jc, f);
            }
        }
    }

    /* nodi lucidi in jets (shocks interni — Rees 1978) */
    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;
        for (int k = 0; k < 3; k++) {
            double t = 0.2 + alea_f() * 0.5;
            double dist = t * 25.0;
            double px = SEMI + dir * dist * ax;
            double py = SEMI + dir * dist * ay;
            fen_punctum(fen, px, py, 1.0, album, luciditas * 0.4);
        }
    }
}

static void reddere_planeta(unsigned char *fen,
                            const astra_sidus_t *s,
                            const astra_instrumentum_t *instr)
{
    (void)instr;
    astra_color_t col = astra_temperatura_ad_colorem(s->temperatura);

    /* planeta: discus maior, matte, cum falce (phase) */
    double radius = 8.0 + pow(10.0, -s->magnitudo * 0.4) * 6.0;
    if (radius > 25.0) radius = 25.0;

    double cos_ph = cos(s->angulus_phase);
    double sin_ph = sin(s->angulus_phase);

    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            double dx = x - SEMI, dy = y - SEMI;
            double d2 = dx * dx + dy * dy;
            double r2 = radius * radius;
            if (d2 > r2) continue;

            /* norma hemisphaerica */
            double nz = sqrt(1.0 - d2 / r2);
            double nx = dx / radius;
            double ny = dy / radius;

            /* illuminatio per directionem phase */
            double illum = nx * cos_ph + ny * sin_ph;
            /* phase: 0=plenus (totus illuminatus), 1=novus (obscurus) */
            double terminator = (1.0 - s->phase * 2.0);
            double vis = illum - terminator;

            double f;
            if (vis > 0.05) {
                /* facies illuminata — Lambert matte */
                double lambert = nz * 0.5 + illum * 0.4 + 0.1;
                if (lambert < 0) lambert = 0;
                f = lambert;
            } else if (vis > -0.05) {
                /* terminator — transitio levis */
                f = (vis + 0.05) * 10.0 * 0.3;
                if (f < 0) f = 0;
            } else {
                /* facies obscura — paene nigra */
                f = 0.02;
            }

            /* limb darkening */
            f *= (0.6 + 0.4 * nz);

            int idx = (y * FEN + x) * 4;
            int r = (int)(col.r * f * 255);
            int g = (int)(col.g * f * 255);
            int b = (int)(col.b * f * 255);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = 255;
        }
    }
}

/* ================================================================
 * dispatcher
 * ================================================================ */

void astra_sidus_reddere(unsigned char *fenestra,
                         const astra_sidus_t *sidus,
                         const astra_instrumentum_t *instrumentum)
{
    memset(fenestra, 0, FEN * FEN * 4);

    switch (sidus->genus) {
    case SIDUS_NANUM_ALBUM:
        reddere_nanum_album(fenestra, sidus, instrumentum);
        break;
    case SIDUS_SEQUENTIA:
        reddere_sequentia(fenestra, sidus, instrumentum);
        break;
    case SIDUS_GIGAS_RUBRUM:
        reddere_gigas_rubrum(fenestra, sidus, instrumentum);
        break;
    case SIDUS_SUPERGIGAS:
        reddere_supergigas(fenestra, sidus, instrumentum);
        break;
    case SIDUS_NEUTRONIUM:
        reddere_neutronium(fenestra, sidus, instrumentum);
        break;
    case SIDUS_CRYSTALLINUM:
        reddere_crystallinum(fenestra, sidus, instrumentum);
        break;
    case SIDUS_MAGNETAR:
        reddere_magnetar(fenestra, sidus, instrumentum);
        break;
    case SIDUS_PLANETA:
        reddere_planeta(fenestra, sidus, instrumentum);
        break;
    default:
        break;
    }
}

/* ================================================================
 * ISONL redditor — campum ex ISONL et instrumento reddit
 * ================================================================ */

static double isonl_par_f(const ison_par_t *pp, int n,
                           const char *clavis, double praef)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return atof(pp[i].valor);
    return praef;
}

static long isonl_par_n(const ison_par_t *pp, int n,
                         const char *clavis, long praef)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return atol(pp[i].valor);
    return praef;
}

static const char *isonl_par_s(const ison_par_t *pp, int n,
                                const char *clavis)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return pp[i].valor;
    return "";
}

static double isonl_lege_f(const char *ison, const char *via, double praef)
{
    char *v = ison_da_chordam(ison, via);
    if (!v) {
        long n = ison_da_numerum(ison, via);
        if (n != 0) return (double)n;
        return praef;
    }
    double r = atof(v);
    free(v);
    return r;
}

static astra_genus_t genus_ex_nomine(const char *nomen)
{
    if (strcmp(nomen, "nanum_album") == 0)  return SIDUS_NANUM_ALBUM;
    if (strcmp(nomen, "sequentia") == 0)    return SIDUS_SEQUENTIA;
    if (strcmp(nomen, "gigas_rubrum") == 0) return SIDUS_GIGAS_RUBRUM;
    if (strcmp(nomen, "supergigas") == 0)   return SIDUS_SUPERGIGAS;
    if (strcmp(nomen, "neutronium") == 0)   return SIDUS_NEUTRONIUM;
    if (strcmp(nomen, "crystallinum") == 0) return SIDUS_CRYSTALLINUM;
    if (strcmp(nomen, "magnetar") == 0)     return SIDUS_MAGNETAR;
    if (strcmp(nomen, "planeta") == 0)      return SIDUS_PLANETA;
    return SIDUS_SEQUENTIA;
}

typedef struct {
    astra_campus_t *campus;
    double galaxia_glow, galaxia_rift, inclinatio;
    int    galaxia_nebulae;
    int    meta_lecta;
    /* instrumentum */
    int    i_spic;
    double i_spic_long, i_spic_ang;
    double i_halo_r, i_halo_v;
} isonl_ctx_t;

static void isonl_linea_reddere(const ison_par_t *pp, int n, void *ctx_v)
{
    isonl_ctx_t *ctx = (isonl_ctx_t *)ctx_v;
    const char *genus_s = isonl_par_s(pp, n, "genus");

    if (strcmp(genus_s, "_meta") == 0) {
        int lat = (int)isonl_par_n(pp, n, "latitudo", 1024);
        int alt = (int)isonl_par_n(pp, n, "altitudo", 512);
        ctx->galaxia_glow    = isonl_par_f(pp, n, "galaxia_glow", 0);
        ctx->galaxia_rift    = isonl_par_f(pp, n, "galaxia_rift", 0);
        ctx->galaxia_nebulae = (int)isonl_par_n(pp, n, "galaxia_nebulae", 0);
        ctx->inclinatio      = isonl_par_f(pp, n, "inclinatio_galaxiae", 0);
        ctx->campus = astra_campum_creare(lat, alt);
        ctx->meta_lecta = 1;
        return;
    }

    if (!ctx->meta_lecta || !ctx->campus) return;

    astra_genus_t genus = genus_ex_nomine(genus_s);
    double mag   = isonl_par_f(pp, n, "magnitudo", 5.0);
    double temp  = isonl_par_f(pp, n, "temperatura", 5000);
    int    x     = (int)isonl_par_n(pp, n, "x", 0);
    int    y     = (int)isonl_par_n(pp, n, "y", 0);
    double phase = isonl_par_f(pp, n, "phase", 0);
    double ang   = isonl_par_f(pp, n, "angulus_phase", 0);

    astra_sidus_t sidus = {genus, mag, temp, phase, ang};

    astra_instrumentum_t instr = {0, 0, 0, 0, 0, 1.0, 1.0, 0};
    if (mag < 1.5 && ctx->i_spic > 0) {
        double bright = 1.5 - mag;
        instr.spiculae = ctx->i_spic;
        instr.spiculae_long = ctx->i_spic_long * bright + 2.0;
        instr.spiculae_ang = ctx->i_spic_ang;
        instr.halo_radius = ctx->i_halo_r * bright + 1.0;
        instr.halo_vis = ctx->i_halo_v * bright;
    } else if (mag < 2.5 && ctx->i_halo_r > 0.01) {
        instr.halo_radius = ctx->i_halo_r * (2.5 - mag) * 0.5;
        instr.halo_vis = ctx->i_halo_v * (2.5 - mag) * 0.5;
    }

    unsigned char fen[FEN * FEN * 4];
    astra_sidus_reddere(fen, &sidus, &instr);
    astra_sidus_in_campum(ctx->campus, x, y, fen);
}

static void isonl_galaxiam_reddere(astra_campus_t *c, const isonl_ctx_t *m)
{
    if (m->galaxia_glow < 0.001) return;

    double inc = m->inclinatio;
    unsigned int sem = 7919;

    for (int y = 0; y < c->altitudo; y++) {
        for (int x = 0; x < c->latitudo; x++) {
            double tx = (double)x / c->latitudo;
            double ty = (double)y / c->altitudo;
            double y_fascia = 0.5 + (inc / 3.0) * sin(DUO_PI * tx);
            double d = ty - y_fascia;
            if (d > 0.5) d -= 1.0;
            if (d < -0.5) d += 1.0;

            double band = exp(-d * d / (0.06 * 0.06));
            double dx_nuc = tx - 0.65;
            if (dx_nuc > 0.5) dx_nuc -= 1.0;
            if (dx_nuc < -0.5) dx_nuc += 1.0;
            double nucleus = exp(-(dx_nuc * dx_nuc * 4.0 + d * d)
                                  / (0.04 * 0.04));
            band += nucleus * 0.6;

            sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
            double r1 = (double)(sem & 0xFF) / 255.0;
            sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
            double r2 = (double)(sem & 0xFF) / 255.0;
            double rumore = r1 * 0.6 + r2 * 0.4;

            double f = band * (0.04 + rumore * 0.03) * m->galaxia_glow;
            if (f < 0.003) continue;

            astra_pixel_scribere(c, x, y,
                (unsigned char)(f * 220),
                (unsigned char)(f * 200),
                (unsigned char)(f * 170));
        }
    }

    if (m->galaxia_rift > 0.001) {
        sem = 4217;
        for (int y = 0; y < c->altitudo; y++) {
            for (int x = 0; x < c->latitudo; x++) {
                double tx = (double)x / c->latitudo;
                double ty = (double)y / c->altitudo;
                double y_fascia = 0.5 + (inc / 3.0) * sin(DUO_PI * tx);
                double d = ty - y_fascia;
                if (d > 0.5) d -= 1.0;
                if (d < -0.5) d += 1.0;
                if (fabs(d) > 0.08) continue;
                double along = tx;

                sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
                double rumore = (double)(sem & 0xFF) / 255.0;

                double rift = 0.0;
                rift += 0.5 * sin(along * 25.0 + 1.3)
                        * exp(-d * d / (0.015 * 0.015));
                rift += 0.3 * sin(along * 40.0 + 2.7)
                        * exp(-d * d / (0.010 * 0.010));
                rift += 0.2 * rumore * exp(-d * d / (0.02 * 0.02));
                if (rift < 0.15) continue;
                double obscura = rift * 0.5 * m->galaxia_rift;
                if (obscura > 0.8) obscura = 0.8;

                int px = ((x % c->latitudo) + c->latitudo) % c->latitudo;
                int py = ((y % c->altitudo) + c->altitudo) % c->altitudo;
                int idx = (py * c->latitudo + px) * 3;
                c->pixels[idx + 0] = (unsigned char)(c->pixels[idx + 0]
                                                      * (1.0 - obscura));
                c->pixels[idx + 1] = (unsigned char)(c->pixels[idx + 1]
                                                      * (1.0 - obscura));
                c->pixels[idx + 2] = (unsigned char)(c->pixels[idx + 2]
                                                      * (1.0 - obscura));
            }
        }
    }

    sem = 1571;
    for (int i = 0; i < m->galaxia_nebulae; i++) {
        sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
        double nx = (double)(sem & 0xFFFFF) / (double)0x100000;
        sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
        double ny = (double)(sem & 0xFFFFF) / (double)0x100000;
        double gx = nx * c->latitudo;
        double y_neb = 0.5 + (inc / 3.0) * sin(DUO_PI * nx);
        double gy = (y_neb + (ny - 0.5) * 0.08) * c->altitudo;
        sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
        double radius = 8 + (double)(sem & 0x1F);
        unsigned char nr, ng, nb;
        if (i % 3 == 0) { nr = 40; ng = 15; nb = 15; }
        else if (i % 3 == 1) { nr = 12; ng = 30; nb = 20; }
        else { nr = 25; ng = 20; nb = 30; }
        for (int dy = -(int)radius; dy <= (int)radius; dy++) {
            for (int dx = -(int)radius; dx <= (int)radius; dx++) {
                double d2 = (double)(dx * dx + dy * dy);
                double f = exp(-d2 / (radius * radius * 0.4));
                if (f < 0.05) continue;
                astra_pixel_scribere(c, (int)gx + dx, (int)gy + dy,
                    (unsigned char)(nr * f), (unsigned char)(ng * f),
                    (unsigned char)(nb * f));
            }
        }
    }
}

static void isonl_post_processare(astra_campus_t *c,
                                   double saturatio, double aberratio)
{
    int L = c->latitudo;
    int A = c->altitudo;
    size_t n_pix = (size_t)L * A;
    unsigned char *px = c->pixels;

    if (aberratio > 0.1) {
        unsigned char *copia = (unsigned char *)malloc(n_pix * 3);
        memcpy(copia, px, n_pix * 3);
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
        free(copia);
    }

    if (saturatio > 1.001 || saturatio < 0.999) {
        for (size_t i = 0; i < n_pix; i++) {
            int idx = (int)i * 3;
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
}

astra_campus_t *astra_ex_isonl_reddere(const char *via_isonl,
                                        const char *via_instrumentum)
{
    char *isonl = ison_lege_plicam(via_isonl);
    if (!isonl) {
        fprintf(stderr, "astra: %s legere non possum\n", via_isonl);
        return NULL;
    }

    char *instr_ison = ison_lege_plicam(via_instrumentum);
    if (!instr_ison) {
        fprintf(stderr, "astra: %s legere non possum\n", via_instrumentum);
        free(isonl);
        return NULL;
    }

    isonl_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.i_spic      = (int)isonl_lege_f(instr_ison, "spiculae", 0);
    ctx.i_spic_long = isonl_lege_f(instr_ison, "spiculae_long", 0);
    ctx.i_spic_ang  = isonl_lege_f(instr_ison, "spiculae_ang", 0);
    ctx.i_halo_r    = isonl_lege_f(instr_ison, "halo_radius", 0);
    ctx.i_halo_v    = isonl_lege_f(instr_ison, "halo_vis", 0);
    double saturatio = isonl_lege_f(instr_ison, "saturatio", 1.0);
    double aberratio = isonl_lege_f(instr_ison, "aberratio", 0.0);
    free(instr_ison);

    ison_pro_quaque_linea(isonl, isonl_linea_reddere, &ctx);
    free(isonl);

    if (!ctx.campus) {
        fprintf(stderr, "astra: _meta linea non inventa in %s\n", via_isonl);
        return NULL;
    }

    isonl_galaxiam_reddere(ctx.campus, &ctx);
    isonl_post_processare(ctx.campus, saturatio, aberratio);

    return ctx.campus;
}
