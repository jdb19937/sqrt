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
    "Galaxia",
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
        astra_instrumentum_t instr = {.saturatio = 1.0};
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
        astra_instrumentum_t instr = {.saturatio = 1.0};

        unsigned char fenestra[FEN * FEN * 4];
        astra_sidus_reddere(fenestra, &sidus, &instr);

        int px = (int)(alea_f() * c->latitudo);
        int py = (int)(alea_f() * c->altitudo);
        if (astra_regio_vacua(c, px, py, 10))
            astra_sidus_in_campum(c, px, py, fenestra);
    }

    /* --- galaxiae distantes --- */
    /* distributio magnitudinis Schechter (1976): pleraeque debiles.
     * implementatio: mag = 3 + 3·(1-r^0.4) dat concentrationem
     * circa mag 5-6 cum cauda rara ad mag 3 (lucidae ut M31).
     * morphologia ex distributione observata (Lintott+ 2008,
     * Galaxy Zoo; Conselice 2006):
     *   ~15% ellipticae, ~35% spirales, ~25% barratae,
     *   ~15% lenticulares, ~10% irregulares. */
    {
        int n_gal = 0;
        for (int i = 0; i < p->numerus_galaxiarum; i++) {
            if (p->max_galaxiae > 0 && n_gal >= p->max_galaxiae) break;

            /* magnitudo: Schechter — pleraeque debiles */
            double r_mag = alea_f();
            double mag = 3.0 + 3.0 * (1.0 - pow(r_mag, 0.4));

            /* morphologia */
            galaxia_morphologia_t morph;
            double mr = alea_f();
            if (mr < 0.15)       morph = GALAXIA_ELLIPTICA;
            else if (mr < 0.50)  morph = GALAXIA_SPIRALIS;
            else if (mr < 0.75)  morph = GALAXIA_SPIRALIS_BARRATA;
            else if (mr < 0.90)  morph = GALAXIA_LENTICULARIS;
            else                 morph = GALAXIA_IRREGULARIS;

            /* inclinatio: cos(i) uniformis in [0,1] (orientationes
             * aleatoriae in spatio 3D). codificamus ut T/10000. */
            double cos_incl = alea_f();
            double temp_code = cos_incl * 10000.0;

            double ang = alea_f() * DUO_PI;

            astra_sidus_t sidus = {SIDUS_GALAXIA, mag, temp_code,
                                   (double)morph, ang};
            astra_instrumentum_t instr = {.saturatio = 1.0};

            unsigned char fenestra[FEN * FEN * 4];
            astra_sidus_reddere(fenestra, &sidus, &instr);

            int px = (int)(alea_f() * c->latitudo);
            int py = (int)(alea_f() * c->altitudo);
            int spatium = (mag < 4.0) ? 8 : (mag < 5.0) ? 4 : 2;
            if (astra_regio_vacua(c, px, py, spatium)) {
                astra_sidus_in_campum(c, px, py, fenestra);
                n_gal++;
            }
        }
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

    /* semen ex magnitudo et temperatura derivatum — unicum per sidus.
     * antea: semen ex sola temperatura, sed magnetares omnes T=5×10⁶ K
     * habent, ergo omnes eandem orientationem reddebant.
     * nunc magnitudinem addimus ut variatio inter magnetares oriatur. */
    semen_g = (unsigned int)(s->temperatura * 71
            + s->magnitudo * 100003 + s->phase * 70001);

    /* axis magneticus — angulus aleatorius per plenum circulum.
     * in magnetaribus veris, axis magneticus non cum axe rotationis
     * congruere necesse est (obliquitas typica 10°-60°, Lander & Jones
     * 2009). axis proiectus in planum caeli quemlibet angulum habere
     * potest, ergo DUO_PI non PI_GRAECUM adhibemus. */
    double axis_ang = alea_f() * DUO_PI;
    double ax = cos(axis_ang), ay = sin(axis_ang);

    /* variatio individualis magnetaris:
     * campus magneticus B variat 10⁹-10¹¹ T (Duncan & Thompson 1992).
     * B fortior → jets longiores et collimatiores (Bucciantini+ 2006).
     * periodus rotationis P = 2-12 s → precession rate variat.
     * aetas τ = P/(2Ṗ) ~ 10³-10⁵ a → jets seniores latiores et debiliores. */
    double jet_long = 16.0 + alea_f() * 20.0;    /* 16-36 pixeles */
    double jet_apertura = 3.0 + alea_f() * 6.0;  /* amplitudo spiralis */
    double jet_freq = 8.0 + alea_f() * 10.0;     /* frequentia spiralis */
    int    jet_fil = 2 + (int)(alea_f() * 3);     /* 2-4 filamenta */
    double halo_long = 2.0 + alea_f() * 3.0;     /* elongatio hali */

    /* nucleus: album intensum cum halo asymmetrico (birefringentia) */
    astra_color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 2.0);

    /* halo elongatum per axem magneticum (birefringentia vacui).
     * QED in campo magnetico extremo (B > B_Schwinger = m²c³/eℏ =
     * 4.414×10⁹ T) vacuum birefringens fit: duae polarizationes
     * cum indicibus refractionis diversis propagantur (Heisenberg &
     * Euler 1936, Adler 1971). hoc imaginem stellae elongat
     * perpendiculare ad campum magneticum proiectum (Heyl & Shaviv
     * 2000, 2002). in data Chandra, RX J1856.5-3754 elongationem
     * consistentem cum birefringentia vacui ostendit (Mignani+ 2017,
     * confirmatio prima effectus QED in campo astronomico). */
    for (int i = -8; i <= 8; i++) {
        double t = (double)i / 8.0;
        double px = SEMI + t * halo_long * ax;
        double py = SEMI + t * halo_long * ay;
        double f = luciditas * 0.5 * exp(-t * t * 2.0);
        astra_color_t hc = {0.7, 0.8, 1.0, 1.0};
        fen_punctum(fen, px, py, 1.0, hc, f);
    }

    /* jets bipolares relativistici spirantes circa lineas campi.
     *
     * mechanismus: materia accreti per campum magneticum ad polos
     * dirigitur (Goldreich & Julian 1969). particuli relativistici
     * in campo magnetico radiationem synchrotron emittunt cum
     * spectro potentiae F_ν ∝ ν^α ubi α ≈ -0.6 ad -0.8
     * (Rybicki & Lightman 1979). color ergo caeruleus-albus.
     *
     * precession Lense-Thirring (Lense & Thirring 1918):
     * stella neutronium rotans spatium-tempus circa se trahit
     * (frame dragging). axis praecessionis Ω_LT = 2GJ/(c²r³)
     * ubi J = momentum angulare. hoc causat jets spirales —
     * helicam cum radio crescente a polo.
     *
     * collimatio: jets magnetaris typice apertura ~5°-30°
     * (Granot+ 2017), strictiores quam jets AGN (~15°-60°). */
    astra_color_t jet_colores[4] = {
        {0.4, 0.6, 1.0, 1.0},    /* synchrotron caeruleum */
        {0.7, 0.5, 1.0, 1.0},    /* violaceum */
        {0.3, 0.9, 0.9, 1.0},    /* cyaneum (Compton inversus) */
        {0.5, 0.4, 1.0, 1.0}     /* indicum */
    };

    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;

        for (int fil = 0; fil < jet_fil; fil++) {
            double phase_offset = DUO_PI * fil / (double)jet_fil;
            astra_color_t jc = jet_colores[fil % 4];

            int n_steps = 120;
            for (int j = 0; j <= n_steps; j++) {
                double t = (double)j / (double)n_steps;
                double dist = t * jet_long;

                double px_ax = SEMI + dir * dist * ax;
                double py_ax = SEMI + dir * dist * ay;

                double spiral_r = t * t * jet_apertura;
                double spiral_ang = phase_offset + t * jet_freq;
                double perp_x = -ay, perp_y = ax;

                double sx = spiral_r * (cos(spiral_ang) * perp_x
                          - sin(spiral_ang) * dir * ax);
                double sy = spiral_r * (cos(spiral_ang) * perp_y
                          - sin(spiral_ang) * dir * ay);

                double px = px_ax + sx;
                double py = py_ax + sy;

                if (px < 0 || px >= FEN || py < 0 || py >= FEN) continue;

                double f = luciditas * 0.35 * (1.0 - t) * (1.0 - t * 0.5);
                fen_punctum(fen, px, py, 0.4 + t * 0.3, jc, f);
            }
        }
    }

    /* nodi lucidi in jets (shocks interni).
     * Rees (1978) primus proposuit fluctuationes in fluxu jet
     * shells cum velocitatibus diversis creare quae collisionibus
     * "shocks internos" producunt. hoc in blazaribus (Marscher &
     * Gear 1985) et in GRBs (Rees & Mészáros 1994) confirmatum.
     * in magnetaribus, giant flares (ut SGR 1806-20, Palmer+ 2005)
     * nodos similes per eruptiones in jets creant. */
    int n_nodi = 2 + (int)(alea_f() * 3);
    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;
        for (int k = 0; k < n_nodi; k++) {
            double t = 0.15 + alea_f() * 0.6;
            double dist = t * jet_long * 0.9;
            double px = SEMI + dir * dist * ax;
            double py = SEMI + dir * dist * ay;
            fen_punctum(fen, px, py, 0.8 + alea_f() * 0.5, album,
                        luciditas * (0.2 + alea_f() * 0.3));
        }
    }
}

