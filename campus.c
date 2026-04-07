/*
 * campus.c — campus stellarum, generatio et compositio
 */

#include "campus.h"
#include "vectores.h"
#include "tessera.h"
#include "instrumentum.h"
#include "aspectus.h"
#include "perceptus.h"
#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEN SIDUS_FENESTRA
#define SEMI (FEN / 2)

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
    /* topologia toroidalis */
    x       = ((x % c->latitudo) + c->latitudo) % c->latitudo;
    y       = ((y % c->altitudo) + c->altitudo) % c->altitudo;
    int idx = (y * c->latitudo + x) * 3;

    /* additiva saturans */
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

            /* topologia toroidalis */
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
    /* inspicit pixeles in cruce circa centrum */
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
            /* alpha blending: planeta tegit fundum (non additiva) */
            c->pixels[ci + 0] = (unsigned char)(fenestra[fi + 0] * af + c->pixels[ci + 0] * (1.0 - af));
            c->pixels[ci + 1] = (unsigned char)(fenestra[fi + 1] * af + c->pixels[ci + 1] * (1.0 - af));
            c->pixels[ci + 2] = (unsigned char)(fenestra[fi + 2] * af + c->pixels[ci + 2] * (1.0 - af));
        }
    }
}

void campus_generare(
    campus_t *c, const campus_parametri_t *p,
    const instrumentum_t *instrumentum
) {
    semen_g = p->semen;
    memset(c->pixels, 0, (size_t)c->latitudo * c->altitudo * 3);

    double cos_inc = cos(p->inclinatio_galaxiae);
    double sin_inc = sin(p->inclinatio_galaxiae);
    int n_gigantes = 0, n_supergigantes = 0, n_exotica = 0;

    /* --- stellae --- */
    for (int i = 0; i < p->numerus_stellarum; i++) {
        double fx = alea_f() * c->latitudo;
        double fy = alea_f() * c->altitudo;

        double tx       = fx / c->latitudo;
        double ty       = fy / c->altitudo;
        double y_fascia = 0.5 + (p->inclinatio_galaxiae / 3.0) * sin(DUO_PI * tx);
        double dy_g     = ty - y_fascia;
        if (dy_g > 0.5)
            dy_g -= 1.0;
        if (dy_g < -0.5)
            dy_g += 1.0;
        double dist_gal = fabs(dy_g) / (p->latitudo_galaxiae + 0.001);

        if (
            alea_f() > p->densitas_galaxiae * exp(-dist_gal * dist_gal * 8.0)
            + (1.0 - p->densitas_galaxiae)
        )
            continue;

        /* mag = 6 - 6r^4: pleraeque mag 5-6, paucissimae lucidae */
        double r_mag = alea_f();
        double mag   = 6.0 - 6.0 * r_mag * r_mag * r_mag * r_mag;

        /* genus ex IMF (Kroupa 2001) */
        sidereus_t genus;
        double gr = alea_f();
        if (gr < 0.06)
            genus = SIDUS_NANUM_ALBUM;
        else if (gr < 0.96)
            genus = SIDUS_SEQUENTIA;
        else if (gr < 0.98)
            genus = SIDUS_GIGAS_RUBRUM;
        else if (gr < 0.985)
            genus = SIDUS_SUPERGIGAS;
        else if (gr < 0.992)
            genus = SIDUS_NEUTRONIUM;
        else if (gr < 0.9995)
            genus = SIDUS_SEQUENTIA;
        else if (gr < 0.99975)
            genus = SIDUS_CRYSTALLINUM;
        else
            genus = SIDUS_MAGNETAR;

        /* limites per genus */
        if (
            genus == SIDUS_GIGAS_RUBRUM && p->max_gigantes > 0
            && n_gigantes >= p->max_gigantes
        )
            genus = SIDUS_SEQUENTIA;
        if (
            genus == SIDUS_SUPERGIGAS && p->max_supergigantes > 0
            && n_supergigantes >= p->max_supergigantes
        )
            genus = SIDUS_SEQUENTIA;
        if (
            (
                genus == SIDUS_NEUTRONIUM || genus == SIDUS_CRYSTALLINUM
                || genus == SIDUS_MAGNETAR
            ) && p->max_exotica > 0
            && n_exotica >= p->max_exotica
        )
            genus = SIDUS_SEQUENTIA;

        /* T_eff per classem spectralem (Gray & Corbally 2009) */
        double temp;
        {
            double tr = alea_f();
            if (tr < 0.40)
                temp = 2400 + alea_f() * 1300;
            else if (tr < 0.76)
                temp = 3700 + alea_f() * 1500;
            else if (tr < 0.88)
                temp = 5200 + alea_f() * 800;
            else if (tr < 0.95)
                temp = 6000 + alea_f() * 1500;
            else if (tr < 0.98)
                temp = 7500 + alea_f() * 2500;
            else
                temp = 10000 + alea_f() * 20000;
        }

        switch (genus) {
        case SIDUS_NANUM_ALBUM:
            temp = 4000 + alea_f() * 30000;
            mag  = 4.0 + alea_f() * 2.0;
            break;
        case SIDUS_GIGAS_RUBRUM:
            temp = 2500 + alea_f() * 2500;
            mag  = 1.5 + alea_f() * 2.5;
            n_gigantes++;
            break;
        case SIDUS_SUPERGIGAS:
            temp = 3000 + alea_f() * 25000;
            mag  = 0.5 + alea_f() * 1.5;
            n_supergigantes++;
            break;
        case SIDUS_NEUTRONIUM:
            temp = 500000;
            mag  = 3.0 + alea_f() * 3.0;
            n_exotica++;
            break;
        case SIDUS_CRYSTALLINUM:
            temp = 6000 + alea_f() * 10000;
            mag  = 2.0 + alea_f() * 3.0;
            n_exotica++;
            break;
        case SIDUS_MAGNETAR:
            temp = 5000000;
            mag  = 1.0 + alea_f() * 2.0;
            n_exotica++;
            break;
        default: break;
        }

        /* spatium minimum */
        int spatium = 1;
        if (mag < 5.0)
            spatium = 2;
        if (mag < 4.0)
            spatium = 4;
        if (mag < 2.5)
            spatium = 8;
        if (mag < 1.5)
            spatium = 14;

        if (!campus_regio_vacua(c, (int)fx, (int)fy, spatium))
            continue;

        sidus_t sidus = {0};
        sidus.qui = genus;
        sidus.ubi.sequentia.pro = (sidulum_t){mag, temp};

        /* instrumentum applicare — spiculae pro lucidis */
        instrumentum_t instr = {.saturatio = 1.0};
        if (mag < 1.5 && instrumentum && instrumentum->spiculae > 0) {
            double bright = 1.5 - mag;
            instr = *instrumentum;
            instr.spiculae_long = instrumentum->spiculae_long * bright + 2.0;
            instr.spiculae_ang += alea_f() * 0.1;
            instr.halo_radius = instrumentum->halo_radius * bright + 1.0;
            instr.halo_vis    = instrumentum->halo_vis * bright;
        } else if (mag < 2.5 && instrumentum && instrumentum->halo_radius > 0.01) {
            instr.halo_radius = instrumentum->halo_radius * (2.5 - mag) * 0.5;
            instr.halo_vis    = instrumentum->halo_vis * (2.5 - mag) * 0.5;
        }

        unsigned char fenestra[FEN * FEN * 4];
        sidus_reddere(fenestra, &sidus, &instr);
        sidus_in_campum(c, (int)fx, (int)fy, fenestra);
    }

    /* --- planetae --- */
    for (int i = 0; i < p->numerus_planetarum; i++) {
        double temp = p->planetae_temp_min
            + alea_f() * (p->planetae_temp_max - p->planetae_temp_min);
        double mag   = 1.0 + alea_f() * 3.0;
        double phase = alea_f() * 0.45;
        double ang   = alea_f() * DUO_PI;

        sidus_t sidus_vagans = {.qui = SIDUS_VAGANS, .ubi.vagans.pro = {mag, temp}, .ubi.vagans.res = {phase, ang}};
        instrumentum_t instr = {.saturatio = 1.0};

        unsigned char fenestra[FEN * FEN * 4];
        sidus_reddere(fenestra, &sidus_vagans, &instr);

        int px = (int)(alea_f() * c->latitudo);
        int py = (int)(alea_f() * c->altitudo);
        if (campus_regio_vacua(c, px, py, 10))
            sidus_in_campum(c, px, py, fenestra);
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
            if (p->max_galaxiae > 0 && n_gal >= p->max_galaxiae)
                break;

            /* magnitudo: Schechter — pleraeque debiles */
            double r_mag = alea_f();
            double mag   = 3.0 + 3.0 * (1.0 - pow(r_mag, 0.4));

            /* morphologia */
            galaxia_morphologia_t morph;
            double mr = alea_f();
            if (mr < 0.15)
                morph = GALAXIA_ELLIPTICA;
            else if (mr < 0.50)
                morph = GALAXIA_SPIRALIS;
            else if (mr < 0.75)
                morph = GALAXIA_SPIRALIS_BARRATA;
            else if (mr < 0.90)
                morph = GALAXIA_LENTICULARIS;
            else
                morph = GALAXIA_IRREGULARIS;

            /* inclinatio: cos(i) uniformis in [0,1] (orientationes
             * aleatoriae in spatio 3D). codificamus ut T/10000. */
            double cos_incl  = alea_f();
            double temp_code = cos_incl * 10000.0;

            double ang = alea_f() * DUO_PI;

            sidus_t sidus_gal = {.qui = SIDUS_GALAXIA, .ubi.galaxia.pro = {mag, temp_code}, .ubi.galaxia.res = {morph, ang}};
            instrumentum_t instr = {.saturatio = 1.0};

            unsigned char fenestra[FEN * FEN * 4];
            sidus_reddere(fenestra, &sidus_gal, &instr);

            int px      = (int)(alea_f() * c->latitudo);
            int py      = (int)(alea_f() * c->altitudo);
            int spatium = (mag < 4.0) ? 8 : (mag < 5.0) ? 4 : 2;
            if (campus_regio_vacua(c, px, py, spatium)) {
                sidus_in_campum(c, px, py, fenestra);
                n_gal++;
            }
        }
    }

    /* --- via lactea glow (supra stellas) --- */
    if (p->galaxia_glow > 0.001) {
        unsigned int sem = semen_g;
        for (int y = 0; y < c->altitudo; y++) {
            for (int x = 0; x < c->latitudo; x++) {
                double lrx  = (double)x / c->latitudo - 0.5;
                double lry  = (double)y / c->altitudo - 0.5;
                double d    = -lrx * sin_inc + lry * cos_inc;
                double band = exp(-d * d / (0.06 * 0.06));

                double dx_nuc  = lrx - 0.15;
                double nucleus = exp(-(dx_nuc * dx_nuc + d * d) / (0.04 * 0.04));
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

                double f = band * (0.04 + rumore * 0.03) * p->galaxia_glow;
                if (f < 0.003)
                    continue;

                unsigned char cr = (unsigned char)(f * 220);
                unsigned char cg = (unsigned char)(f * 200);
                unsigned char cb = (unsigned char)(f * 170);
                campus_pixel_scribere(c, x, y, cr, cg, cb);
            }
        }

        /* fasciae pulveris (Great Rift) */
        if (p->galaxia_rift > 0.001) {
            sem = semen_g + 4217;
            for (int y = 0; y < c->altitudo; y++) {
                for (int x = 0; x < c->latitudo; x++) {
                    double lrx = (double)x / c->latitudo - 0.5;
                    double lry = (double)y / c->altitudo - 0.5;
                    double d   = -lrx * sin_inc + lry * cos_inc;
                    if (fabs(d) > 0.08)
                        continue;
                    double along = lrx * cos_inc + lry * sin_inc;

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
                    double obscura = rift * 0.5 * p->galaxia_rift;
                    if (obscura > 0.8)
                        obscura = 0.8;

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
            sem ^= sem << 13;
            sem ^= sem >> 17;
            sem ^= sem << 5;
            double nx = (double)(sem & 0xFFFFF) / (double)0x100000;
            sem ^= sem << 13;
            sem ^= sem >> 17;
            sem ^= sem << 5;
            double ny = (double)(sem & 0xFFFFF) / (double)0x100000;
            double gx = nx * c->latitudo;
            double gy = c->altitudo * 0.5 + (nx - 0.5) * sin_inc * c->altitudo
                + (ny - 0.5) * 0.08 * c->altitudo;
            sem ^= sem << 13;
            sem ^= sem >> 17;
            sem ^= sem << 5;
            double radius = 8 + (double)(sem & 0x1F);
            unsigned char nr, ng, nb;
            if (i % 3 == 0) {
                nr = 40;
                ng = 15;
                nb = 15;
            }else if (i % 3 == 1) {
                nr = 12;
                ng = 30;
                nb = 20;
            }else {
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
                        (unsigned char)(nr * f), (unsigned char)(ng * f),
                        (unsigned char)(nb * f)
                    );
                }
            }
        }
    }
}

