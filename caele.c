/*
 * caele.c — campum stellarum ex ISON generare
 *
 * Legit parametros campi ex plica ISON (caelae/nomen.ison),
 * generat stellas, emittit ISONL ad stdout.
 * Solum proprietates intrinsecae siderum emittuntur —
 * instrumentum opticum a redditore applicatur.
 *
 * Usus: ./caele caelae/terra.ison > caelae/terra.isonl
 */

#include "astra.h"
#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static unsigned int semen_loc = 1;
static unsigned int alea(void)
{
    semen_loc ^= semen_loc << 13;
    semen_loc ^= semen_loc >> 17;
    semen_loc ^= semen_loc << 5;
    return semen_loc;
}
static double alea_f(void)
{
    return (double)(alea() & 0xFFFFFF) / (double)0x1000000;
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

static long lege_n(const char *ison, const char *via, long praef)
{
    long n = ison_da_numerum(ison, via);
    return (n != 0) ? n : praef;
}

static void sidus_emittere(const char *genus, double mag, double temp,
                           int x, int y, double phase, double ang_phase)
{
    ison_scriptor_t *js = ison_scriptor_crea();
    ison_scriptor_adde(js, "genus", genus);

    char buf[64];
    snprintf(buf, sizeof(buf), "%.3f", mag);
    ison_scriptor_adde_crudum(js, "magnitudo", buf);
    snprintf(buf, sizeof(buf), "%.1f", temp);
    ison_scriptor_adde_crudum(js, "temperatura", buf);
    snprintf(buf, sizeof(buf), "%d", x);
    ison_scriptor_adde_crudum(js, "x", buf);
    snprintf(buf, sizeof(buf), "%d", y);
    ison_scriptor_adde_crudum(js, "y", buf);

    if (phase > 0.001) {
        snprintf(buf, sizeof(buf), "%.3f", phase);
        ison_scriptor_adde_crudum(js, "phase", buf);
        snprintf(buf, sizeof(buf), "%.3f", ang_phase);
        ison_scriptor_adde_crudum(js, "angulus_phase", buf);
    }

    char *linea = ison_scriptor_fini(js);
    puts(linea);
    free(linea);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usus: caele <campus.ison>\n");
        return 1;
    }

    char *ison = ison_lege_plicam(argv[1]);
    if (!ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[1]);
        return 1;
    }

    int latitudo        = (int)lege_n(ison, "latitudo", 1024);
    int altitudo        = (int)lege_n(ison, "altitudo", 512);
    int num_stellarum   = (int)lege_n(ison, "numerus_stellarum", 10000);
    double dens_gal     = lege_f(ison, "densitas_galaxiae", 0.5);
    double inc_gal      = lege_f(ison, "inclinatio_galaxiae", 0.0);
    double lat_gal      = lege_f(ison, "latitudo_galaxiae", 0.2);
    unsigned int semen  = (unsigned int)lege_n(ison, "semen", 1);
    int max_sg          = (int)lege_n(ison, "max_supergigantes", 0);
    int max_gi          = (int)lege_n(ison, "max_gigantes", 0);
    int max_ex          = (int)lege_n(ison, "max_exotica", 0);
    int num_planet      = (int)lege_n(ison, "numerus_planetarum", 0);
    double pl_tmin      = lege_f(ison, "planetae_temp_min", 4000);
    double pl_tmax      = lege_f(ison, "planetae_temp_max", 6000);
    int num_galaxiarum  = (int)lege_n(ison, "numerus_galaxiarum", 0);
    int max_galaxiae    = (int)lege_n(ison, "max_galaxiae", 0);

    /* meta linea — parametri campi pro redditore */
    {
        ison_scriptor_t *js = ison_scriptor_crea();
        char buf[64];
        ison_scriptor_adde(js, "genus", "_meta");
        snprintf(buf, sizeof(buf), "%d", latitudo);
        ison_scriptor_adde_crudum(js, "latitudo", buf);
        snprintf(buf, sizeof(buf), "%d", altitudo);
        ison_scriptor_adde_crudum(js, "altitudo", buf);
        snprintf(buf, sizeof(buf), "%.4f", lege_f(ison, "galaxia_glow", 0));
        ison_scriptor_adde_crudum(js, "galaxia_glow", buf);
        snprintf(buf, sizeof(buf), "%.4f", lege_f(ison, "galaxia_rift", 0));
        ison_scriptor_adde_crudum(js, "galaxia_rift", buf);
        snprintf(buf, sizeof(buf), "%d", (int)lege_n(ison, "galaxia_nebulae", 0));
        ison_scriptor_adde_crudum(js, "galaxia_nebulae", buf);
        snprintf(buf, sizeof(buf), "%.4f", inc_gal);
        ison_scriptor_adde_crudum(js, "inclinatio_galaxiae", buf);
        char *linea = ison_scriptor_fini(js);
        puts(linea);
        free(linea);
    }

    free(ison);
    semen_loc = semen;

    int n_gi = 0, n_sg = 0, n_ex = 0;

    for (int i = 0; i < num_stellarum; i++) {
        double fx = alea_f() * latitudo;
        double fy = alea_f() * altitudo;

        /* distantia a fascia galatctica.
         * fascia est sinusoida in toro: y_fascia(x) = 0.5 + A*sin(2π*x/L)
         * ubi A = inclinatio/2. haec curva toroidaliter clauditur
         * quia sin(0) = sin(2π). */
        double tx = fx / latitudo;  /* [0,1) */
        double ty = fy / altitudo;  /* [0,1) */
        double y_fascia = 0.5 + (inc_gal / 3.0) * sin(DUO_PI * tx);
        double dy = ty - y_fascia;
        /* toroidale: minima distantia */
        if (dy > 0.5) dy -= 1.0;
        if (dy < -0.5) dy += 1.0;
        double dist_gal = fabs(dy) / (lat_gal + 0.001);

        /* probabilitas acceptandi: in fascia ~1.0, extra fasciam ~(1-dens).
         * exp(-d²*8) dat fasciam angustam cum transitu leni. */
        if (alea_f() > dens_gal * exp(-dist_gal * dist_gal * 8.0)
            + (1.0 - dens_gal))
            continue;

        double r_mag = alea_f();
        double mag = 6.0 - 6.0 * r_mag * r_mag * r_mag * r_mag;

        const char *genus = "sequentia";
        int genus_id = 1;
        double gr = alea_f();

        if (gr < 0.06)         { genus = "nanum_album"; genus_id = 0; }
        else if (gr < 0.96)    { genus = "sequentia";   genus_id = 1; }
        else if (gr < 0.98)    { genus = "gigas_rubrum"; genus_id = 2; }
        else if (gr < 0.985)   { genus = "supergigas";  genus_id = 3; }
        else if (gr < 0.992)   { genus = "neutronium";  genus_id = 4; }
        else if (gr < 0.9995)  { genus = "sequentia";   genus_id = 1; }
        else if (gr < 0.99975) { genus = "crystallinum"; genus_id = 5; }
        else                    { genus = "magnetar";    genus_id = 6; }

        if (genus_id == 2 && max_gi > 0 && n_gi >= max_gi)
            { genus = "sequentia"; genus_id = 1; }
        if (genus_id == 3 && max_sg > 0 && n_sg >= max_sg)
            { genus = "sequentia"; genus_id = 1; }
        if ((genus_id >= 4 && genus_id <= 6) && max_ex > 0 && n_ex >= max_ex)
            { genus = "sequentia"; genus_id = 1; }

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

        switch (genus_id) {
        case 0: temp = 4000 + alea_f() * 30000; mag = 4.0 + alea_f() * 2.0; break;
        case 2: temp = 2500 + alea_f() * 2500; mag = 1.5 + alea_f() * 2.5; n_gi++; break;
        case 3: temp = 3000 + alea_f() * 25000; mag = 0.5 + alea_f() * 1.5; n_sg++; break;
        case 4: temp = 500000; mag = 3.0 + alea_f() * 3.0; n_ex++; break;
        case 5: temp = 6000 + alea_f() * 10000; mag = 2.0 + alea_f() * 3.0; n_ex++; break;
        case 6: temp = 5000000; mag = 1.0 + alea_f() * 2.0; n_ex++; break;
        default: break;
        }

        sidus_emittere(genus, mag, temp, (int)fx, (int)fy, 0, 0);
    }

    /* --- galaxiae distantes --- */
    {
        int n_gal = 0;
        for (int i = 0; i < num_galaxiarum; i++) {
            if (max_galaxiae > 0 && n_gal >= max_galaxiae) break;

            double r_mag = alea_f();
            double mag = 3.0 + 3.0 * (1.0 - pow(r_mag, 0.4));

            const char *morph_s;
            int morph_id;
            double mr = alea_f();
            if (mr < 0.15)       { morph_s = "elliptica";  morph_id = 0; }
            else if (mr < 0.50)  { morph_s = "spiralis";   morph_id = 1; }
            else if (mr < 0.75)  { morph_s = "barrata";    morph_id = 2; }
            else if (mr < 0.90)  { morph_s = "lenticularis"; morph_id = 3; }
            else                 { morph_s = "irregularis"; morph_id = 4; }

            double cos_incl = alea_f();
            double temp_code = cos_incl * 10000.0;
            double ang = alea_f() * DUO_PI;
            int px = (int)(alea_f() * latitudo);
            int py = (int)(alea_f() * altitudo);

            /* emittere galaxiam cum proprietatibus additis */
            {
                ison_scriptor_t *js = ison_scriptor_crea();
                char buf[64];
                ison_scriptor_adde(js, "genus", "galaxia");
                snprintf(buf, sizeof(buf), "%.3f", mag);
                ison_scriptor_adde_crudum(js, "magnitudo", buf);
                snprintf(buf, sizeof(buf), "%.1f", temp_code);
                ison_scriptor_adde_crudum(js, "temperatura", buf);
                snprintf(buf, sizeof(buf), "%d", px);
                ison_scriptor_adde_crudum(js, "x", buf);
                snprintf(buf, sizeof(buf), "%d", py);
                ison_scriptor_adde_crudum(js, "y", buf);
                snprintf(buf, sizeof(buf), "%d", morph_id);
                ison_scriptor_adde_crudum(js, "phase", buf);
                snprintf(buf, sizeof(buf), "%.3f", ang);
                ison_scriptor_adde_crudum(js, "angulus_phase", buf);
                ison_scriptor_adde(js, "morphologia", morph_s);
                char *linea = ison_scriptor_fini(js);
                puts(linea);
                free(linea);
            }
            n_gal++;
        }
    }

    for (int i = 0; i < num_planet; i++) {
        double temp = pl_tmin + alea_f() * (pl_tmax - pl_tmin);
        double mag = 1.0 + alea_f() * 3.0;
        double phase = alea_f() * 0.45;
        double ang = alea_f() * DUO_PI;
        int px = (int)(alea_f() * latitudo);
        int py = (int)(alea_f() * altitudo);

        sidus_emittere("planeta", mag, temp, px, py, phase, ang);
    }

    return 0;
}