/*
 * reddere_galaxia — galaxia distans in fenestra 64×64.
 *
 * Galaxiae ut objecta extendida redduntur, non ut puncta.
 * Morphologia (in campo "phase" sideris) formam determinat:
 *
 *   GALAXIA_ELLIPTICA:
 *     Profilo de Vaucouleurs (1948, "R^{1/4} law"):
 *       I(r) = I_e · exp(-7.67·((r/r_e)^{1/4} - 1))
 *     ubi I_e = luminositas superficialis ad r_e.
 *     Sérsic (1963) generalizavit: exp(-b_n·((r/r_e)^{1/n}-1))
 *     ubi n=4 dat de Vaucouleurs, n=1 dat exponentialem.
 *     Ellipticitas ex inclinatione et forma intrinseca:
 *     galaxiae vere triaxiales (Binney 1978, Franx+ 1991).
 *     Colores: rubri-aurei, T_eff effectivus ~4000-5000 K
 *     (dominantur stellae K gigantes, Worthey 1994).
 *
 *   GALAXIA_SPIRALIS:
 *     Nucleus (bulge): profilo Sérsic n~2-3.
 *     Discus: profilo exponentiale I(r) = I_0·exp(-r/h_r)
 *     ubi h_r = scala longitudinis (Freeman 1970).
 *     Brachia: perturbatio logarithmica r(θ) = a·exp(b·θ)
 *     (Ringermacher & Mead 2009). numerus brachiorum typice
 *     m=2 (dominans) vel m=4 (flocculent).
 *     Face-on: brachia caerulea visibilia.
 *     Edge-on: discus tenuis cum fascia pulveris centrali
 *     (sicut NGC 891, van der Kruit & Searle 1981).
 *     Inclinatio continua inter haec extrema.
 *
 *   GALAXIA_SPIRALIS_BARRATA:
 *     Ut spiralis sed cum barra stellari centrali.
 *     Barra: structura elongata cum profilo Ferrers (1877):
 *       ρ(m) ∝ (1 - m²)^n ubi m = distantia normalizata.
 *     Brachia ex extremis barrae oriuntur, non ex nucleo.
 *     Resonantiae orbitales (Contopoulos & Papayannopoulos
 *     1980): orbita x1 sustinet barram, x2 perpendicularis
 *     destruit eam — transitus inter regimina determinat
 *     longitudinem barrae.
 *
 *   GALAXIA_LENTICULARIS:
 *     Discus sine brachiis + nucleus prominens.
 *     Profilo compositus: Sérsic (nucleus, n~2-4) +
 *     exponentiale (discus). similaris spirali edge-on
 *     sed sine fascia pulveris et sine nodis formationis.
 *     Laurikainen+ (2010): pleraeque S0 habent etiam
 *     structuras barrae debiles ("S0/a").
 *
 *   GALAXIA_IRREGULARIS:
 *     Sine symmetria regulari. nodi starburst (luminosi,
 *     caerulei) distributi asymmetrice. Hunter & Elmegreen
 *     (2004): formatio stellarum in irregularibus non in
 *     brachiis sed in regionibus stochasticis compressionis.
 *     Exempla: LMC (cum structura barrae residua), SMC,
 *     NGC 1427A (in Fornax, cum cauda mareali).
 *
 * Distributio magnitudinis sequitur Schechter (1976):
 *   φ(L) ∝ (L/L*)^α · exp(-L/L*), α ≈ -1.25.
 *   implementatio: mag = mag_min + Δmag · (1-r^0.4)
 *   ubi r uniformis — pleraeque debiles, paucae lucidae.
 */