/* ================================================================
 * ISONL redditor — campum ex ISONL et instrumento reddit
 * ================================================================ */


static planeta_t *planeta_ex_pares_c(const ison_par_t *pp, int n)
{
    const char *g = ison_pares_s(pp, n, "genus");
    planetarius_t genus;
    if (strcmp(g, "gaseosum") == 0)
        genus = PLANETA_GASEOSUM;
    else if (strcmp(g, "glaciale") == 0)
        genus = PLANETA_GLACIALE;
    else if (strcmp(g, "parvum") == 0)
        genus = PLANETA_PARVUM;
    else if (strcmp(g, "sol") == 0)
        genus = PLANETA_SOL;
    else if (strcmp(g, "nebula") == 0)
        genus = PLANETA_NEBULA;
    else
        genus = PLANETA_SAXOSUM;

    planeta_t *v = calloc(1, sizeof(planeta_t));
    v->qui = genus;
    v->ubi.saxosum.pro.radius     = ison_pares_f(pp, n, "radius", 0.9);
    v->ubi.saxosum.pro.inclinatio = ison_pares_f(pp, n, "inclinatio", 0.0);
    v->ubi.saxosum.pro.rotatio    = ison_pares_f(pp, n, "rotatio", 0.0);
    v->ubi.saxosum.pro.semen      = (unsigned)ison_pares_n(pp, n, "semen", 42);

    switch (genus) {
    case PLANETA_SAXOSUM:
        v->ubi.saxosum.res.silicata         = ison_pares_f(pp, n, "silicata", 0.0);
        v->ubi.saxosum.res.ferrum           = ison_pares_f(pp, n, "ferrum", 0.0);
        v->ubi.saxosum.res.sulphur          = ison_pares_f(pp, n, "sulphur", 0.0);
        v->ubi.saxosum.res.carbo            = ison_pares_f(pp, n, "carbo", 0.0);
        v->ubi.saxosum.res.glacies          = ison_pares_f(pp, n, "glacies", 0.0);
        v->ubi.saxosum.res.glacies_co2      = ison_pares_f(pp, n, "glacies_co2", 0.0);
        v->ubi.saxosum.res.malachita        = ison_pares_f(pp, n, "malachita", 0.0);
        v->ubi.saxosum.res.aqua             = ison_pares_f(pp, n, "aqua", 0.0);
        v->ubi.saxosum.res.aqua_profunditas = ison_pares_f(pp, n, "aqua_profunditas", 0.5);
        v->ubi.saxosum.res.continentes      = (int)ison_pares_n(pp, n, "continentes", 0);
        v->ubi.saxosum.res.scala            = ison_pares_f(pp, n, "scala_featurae", 1.0);
        v->ubi.saxosum.res.tectonica        = ison_pares_f(pp, n, "tectonica", 0.3);
        v->ubi.saxosum.res.craterae         = ison_pares_f(pp, n, "craterae", 0.0);
        v->ubi.saxosum.res.maria            = ison_pares_f(pp, n, "maria", 0.0);
        v->ubi.saxosum.res.vulcanismus      = ison_pares_f(pp, n, "vulcanismus", 0.0);
        v->ubi.saxosum.res.pressio_kPa      = ison_pares_f(pp, n, "pressio_kPa", 0.0);
        v->ubi.saxosum.res.n2               = ison_pares_f(pp, n, "n2", 0.0);
        v->ubi.saxosum.res.o2               = ison_pares_f(pp, n, "o2", 0.0);
        v->ubi.saxosum.res.co2              = ison_pares_f(pp, n, "co2", 0.0);
        v->ubi.saxosum.res.ch4              = ison_pares_f(pp, n, "ch4", 0.0);
        v->ubi.saxosum.res.h2               = ison_pares_f(pp, n, "h2", 0.0);
        v->ubi.saxosum.res.he               = ison_pares_f(pp, n, "he", 0.0);
        v->ubi.saxosum.res.nh3              = ison_pares_f(pp, n, "nh3", 0.0);
        v->ubi.saxosum.res.pulvis           = ison_pares_f(pp, n, "pulvis", 0.0);
        v->ubi.saxosum.res.nubes            = ison_pares_f(pp, n, "nubes", 0.0);
        v->ubi.saxosum.res.polaris          = ison_pares_f(pp, n, "polaris", 0.0);
        break;
    case PLANETA_GASEOSUM:
        v->ubi.gaseosum.res.n2               = ison_pares_f(pp, n, "n2", 0.0);
        v->ubi.gaseosum.res.o2               = ison_pares_f(pp, n, "o2", 0.0);
        v->ubi.gaseosum.res.co2              = ison_pares_f(pp, n, "co2", 0.0);
        v->ubi.gaseosum.res.ch4              = ison_pares_f(pp, n, "ch4", 0.0);
        v->ubi.gaseosum.res.h2               = ison_pares_f(pp, n, "h2", 0.0);
        v->ubi.gaseosum.res.he               = ison_pares_f(pp, n, "he", 0.0);
        v->ubi.gaseosum.res.nh3              = ison_pares_f(pp, n, "nh3", 0.0);
        v->ubi.gaseosum.res.pulvis           = ison_pares_f(pp, n, "pulvis", 0.0);
        v->ubi.gaseosum.res.fasciae          = (int)ison_pares_n(pp, n, "fasciae", 0);
        v->ubi.gaseosum.res.fasciae_contrast = ison_pares_f(pp, n, "fasciae_contrast", 0.5);
        v->ubi.gaseosum.res.maculae          = (int)ison_pares_n(pp, n, "maculae", 0);
        v->ubi.gaseosum.res.macula_lat       = ison_pares_f(pp, n, "macula_lat", 0.0);
        v->ubi.gaseosum.res.macula_lon       = ison_pares_f(pp, n, "macula_lon", 0.0);
        v->ubi.gaseosum.res.macula_radius    = ison_pares_f(pp, n, "macula_radius", 0.1);
        v->ubi.gaseosum.res.macula_obscuritas = ison_pares_f(pp, n, "macula_obscuritas", 0.5);
        v->ubi.gaseosum.res.fusio            = ison_pares_f(pp, n, "fusio", 0.0);
        v->ubi.gaseosum.res.temperatura      = ison_pares_f(pp, n, "temperatura", 0.0);
        v->ubi.gaseosum.res.luminositas      = ison_pares_f(pp, n, "luminositas", 1.0);
        v->ubi.gaseosum.res.corona           = ison_pares_f(pp, n, "corona", 0.0);
        v->ubi.gaseosum.res.granulatio       = ison_pares_f(pp, n, "granulatio", 0.0);
        break;
    case PLANETA_GLACIALE:
        v->ubi.glaciale.res.n2               = ison_pares_f(pp, n, "n2", 0.0);
        v->ubi.glaciale.res.o2               = ison_pares_f(pp, n, "o2", 0.0);
        v->ubi.glaciale.res.co2              = ison_pares_f(pp, n, "co2", 0.0);
        v->ubi.glaciale.res.ch4              = ison_pares_f(pp, n, "ch4", 0.0);
        v->ubi.glaciale.res.h2               = ison_pares_f(pp, n, "h2", 0.0);
        v->ubi.glaciale.res.he               = ison_pares_f(pp, n, "he", 0.0);
        v->ubi.glaciale.res.nh3              = ison_pares_f(pp, n, "nh3", 0.0);
        v->ubi.glaciale.res.pulvis           = ison_pares_f(pp, n, "pulvis", 0.0);
        v->ubi.glaciale.res.fasciae          = (int)ison_pares_n(pp, n, "fasciae", 0);
        v->ubi.glaciale.res.fasciae_contrast = ison_pares_f(pp, n, "fasciae_contrast", 0.5);
        v->ubi.glaciale.res.maculae          = (int)ison_pares_n(pp, n, "maculae", 0);
        v->ubi.glaciale.res.macula_lat       = ison_pares_f(pp, n, "macula_lat", 0.0);
        v->ubi.glaciale.res.macula_lon       = ison_pares_f(pp, n, "macula_lon", 0.0);
        v->ubi.glaciale.res.macula_radius    = ison_pares_f(pp, n, "macula_radius", 0.1);
        break;
    case PLANETA_PARVUM:
        v->ubi.parvum.res.silicata         = ison_pares_f(pp, n, "silicata", 0.0);
        v->ubi.parvum.res.ferrum           = ison_pares_f(pp, n, "ferrum", 0.0);
        v->ubi.parvum.res.sulphur          = ison_pares_f(pp, n, "sulphur", 0.0);
        v->ubi.parvum.res.carbo            = ison_pares_f(pp, n, "carbo", 0.0);
        v->ubi.parvum.res.glacies          = ison_pares_f(pp, n, "glacies", 0.0);
        v->ubi.parvum.res.glacies_co2      = ison_pares_f(pp, n, "glacies_co2", 0.0);
        v->ubi.parvum.res.malachita        = ison_pares_f(pp, n, "malachita", 0.0);
        v->ubi.parvum.res.aqua             = ison_pares_f(pp, n, "aqua", 0.0);
        v->ubi.parvum.res.aqua_profunditas = ison_pares_f(pp, n, "aqua_profunditas", 0.5);
        v->ubi.parvum.res.continentes      = (int)ison_pares_n(pp, n, "continentes", 0);
        v->ubi.parvum.res.scala            = ison_pares_f(pp, n, "scala_featurae", 1.0);
        v->ubi.parvum.res.tectonica        = ison_pares_f(pp, n, "tectonica", 0.3);
        v->ubi.parvum.res.craterae         = ison_pares_f(pp, n, "craterae", 0.0);
        v->ubi.parvum.res.maria            = ison_pares_f(pp, n, "maria", 0.0);
        v->ubi.parvum.res.vulcanismus      = ison_pares_f(pp, n, "vulcanismus", 0.0);
        v->ubi.parvum.res.pressio_kPa      = ison_pares_f(pp, n, "pressio_kPa", 0.0);
        v->ubi.parvum.res.n2               = ison_pares_f(pp, n, "n2", 0.0);
        v->ubi.parvum.res.o2               = ison_pares_f(pp, n, "o2", 0.0);
        v->ubi.parvum.res.co2              = ison_pares_f(pp, n, "co2", 0.0);
        v->ubi.parvum.res.ch4              = ison_pares_f(pp, n, "ch4", 0.0);
        v->ubi.parvum.res.h2               = ison_pares_f(pp, n, "h2", 0.0);
        v->ubi.parvum.res.he               = ison_pares_f(pp, n, "he", 0.0);
        v->ubi.parvum.res.nh3              = ison_pares_f(pp, n, "nh3", 0.0);
        v->ubi.parvum.res.pulvis           = ison_pares_f(pp, n, "pulvis", 0.0);
        v->ubi.parvum.res.nubes            = ison_pares_f(pp, n, "nubes", 0.0);
        v->ubi.parvum.res.polaris          = ison_pares_f(pp, n, "polaris", 0.0);
        break;
    case PLANETA_SOL:
        v->ubi.sol.res.fusio            = ison_pares_f(pp, n, "fusio", 1.0);
        v->ubi.sol.res.temperatura      = ison_pares_f(pp, n, "temperatura", 0.0);
        v->ubi.sol.res.luminositas      = ison_pares_f(pp, n, "luminositas", 1.0);
        v->ubi.sol.res.corona           = ison_pares_f(pp, n, "corona", 0.0);
        v->ubi.sol.res.granulatio       = ison_pares_f(pp, n, "granulatio", 0.0);
        v->ubi.sol.res.maculae          = (int)ison_pares_n(pp, n, "maculae", 0);
        v->ubi.sol.res.macula_radius    = ison_pares_f(pp, n, "macula_radius", 0.1);
        v->ubi.sol.res.macula_obscuritas = ison_pares_f(pp, n, "macula_obscuritas", 0.5);
        v->ubi.sol.res.h2               = ison_pares_f(pp, n, "h2", 0.0);
        v->ubi.sol.res.he               = ison_pares_f(pp, n, "he", 0.0);
        v->ubi.sol.res.ch4              = ison_pares_f(pp, n, "ch4", 0.0);
        v->ubi.sol.res.nh3              = ison_pares_f(pp, n, "nh3", 0.0);
        break;
    case PLANETA_NEBULA:
        v->ubi.nebula.res.temperatura      = ison_pares_f(pp, n, "temperatura", 0.0);
        v->ubi.nebula.res.luminositas      = ison_pares_f(pp, n, "luminositas", 1.0);
        v->ubi.nebula.res.h2               = ison_pares_f(pp, n, "h2", 0.0);
        v->ubi.nebula.res.o2               = ison_pares_f(pp, n, "o2", 0.0);
        v->ubi.nebula.res.carbo            = ison_pares_f(pp, n, "carbo", 0.0);
        v->ubi.nebula.res.tectonica        = ison_pares_f(pp, n, "tectonica", 0.5);
        v->ubi.nebula.res.nubes            = ison_pares_f(pp, n, "nubes", 0.4);
        break;
    default:
        break;
    }
    return v;
}

