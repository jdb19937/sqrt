/*
 * redde.c — campum stellarum ex ISONL in PPM reddit
 *
 * Legit ISONL stellarum (ex caele) et instrumentum ISON,
 * applicat effectus opticos, reddit PPM.
 *
 * Usus: ./redde <stellae.isonl> <instrumentum.ison> [imago.ppm]
 *
 * Si imago.ppm omittitur, scribit ad stdout.
 */

#include "astra.h"
#include "ison/ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI_GRAECUM 3.14159265358979323846
#define FEN ASTRA_FENESTRA

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
    int    latitudo;
    int    altitudo;
    double galaxia_glow;
    double galaxia_rift;
    int    galaxia_nebulae;
    double inclinatio;
} meta_t;

static double par_f(const ison_par_t *pp, int n, const char *clavis, double praef)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return atof(pp[i].valor);
    return praef;
}

static long par_n(const ison_par_t *pp, int n, const char *clavis, long praef)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return atol(pp[i].valor);
    return praef;
}

static const char *par_s(const ison_par_t *pp, int n, const char *clavis)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return pp[i].valor;
    return "";
}

static double lege_f(const char *ison, const char *via, double praef)
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

/* galaxia glow/rift/nebulae */
static void galaxiam_reddere(astra_campus_t *c, const meta_t *m)
{
    if (m->galaxia_glow < 0.001) return;

    double inc = m->inclinatio;

    unsigned int sem = 7919;
    for (int y = 0; y < c->altitudo; y++) {
        for (int x = 0; x < c->latitudo; x++) {
            double tx = (double)x / c->latitudo;
            double ty = (double)y / c->altitudo;

            /* fascia sinusoidalis — toroidaliter clausa */
            double y_fascia = 0.5 + (inc / 3.0) * sin(ASTRA_DUO_PI * tx);
            double d = ty - y_fascia;
            if (d > 0.5) d -= 1.0;
            if (d < -0.5) d += 1.0;

            double band = exp(-d * d / (0.06 * 0.06));

            /* nucleus lucidior circa centrum galacticum */
            double dx_nuc = tx - 0.65;
            if (dx_nuc > 0.5) dx_nuc -= 1.0;
            if (dx_nuc < -0.5) dx_nuc += 1.0;
            double nucleus = exp(-(dx_nuc * dx_nuc * 4.0 + d * d) / (0.04 * 0.04));
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
                double y_fascia = 0.5 + (inc / 3.0) * sin(ASTRA_DUO_PI * tx);
                double d = ty - y_fascia;
                if (d > 0.5) d -= 1.0;
                if (d < -0.5) d += 1.0;
                if (fabs(d) > 0.08) continue;
                double along = tx;

                sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
                double rumore = (double)(sem & 0xFF) / 255.0;

                double rift = 0.0;
                rift += 0.5 * sin(along * 25.0 + 1.3) * exp(-d * d / (0.015 * 0.015));
                rift += 0.3 * sin(along * 40.0 + 2.7) * exp(-d * d / (0.010 * 0.010));
                rift += 0.2 * rumore * exp(-d * d / (0.02 * 0.02));
                if (rift < 0.15) continue;
                double obscura = rift * 0.5 * m->galaxia_rift;
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

    sem = 1571;
    for (int i = 0; i < m->galaxia_nebulae; i++) {
        sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
        double nx = (double)(sem & 0xFFFFF) / (double)0x100000;
        sem ^= sem << 13; sem ^= sem >> 17; sem ^= sem << 5;
        double ny = (double)(sem & 0xFFFFF) / (double)0x100000;
        double gx = nx * c->latitudo;
        double y_neb = 0.5 + (inc / 3.0) * sin(ASTRA_DUO_PI * nx);
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

/* contextus */
typedef struct {
    astra_campus_t *campus;
    meta_t meta;
    int meta_lecta;
    int stellae_redditae;
    /* instrumentum (applicatur ad sidera lucida) */
    int    i_spic;
    double i_spic_long;
    double i_spic_ang;
    double i_halo_r;
    double i_halo_v;
} ctx_t;

static void linea_reddere(const ison_par_t *pp, int n, void *ctx_v)
{
    ctx_t *ctx = (ctx_t *)ctx_v;
    const char *genus_s = par_s(pp, n, "genus");

    if (strcmp(genus_s, "_meta") == 0) {
        ctx->meta.latitudo        = (int)par_n(pp, n, "latitudo", 1024);
        ctx->meta.altitudo        = (int)par_n(pp, n, "altitudo", 512);
        ctx->meta.galaxia_glow    = par_f(pp, n, "galaxia_glow", 0);
        ctx->meta.galaxia_rift    = par_f(pp, n, "galaxia_rift", 0);
        ctx->meta.galaxia_nebulae = (int)par_n(pp, n, "galaxia_nebulae", 0);
        ctx->meta.inclinatio      = par_f(pp, n, "inclinatio_galaxiae", 0);

        ctx->campus = astra_campum_creare(ctx->meta.latitudo, ctx->meta.altitudo);
        ctx->meta_lecta = 1;
        return;
    }

    if (!ctx->meta_lecta || !ctx->campus) return;

    astra_genus_t genus = genus_ex_nomine(genus_s);
    double mag   = par_f(pp, n, "magnitudo", 5.0);
    double temp  = par_f(pp, n, "temperatura", 5000);
    int    x     = (int)par_n(pp, n, "x", 0);
    int    y     = (int)par_n(pp, n, "y", 0);
    double phase = par_f(pp, n, "phase", 0);
    double ang   = par_f(pp, n, "angulus_phase", 0);

    astra_sidus_t sidus = {genus, mag, temp, phase, ang};

    /* instrumentum applicare secundum magnitudinem.
     * saturatio et aberratio applicantur post-processu, non hic. */
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

    ctx->stellae_redditae++;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usus: redde <stellae.isonl> <instrumentum.ison> [imago.ppm]\n");
        return 1;
    }

    /* instrumentum legere */
    char *instr_ison = ison_lege_plicam(argv[2]);
    if (!instr_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[2]);
        return 1;
    }

    ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.i_spic      = (int)lege_f(instr_ison, "spiculae", 0);
    ctx.i_spic_long = lege_f(instr_ison, "spiculae_long", 0);
    ctx.i_spic_ang  = lege_f(instr_ison, "spiculae_ang", 0);
    ctx.i_halo_r    = lege_f(instr_ison, "halo_radius", 0);
    ctx.i_halo_v    = lege_f(instr_ison, "halo_vis", 0);
    double saturatio = lege_f(instr_ison, "saturatio", 1.0);
    double aberratio = lege_f(instr_ison, "aberratio", 0.0);
    free(instr_ison);

    /* stellas legere */
    char *isonl = ison_lege_plicam(argv[1]);
    if (!isonl) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[1]);
        return 1;
    }

    fprintf(stderr, "Stellas reddens...\n");
    ison_pro_quaque_linea(isonl, linea_reddere, &ctx);

    if (!ctx.campus) {
        fprintf(stderr, "ERROR: _meta linea non inventa\n");
        free(isonl);
        return 1;
    }

    fprintf(stderr, "Stellae redditae: %d\n", ctx.stellae_redditae);
    galaxiam_reddere(ctx.campus, &ctx.meta);

    /* post-processus instrumenti: saturatio et aberratio chromatica */
    {
        int L = ctx.campus->latitudo;
        int A = ctx.campus->altitudo;
        size_t n_pix = (size_t)L * A;
        unsigned char *px = ctx.campus->pixels;

        /*
         * Aberratio chromatica: canales RGB dislocantur radialiter
         * a centro imaginis. Rubrum longius, caeruleum propius
         * (dispersio normalis: n_rubrum < n_caeruleum).
         * Effectus: fimbriae rubrae exteriores, caeruleae interiores.
         */
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

                    /* rubrum: dislocatum extra */
                    double shift_r = aberratio * dist / (L * 0.5);
                    int rx = ((int)(x + nx * shift_r) % L + L) % L;
                    int ry = ((int)(y + ny * shift_r) % A + A) % A;

                    /* caeruleum: dislocatum intra */
                    int bx = ((int)(x - nx * shift_r) % L + L) % L;
                    int by = ((int)(y - ny * shift_r) % A + A) % A;

                    int idx = (y * L + x) * 3;
                    px[idx + 0] = copia[(ry * L + rx) * 3 + 0]; /* R ex dislocato */
                    /* G manet */
                    px[idx + 2] = copia[(by * L + bx) * 3 + 2]; /* B ex dislocato */
                }
            }
            free(copia);
        }

        /*
         * Saturatio: colorem in spatium HSL convertere,
         * S multiplicare, reconvertere.
         *
         * Physice: CCD silicon quantum efficiency ~80% per totum
         * spectrum visibile vs oculus humanus ~5% in rubro extremo.
         * Ergo CCD differentias spectrales multo fortius videt.
         * Processio digitalis saturationem auget ut haec differentia
         * visibilis fiat in imagine finali.
         */
        if (saturatio > 1.001 || saturatio < 0.999) {
            for (size_t i = 0; i < n_pix; i++) {
                int idx = (int)i * 3;
                double r = px[idx + 0] / 255.0;
                double g = px[idx + 1] / 255.0;
                double b = px[idx + 2] / 255.0;

                /* luminantia (Rec. 709) */
                double lum = 0.2126 * r + 0.7152 * g + 0.0722 * b;

                /* saturare: distantiam a griseo multiplicare */
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

    FILE *f;
    const char *via;
    if (argc >= 4) {
        via = argv[3];
        f = fopen(via, "wb");
        if (!f) {
            fprintf(stderr, "ERROR: %s aperire non possum\n", via);
            free(isonl);
            return 1;
        }
    } else {
        via = "stdout";
        f = stdout;
    }

    fprintf(stderr, "Imaginem scribens: %s\n", via);
    fprintf(f, "P6\n%d %d\n255\n", ctx.campus->latitudo, ctx.campus->altitudo);
    fwrite(ctx.campus->pixels, 1,
           (size_t)ctx.campus->latitudo * ctx.campus->altitudo * 3, f);

    if (f != stdout) fclose(f);

    astra_campum_destruere(ctx.campus);
    free(isonl);

    fprintf(stderr, "Opus perfectum est.\n");
    return 0;
}