static void reddere_galaxia(unsigned char *fen,
                             const astra_sidus_t *s,
                             const astra_instrumentum_t *instr)
{
    (void)instr;

    /* semen unicum ex omnibus proprietatibus */
    semen_g = (unsigned int)(s->temperatura * 137
            + s->magnitudo * 100003 + s->phase * 7001
            + s->angulus_phase * 50021);

    galaxia_morphologia_t morph = (galaxia_morphologia_t)(int)s->phase;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.5;
    double ang = s->angulus_phase;   /* angulus positionis in caelo */
    double ca = cos(ang), sa = sin(ang);

    /* inclinatio ad lineam visus: 0 = face-on, π/2 = edge-on.
     * distributio uniformis in cos(i) (orientationes aleatoriae
     * in spatio 3D, Hubble 1926). ergo i = arccos(r) ubi r ∈ [0,1].
     * implementamus per "temperatura" campi: T/10000 dat cos(i). */
    double cos_incl = s->temperatura / 10000.0;
    if (cos_incl < 0.05) cos_incl = 0.05;
    if (cos_incl > 1.0) cos_incl = 1.0;

    /* radius effectivus — galaxiae distantes parvae,
     * proximae (lucidiores) maiores. */
    double r_eff = 1.5 + luciditas * 3.0;
    if (r_eff > 20.0) r_eff = 20.0;

    /* ellipticitas apparens ex inclinatione:
     * discus circularis inclinatus: b/a = cos(i).
     * elliptica intrinseca: b/a ∈ [0.3, 1.0]. */
    double ba_ratio;  /* b/a axis ratio */
    double ell_intrin = 0.7 + alea_f() * 0.3;  /* ellipticae: 0.7-1.0 */

    switch (morph) {
    case GALAXIA_ELLIPTICA:
        ba_ratio = ell_intrin;
        break;
    case GALAXIA_LENTICULARIS:
        ba_ratio = cos_incl * 0.9 + 0.1;
        break;
    default:  /* spirales, barratae, irregulares */
        ba_ratio = cos_incl * 0.95 + 0.05;
        break;
    }

    /* color dominans per morphologiam.
     * ellipticae et lenticulares: stellae veteres, rubrae-aureae.
     *   metalicitas alta, [Fe/H] ~ 0 ad +0.3 (Thomas+ 2005).
     * spirales: nucleus aureus, brachia caerulea (O/B stellae).
     * irregulares: caerulescentes (formatio stellarum activa). */
    astra_color_t col_nuc, col_ext;
    switch (morph) {
    case GALAXIA_ELLIPTICA:
        col_nuc = (astra_color_t){1.0, 0.85, 0.6, 1.0};  /* aureum */
        col_ext = (astra_color_t){0.9, 0.75, 0.55, 1.0};
        break;
    case GALAXIA_LENTICULARIS:
        col_nuc = (astra_color_t){1.0, 0.88, 0.65, 1.0};
        col_ext = (astra_color_t){0.85, 0.78, 0.6, 1.0};
        break;
    case GALAXIA_SPIRALIS:
    case GALAXIA_SPIRALIS_BARRATA:
        col_nuc = (astra_color_t){1.0, 0.9, 0.65, 1.0};  /* nucleus aureus */
        col_ext = (astra_color_t){0.6, 0.75, 1.0, 1.0};  /* brachia caerulea */
        break;
    case GALAXIA_IRREGULARIS:
        col_nuc = (astra_color_t){0.7, 0.8, 1.0, 1.0};   /* caerulea */
        col_ext = (astra_color_t){0.6, 0.7, 1.0, 1.0};
        break;
    default:
        col_nuc = (astra_color_t){1.0, 0.9, 0.7, 1.0};
        col_ext = (astra_color_t){0.8, 0.8, 0.8, 1.0};
        break;
    }

    /* variatio individualis coloris */
    double dR = (alea_f() - 0.5) * 0.15;
    double dG = (alea_f() - 0.5) * 0.10;
    double dB = (alea_f() - 0.5) * 0.10;
    col_nuc.r += dR; col_nuc.g += dG; col_nuc.b += dB;
    col_ext.r += dR * 0.5; col_ext.g += dG * 0.5; col_ext.b += dB * 0.5;

    /* --- redditio per morphologiam --- */

    if (morph == GALAXIA_ELLIPTICA) {
        /* profilo de Vaucouleurs: I ∝ exp(-7.67·((r/r_e)^0.25 - 1)).
         * simplificamus ad Gaussianam pro galaxiis parvis,
         * cum forma elliptica ex ba_ratio. */
        for (int dy = -30; dy <= 30; dy++) {
            for (int dx = -30; dx <= 30; dx++) {
                /* coordinatae rotatae per angulum positionis */
                double lx = dx * ca + dy * sa;
                double ly = -dx * sa + dy * ca;
                /* ellipticitas */
                double rx = lx / r_eff;
                double ry = ly / (r_eff * ba_ratio);
                double r2 = rx * rx + ry * ry;
                double r = sqrt(r2);
                if (r > 5.0) continue;
                /* Sérsic n=4 approximatum: exp(-7.67·(r^0.25 - 1)) */
                double sersic = exp(-7.67 * (pow(r + 0.01, 0.25) - 1.0));
                double f = luciditas * sersic;
                if (f < 0.002) continue;
                /* color: gradiens ab aureo (centrum) ad rubescente (extra) */
                astra_color_t c;
                double t = r / 3.0; if (t > 1.0) t = 1.0;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.6, c, f);
            }
        }
    } else if (morph == GALAXIA_SPIRALIS || morph == GALAXIA_SPIRALIS_BARRATA) {
        /* nucleus (bulge) — Sérsic n~2 */
        double r_bulge = r_eff * 0.3;
        fen_punctum(fen, SEMI, SEMI, r_bulge, col_nuc, luciditas * 1.5);

        /* barra (solum pro barratis) */
        if (morph == GALAXIA_SPIRALIS_BARRATA) {
            double bar_len = r_eff * (0.4 + alea_f() * 0.3);
            double bar_wid = r_eff * 0.12;
            for (int i = -40; i <= 40; i++) {
                double t = (double)i / 40.0;
                double bx = t * bar_len;
                double by = 0;
                /* rotare per ang + applicare inclinationem */
                double px = bx * ca - by * sa;
                double py = (bx * sa + by * ca) * ba_ratio;
                double dist = fabs(t);
                double f = luciditas * 0.6 * exp(-dist * dist * 3.0);
                if (f < 0.002) continue;
                fen_punctum(fen, SEMI + px, SEMI + py, bar_wid * ba_ratio,
                            col_nuc, f);
            }
        }

        /* brachia spiralia — r(θ) = a·exp(b·θ), spirala logarithmica.
         * pitch angle typice 10°-30° (Kennicutt 1981):
         *   tan(pitch) = 1/(b·r), ergo b = 1/tan(pitch).
         *   Sa: pitch ~10° (stricta), Sc: pitch ~25° (laxa).
         * duo brachia (m=2) dominans modus per theoriam Lin-Shu. */
        int n_brachia = 2;
        double pitch = 0.3 + alea_f() * 0.4;  /* 0.3-0.7 rad (~17°-40°) */
        double r_start = r_eff * 0.25;
        if (morph == GALAXIA_SPIRALIS_BARRATA)
            r_start = r_eff * 0.45;  /* brachia ex extremis barrae */
        double arm_bright = 0.5 + alea_f() * 0.3;

        for (int arm = 0; arm < n_brachia; arm++) {
            double theta0 = DUO_PI * arm / n_brachia + alea_f() * 0.3;
            int n_steps = 200;
            for (int j = 0; j < n_steps; j++) {
                double t = (double)j / (double)n_steps;
                double theta = theta0 + t * DUO_PI / tan(pitch);
                double r = r_start + t * r_eff * 1.8;

                /* spirala logarithmica cum perturbatione */
                double perturb = alea_f() * 0.15 - 0.075;
                double x_arm = r * cos(theta + perturb);
                double y_arm = r * sin(theta + perturb);

                /* proiectio cum inclinatione et rotatione */
                double px = x_arm * ca - y_arm * sa;
                double py = (x_arm * sa + y_arm * ca) * ba_ratio;

                if (fabs(px) > 28 || fabs(py) > 28) continue;

                double fade = (1.0 - t) * (1.0 - t * 0.3);
                double f = luciditas * arm_bright * fade;
                if (f < 0.002) continue;

                /* regiones HII in brachiis (Kennicutt 1998):
                 * formatio stellarum in undis densitatis.
                 * nodi lucidi stochastice distributi. */
                double nod = 1.0;
                if (alea_f() < 0.08) nod = 1.5 + alea_f();
                double wid = 0.5 + t * 0.8;
                fen_punctum(fen, SEMI + px, SEMI + py, wid,
                            col_ext, f * nod);
            }
        }

        /* discus diffusus exponentialis (Freeman 1970) */
        for (int dy = -25; dy <= 25; dy++) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca + dy * sa;
                double ly = (-dx * sa + dy * ca) / (ba_ratio + 0.01);
                double r = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 3.0) continue;
                double disc = exp(-r / r_eff) * 0.3;
                double f = luciditas * disc;
                if (f < 0.002) continue;
                double t = r / (r_eff * 2.5); if (t > 1.0) t = 1.0;
                astra_color_t c;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, c, f);
            }
        }

        /* fascia pulveris pro edge-on (cos_incl < 0.3):
         * in galaxiis edge-on, fascia obscura centralis visibilis
         * (exemplum classicum: NGC 891, van der Kruit & Searle 1981).
         * extinctio per pulverem: A_V ≈ τ_V / (cos(i) + 0.1). */
        if (cos_incl < 0.3) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca;
                double ly = dx * sa * ba_ratio;
                double dist = fabs((double)dx) / (r_eff * 2.0);
                if (dist > 1.2) continue;
                double f = 0.4 * (1.0 - dist) * (0.3 - cos_incl) / 0.3;
                if (f < 0.01) continue;
                int px = SEMI + (int)(lx + 0.5);
                int py = SEMI + (int)(ly + 0.5);
                if (px < 0 || px >= FEN || py < 0 || py >= FEN) continue;
                /* obscurare pixeles existentes */
                int idx = (py * FEN + px) * 4;
                fen[idx + 0] = (unsigned char)(fen[idx + 0] * (1.0 - f));
                fen[idx + 1] = (unsigned char)(fen[idx + 1] * (1.0 - f));
                fen[idx + 2] = (unsigned char)(fen[idx + 2] * (1.0 - f));
            }
        }

    } else if (morph == GALAXIA_LENTICULARIS) {
        /* nucleus prominens Sérsic n~3 */
        fen_punctum(fen, SEMI, SEMI, r_eff * 0.35, col_nuc, luciditas * 1.8);

        /* discus diffusus sine brachiis — exponentiale */
        for (int dy = -25; dy <= 25; dy++) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca + dy * sa;
                double ly = (-dx * sa + dy * ca) / (ba_ratio + 0.01);
                double r = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 3.0) continue;
                double disc = exp(-r / r_eff) * 0.5;
                double f = luciditas * disc;
                if (f < 0.002) continue;
                double t = r / (r_eff * 2.0); if (t > 1.0) t = 1.0;
                astra_color_t c;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, c, f);
            }
        }

    } else {
        /* GALAXIA_IRREGULARIS — nodi stochastici (Hunter & Elmegreen 2004).
         * distributio spatiosa non symmetrica, nodi luminosi
         * (regiones HII gigantes, 30 Doradus in LMC exemplum)
         * cum fundo diffuso debili. */
        double offset_x = (alea_f() - 0.5) * r_eff * 0.3;
        double offset_y = (alea_f() - 0.5) * r_eff * 0.3;

        /* fundus diffusus asymmetricus */
        for (int dy = -20; dy <= 20; dy++) {
            for (int dx = -20; dx <= 20; dx++) {
                double lx = dx - offset_x;
                double ly = dy - offset_y;
                double r = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 2.5) continue;
                double f = luciditas * 0.3 * exp(-r / r_eff);
                if (f < 0.002) continue;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, col_ext, f);
            }
        }

        /* nodi starburst — 3-8 regiones luminosae */
        int n_nodi = 3 + (int)(alea_f() * 6);
        for (int k = 0; k < n_nodi; k++) {
            double nx = (alea_f() - 0.5) * r_eff * 2.0;
            double ny = (alea_f() - 0.5) * r_eff * 2.0;
            double nr = 0.5 + alea_f() * 1.5;
            double nf = luciditas * (0.3 + alea_f() * 0.5);
            /* colores: caerulei (formatio stellarum) vel rosei (Hα) */
            astra_color_t nc;
            if (alea_f() < 0.6)
                nc = (astra_color_t){0.5, 0.7, 1.0, 1.0};  /* OB stellae */
            else
                nc = (astra_color_t){1.0, 0.5, 0.6, 1.0};  /* HII Hα */
            fen_punctum(fen, SEMI + nx, SEMI + ny, nr, nc, nf);
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
    case SIDUS_GALAXIA:
        reddere_galaxia(fenestra, sidus, instrumentum);
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
    if (strcmp(nomen, "galaxia") == 0)      return SIDUS_GALAXIA;
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

    astra_instrumentum_t instr = {.saturatio = 1.0};
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

/* ================================================================
 * post-processatio — effectus globales in campum completum
 *
 * Ordo physice motivatus:
 *   1. visio (blur atmosphaericus — ante omnia optica)
 *   2. scintillatio (variatio intensitatis atmosphaerica)
 *   3. caeli_lumen (pollutio luminosa additiva)
 *   4. florescentia (bloom opticum)
 *   5. acuitas (acuificatio / lenitio)
 *   6. aberratio (aberratio chromatica)
 *   7. distorsio (lens gravitationalis — deformatio toroidalis)
 *   8. saturatio (processio coloris)
 *   9. vignetta (obscuratio marginalis)
 *  10. fenestra (maschera superelliptica)
 * ================================================================ */

/* blur Gaussianus separabilis — duae transitus (horiz + vert).
 * Kolmogorov (1941): spectrum turbulentiae E(k) ∝ k^{-5/3}
 * dat PSF fere Gaussianam in "long exposure" (Roddier 1981). */
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
static void isonl_post_processare(astra_campus_t *c,
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

    astra_instrumentum_t inst;
    memset(&inst, 0, sizeof(inst));
    inst.saturatio    = isonl_lege_f(instr_ison, "saturatio", 1.0);
    inst.aberratio    = isonl_lege_f(instr_ison, "aberratio", 0.0);
    inst.visio        = isonl_lege_f(instr_ison, "visio", 0.0);
    inst.scintillatio = isonl_lege_f(instr_ison, "scintillatio", 0.0);
    inst.caeli_lumen  = isonl_lege_f(instr_ison, "caeli_lumen", 0.0);
    inst.florescentia = isonl_lege_f(instr_ison, "florescentia", 0.0);
    inst.acuitas      = isonl_lege_f(instr_ison, "acuitas", 0.0);
    inst.refractio    = isonl_lege_f(instr_ison, "refractio", 0.0);
    inst.vignetta     = isonl_lege_f(instr_ison, "vignetta", 0.0);
    inst.distorsio    = isonl_lege_f(instr_ison, "distorsio", 0.0);
    inst.fenestra     = isonl_lege_f(instr_ison, "fenestra", 0.0);
    free(instr_ison);

    ison_pro_quaque_linea(isonl, isonl_linea_reddere, &ctx);
    free(isonl);

    if (!ctx.campus) {
        fprintf(stderr, "astra: _meta linea non inventa in %s\n", via_isonl);
        return NULL;
    }

    isonl_galaxiam_reddere(ctx.campus, &ctx);
    isonl_post_processare(ctx.campus, &inst);

    return ctx.campus;
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