static sidereus_t genus_ex_nomine(const char *nomen)
{
    if (strcmp(nomen, "nanum_album") == 0)
        return SIDUS_NANUM_ALBUM;
    if (strcmp(nomen, "sequentia") == 0)
        return SIDUS_SEQUENTIA;
    if (strcmp(nomen, "gigas_rubrum") == 0)
        return SIDUS_GIGAS_RUBRUM;
    if (strcmp(nomen, "supergigas") == 0)
        return SIDUS_SUPERGIGAS;
    if (strcmp(nomen, "neutronium") == 0)
        return SIDUS_NEUTRONIUM;
    if (strcmp(nomen, "crystallinum") == 0)
        return SIDUS_CRYSTALLINUM;
    if (strcmp(nomen, "magnetar") == 0)
        return SIDUS_MAGNETAR;
    if (strcmp(nomen, "galaxia") == 0)
        return SIDUS_GALAXIA;
    if (strcmp(nomen, "vagans") == 0)
        return SIDUS_VAGANS;
    return SIDUS_SEQUENTIA;
}

typedef struct {
    campus_t *campus;
    double galaxia_glow, galaxia_rift, inclinatio;
    int    galaxia_nebulae;
    int    meta_lecta;
    /* instrumentum */
    int    i_spic;
    double i_spic_long, i_spic_ang;
    double i_halo_r, i_halo_v;
} isonl_ctx_t;

static void isonl_linea_reddere(const char *linea, void *ctx_v)
{
    isonl_ctx_t *ctx = (isonl_ctx_t *)ctx_v;
    char *internum;
    ison_par_t pp[64];
    int n;

    /* {"_meta": {...}} */
    internum = ison_da_crudum(linea, "_meta");
    if (internum) {
        n = ison_lege(internum, pp, 64);
        free(internum);
        if (n > 0) {
            int lat = (int)ison_pares_n(pp, n, "latitudo", 1024);
            int alt = (int)ison_pares_n(pp, n, "altitudo", 512);
            ctx->galaxia_glow    = ison_pares_f(pp, n, "galaxia_glow", 0);
            ctx->galaxia_rift    = ison_pares_f(pp, n, "galaxia_rift", 0);
            ctx->galaxia_nebulae = (int)ison_pares_n(pp, n, "galaxia_nebulae", 0);
            ctx->inclinatio      = ison_pares_f(pp, n, "inclinatio_galaxiae", 0);
            ctx->campus = campus_creare(lat, alt);
            ctx->meta_lecta = 1;
        }
        return;
    }

    if (!ctx->meta_lecta || !ctx->campus)
        return;

    /* {"planeta": {...}} */
    internum = ison_da_crudum(linea, "planeta");
    if (internum) {
        char *per_s = ison_da_crudum(internum, "perceptus");
        n = ison_lege(internum, pp, 64);
        free(internum);
        if (n > 0) {
            int x     = (int)ison_pares_n(pp, n, "x", 0);
            int y     = (int)ison_pares_n(pp, n, "y", 0);
            double sc = ison_pares_f(pp, n, "scala", 1.0);
            planeta_t *pl = planeta_ex_pares_c(pp, n);
            planeta_perceptus_t perc = planeta_perceptus_ex_ison(per_s);
            free(per_s);
            unsigned char *fen = (unsigned char *)calloc(
                (size_t)PLANETA_FENESTRA * PLANETA_FENESTRA * 4, 1
            );
            if (fen) {
                planeta_reddere(fen, pl, &perc);
                planeta_perceptum_applicare(fen, &perc);
                planeta_in_campum(ctx->campus, x, y, fen, sc);
                free(fen);
            }
            free(pl);
        } else {
            free(per_s);
        }
        return;
    }

    /* {"sidus": {...}} */
    internum = ison_da_crudum(linea, "sidus");
    if (!internum)
        return;
    n = ison_lege(internum, pp, 64);
    free(internum);
    if (n <= 0)
        return;

    int x = (int)ison_pares_n(pp, n, "x", 0);
    int y = (int)ison_pares_n(pp, n, "y", 0);

    sidereus_t genus = genus_ex_nomine(ison_pares_s(pp, n, "genus"));
    double mag       = ison_pares_f(pp, n, "magnitudo", 5.0);
    double temp      = ison_pares_f(pp, n, "temperatura", 5000);

    sidus_t sidus;
    memset(&sidus, 0, sizeof(sidus));
    sidus.qui = genus;
    if (genus == SIDUS_VAGANS) {
        sidus.ubi.vagans.pro = (sidulum_t){mag, temp};
        sidus.ubi.vagans.res.phase   = ison_pares_f(pp, n, "phase", 0);
        sidus.ubi.vagans.res.angulus = ison_pares_f(pp, n, "angulus_phase", 0);
    } else if (genus == SIDUS_GALAXIA) {
        sidus.ubi.galaxia.pro = (sidulum_t){mag, temp};
        sidus.ubi.galaxia.res.morphologia = (galaxia_morphologia_t)(int)ison_pares_f(pp, n, "morphologia", 0);
        sidus.ubi.galaxia.res.angulus     = ison_pares_f(pp, n, "angulus_phase", 0);
    } else if (genus == SIDUS_MAGNETAR) {
        sidus.ubi.magnetar.pro = (sidulum_t){mag, temp};
        sidus.ubi.magnetar.res.phase = ison_pares_f(pp, n, "phase", 0);
    } else {
        sidus.ubi.sequentia.pro = (sidulum_t){mag, temp};
    }

    instrumentum_t instr = {.saturatio = 1.0};
    if (mag < 1.5 && ctx->i_spic > 0) {
        double bright       = 1.5 - mag;
        instr.spiculae      = ctx->i_spic;
        instr.spiculae_long = ctx->i_spic_long * bright + 2.0;
        instr.spiculae_ang  = ctx->i_spic_ang;
        instr.halo_radius   = ctx->i_halo_r * bright + 1.0;
        instr.halo_vis      = ctx->i_halo_v * bright;
    } else if (mag < 2.5 && ctx->i_halo_r > 0.01) {
        instr.halo_radius = ctx->i_halo_r * (2.5 - mag) * 0.5;
        instr.halo_vis    = ctx->i_halo_v * (2.5 - mag) * 0.5;
    }

    unsigned char fen[FEN * FEN * 4];
    sidus_reddere(fen, &sidus, &instr);
    sidus_in_campum(ctx->campus, x, y, fen);
}

static void isonl_galaxiam_reddere(campus_t *c, const isonl_ctx_t *m)
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
            double nucleus = exp(
                -(dx_nuc * dx_nuc * 4.0 + d * d)
                / (0.04 * 0.04)
            );
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
                (unsigned char)(f * 220),
                (unsigned char)(f * 200),
                (unsigned char)(f * 170)
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
                rift += 0.5 * sin(along * 25.0 + 1.3)
                * exp(-d * d / (0.015 * 0.015));
                rift += 0.3 * sin(along * 40.0 + 2.7)
                * exp(-d * d / (0.010 * 0.010));
                rift += 0.2 * rumore * exp(-d * d / (0.02 * 0.02));
                if (rift < 0.15)
                    continue;
                double obscura = rift * 0.5 * m->galaxia_rift;
                if (obscura > 0.8)
                    obscura = 0.8;

                int px  = ((x % c->latitudo) + c->latitudo) % c->latitudo;
                int py  = ((y % c->altitudo) + c->altitudo) % c->altitudo;
                int idx = (py * c->latitudo + px) * 3;
                c->pixels[idx + 0] = (unsigned char)(
                    c->pixels[idx + 0]
                    * (1.0 - obscura)
                );
                c->pixels[idx + 1] = (unsigned char)(
                    c->pixels[idx + 1]
                    * (1.0 - obscura)
                );
                c->pixels[idx + 2] = (unsigned char)(
                    c->pixels[idx + 2]
                    * (1.0 - obscura)
                );
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
        if (i % 3 == 0) {
            nr = 40;
            ng = 15;
            nb = 15;
        }else if (i % 3 == 1) {
            nr = 12;
            ng = 30;
            nb = 20;
        }else {
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
                    (unsigned char)(nr * f), (unsigned char)(ng * f),
                    (unsigned char)(nb * f)
                );
            }
        }
    }
}

campus_t *campus_ex_isonl_reddere(
    const char *via_isonl,
    const char *via_instrumentum
) {
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
    ctx.i_spic      = (int)ison_da_f(instr_ison, "spiculae", 0);
    ctx.i_spic_long = ison_da_f(instr_ison, "spiculae_long", 0);
    ctx.i_spic_ang  = ison_da_f(instr_ison, "spiculae_ang", 0);
    ctx.i_halo_r    = ison_da_f(instr_ison, "halo_radius", 0);
    ctx.i_halo_v    = ison_da_f(instr_ison, "halo_vis", 0);

    instrumentum_t inst;
    memset(&inst, 0, sizeof(inst));
    inst.saturatio    = ison_da_f(instr_ison, "saturatio", 1.0);
    inst.aberratio    = ison_da_f(instr_ison, "aberratio", 0.0);
    inst.visio        = ison_da_f(instr_ison, "visio", 0.0);
    inst.scintillatio = ison_da_f(instr_ison, "scintillatio", 0.0);
    inst.caeli_lumen  = ison_da_f(instr_ison, "caeli_lumen", 0.0);
    inst.florescentia = ison_da_f(instr_ison, "florescentia", 0.0);
    inst.acuitas      = ison_da_f(instr_ison, "acuitas", 0.0);
    inst.refractio    = ison_da_f(instr_ison, "refractio", 0.0);
    inst.vignetta     = ison_da_f(instr_ison, "vignetta", 0.0);
    inst.distorsio    = ison_da_f(instr_ison, "distorsio", 0.0);
    inst.fenestra     = ison_da_f(instr_ison, "fenestra", 0.0);
    free(instr_ison);

    ison_pro_quaque_linea_s(isonl, isonl_linea_reddere, &ctx);
    free(isonl);

    if (!ctx.campus) {
        fprintf(stderr, "astra: _meta linea non inventa in %s\n", via_isonl);
        return NULL;
    }

    isonl_galaxiam_reddere(ctx.campus, &ctx);
    isonl_post_processare(ctx.campus, &inst);

    return ctx.campus;
}
