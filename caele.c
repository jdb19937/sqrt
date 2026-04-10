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

#include "instrumentum.h"
#include "tessera.h"
#include "campus.h"
#include "vectores.h"
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

/* ison_da_f / ison_da_n ex bibliotheca ison */

static int sidera_emissa = 0;

static void sidus_emittere(
    const char *genus, double mag, double temp,
    int x, int y, double phase, double ang_phase
) {
    if (sidera_emissa > 0)
        printf(",\n");
    printf(
        "    {\"x\": %d, \"y\": %d, \"sidus\": {\"sidulum\": {\"magnitudo\": %.3f, \"temperatura\": %.1f}",
        x, y, mag, temp
    );

    if (phase > 0.001) {
        if (strcmp(genus, "galaxia") == 0)
            printf(
                ", \"galaxiola\": {\"morphologia\": %d, \"angulus\": %.3f}",
                (int)phase, ang_phase
            );
        else if (strcmp(genus, "magnetar") == 0)
            printf(
                ", \"magnetarulum\": {\"phase\": %.3f, \"angulus\": %.3f}",
                phase, ang_phase
            );
        else
            printf(
                ", \"vaganulus\": {\"phase\": %.3f, \"angulus\": %.3f}",
                phase, ang_phase
            );
    }
    printf("}}");
    sidera_emissa++;
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

    int latitudo       = (int)ison_da_n(ison, "latitudo", 1024);
    int altitudo       = (int)ison_da_n(ison, "altitudo", 512);
    int num_stellarum  = (int)ison_da_n(ison, "numerus_stellarum", 10000);
    double dens_gal    = ison_da_f(ison, "densitas_galaxiae", 0.5);
    double inc_gal     = ison_da_f(ison, "inclinatio_galaxiae", 0.0);
    double lat_gal     = ison_da_f(ison, "latitudo_galaxiae", 0.2);
    unsigned int semen = (unsigned int)ison_da_n(ison, "semen", 1);
    int max_sg         = (int)ison_da_n(ison, "max_supergigantes", 0);
    int max_gi         = (int)ison_da_n(ison, "max_gigantes", 0);
    int max_ex         = (int)ison_da_n(ison, "max_exotica", 0);
    int num_planet     = (int)ison_da_n(ison, "numerus_planetarum", 0);
    double pl_tmin     = ison_da_f(ison, "planetae_temp_min", 4000);
    double pl_tmax     = ison_da_f(ison, "planetae_temp_max", 6000);
    int num_galaxiarum = (int)ison_da_n(ison, "numerus_galaxiarum", 0);
    int max_galaxiae   = (int)ison_da_n(ison, "max_galaxiae", 0);

    /* caput ISON */
    printf("{\n");
    printf("  \"latitudo\": %d,\n", latitudo);
    printf("  \"altitudo\": %d,\n", altitudo);
    printf("  \"galaxia_glow\": %.4f,\n", ison_da_f(ison, "galaxia_glow", 0.0));
    printf("  \"galaxia_rift\": %.4f,\n", ison_da_f(ison, "galaxia_rift", 0.0));
    printf("  \"galaxia_nebulae\": %d,\n", (int)ison_da_n(ison, "galaxia_nebulae", 0));
    printf("  \"inclinatio_galaxiae\": %.4f,\n", inc_gal);

    /* sidera */
    printf("  \"sidera\": [\n");

    semen_loc = semen;

    int n_gi = 0, n_sg = 0, n_ex = 0;

    for (int i = 0; i < num_stellarum; i++) {
        double fx = alea_f() * latitudo;
        double fy = alea_f() * altitudo;

        /* distantia a fascia galatctica.
         * fascia est sinusoida in toro: y_fascia(x) = 0.5 + A*sin(2π*x/L)
         * ubi A = inclinatio/2. haec curva toroidaliter clauditur
         * quia sin(0) = sin(2π). */
        double tx       = fx / latitudo;  /* [0,1) */
        double ty       = fy / altitudo;  /* [0,1) */
        double y_fascia = 0.5 + (inc_gal / 3.0) * sin(DUO_PI * tx);
        double dy       = ty - y_fascia;
        /* toroidale: minima distantia */
        if (dy > 0.5)
            dy -= 1.0;
        if (dy < -0.5)
            dy += 1.0;
        double dist_gal = fabs(dy) / (lat_gal + 0.001);

        /* probabilitas acceptandi: in fascia ~1.0, extra fasciam ~(1-dens).
         * exp(-d²*8) dat fasciam angustam cum transitu leni. */
        if (
            alea_f() > dens_gal * exp(-dist_gal * dist_gal * 8.0)
            + (1.0 - dens_gal)
        )
            continue;

        double r_mag = alea_f();
        double mag   = 6.0 - 6.0 * r_mag * r_mag * r_mag * r_mag;

        const char *genus = "sequentia";
        int genus_id      = 1;
        double gr         = alea_f();

        if (gr < 0.06)         {
            genus    = "nanum_album";
            genus_id = 0;
        }else if (gr < 0.96)    {
            genus    = "sequentia";
            genus_id = 1;
        }else if (gr < 0.98)    {
            genus    = "gigas_rubrum";
            genus_id = 2;
        }else if (gr < 0.985)   {
            genus    = "supergigas";
            genus_id = 3;
        }else if (gr < 0.992)   {
            genus    = "neutronium";
            genus_id = 4;
        }else if (gr < 0.9995)  {
            genus    = "sequentia";
            genus_id = 1;
        }else if (gr < 0.99975) {
            genus    = "crystallinum";
            genus_id = 5;
        }else                    {
            genus    = "magnetar";
            genus_id = 6;
        }

        if (genus_id == 2 && max_gi > 0 && n_gi >= max_gi) {
            genus    = "sequentia";
            genus_id = 1;
        }
        if (genus_id == 3 && max_sg > 0 && n_sg >= max_sg) {
            genus    = "sequentia";
            genus_id = 1;
        }
        if ((genus_id >= 4 && genus_id <= 6) && max_ex > 0 && n_ex >= max_ex) {
            genus    = "sequentia";
            genus_id = 1;
        }

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

        switch (genus_id) {
        case 0: temp = 4000 + alea_f() * 30000;
            mag = 4.0 + alea_f() * 2.0;
            break;
        case 2: temp = 2500 + alea_f() * 2500;
            mag = 1.5 + alea_f() * 2.5;
            n_gi++;
            break;
        case 3: temp = 3000 + alea_f() * 25000;
            mag = 0.5 + alea_f() * 1.5;
            n_sg++;
            break;
        case 4: temp = 500000;
            mag = 3.0 + alea_f() * 3.0;
            n_ex++;
            break;
        case 5: temp = 6000 + alea_f() * 10000;
            mag = 2.0 + alea_f() * 3.0;
            n_ex++;
            break;
        case 6: temp = 5000000;
            mag = 1.0 + alea_f() * 2.0;
            n_ex++;
            break;
        default: break;
        }

        sidus_emittere(genus, mag, temp, (int)fx, (int)fy, 0, 0);
    }

    /* --- galaxiae distantes --- */
    {
        int n_gal = 0;
        for (int i = 0; i < num_galaxiarum; i++) {
            if (max_galaxiae > 0 && n_gal >= max_galaxiae)
                break;

            double r_mag = alea_f();
            double mag   = 3.0 + 3.0 * (1.0 - pow(r_mag, 0.4));

            int morph_id;
            double mr = alea_f();
            if (mr < 0.15)
                morph_id = 0;
            else if (mr < 0.50)
                morph_id = 1;
            else if (mr < 0.75)
                morph_id = 2;
            else if (mr < 0.90)
                morph_id = 3;
            else
                morph_id = 4;

            double cos_incl = alea_f();
            double temp_code = cos_incl * 10000.0;
            double ang = alea_f() * DUO_PI;
            int px = (int)(alea_f() * latitudo);
            int py = (int)(alea_f() * altitudo);

            /* emittere galaxiam cum proprietatibus additis */
            sidus_emittere("galaxia", mag, temp_code, px, py, morph_id, ang);
            n_gal++;
        }
    }

    for (int i = 0; i < num_planet; i++) {
        double temp  = pl_tmin + alea_f() * (pl_tmax - pl_tmin);
        double mag   = 1.0 + alea_f() * 3.0;
        double phase = alea_f() * 0.45;
        double ang   = alea_f() * DUO_PI;
        int px       = (int)(alea_f() * latitudo);
        int py       = (int)(alea_f() * altitudo);

        sidus_emittere("vagans", mag, temp, px, py, phase, ang);
    }

    printf("\n  ],\n"); /* finis siderum */

    /* planetae — ex campus .ison transcripta */
    printf("  \"planetae\": [\n");
    {
        int n_pl = 0;
        for (int i = 0; ; i++) {
            char via[128];
            snprintf(via, sizeof(via), "planetae[%d]", i);
            char *raw = ison_da_crudum(ison, via);
            if (!raw)
                break;
            char *compacta = ison_compacta(raw);
            free(raw);
            if (!compacta)
                continue;
            if (n_pl > 0)
                printf(",\n");
            printf("    %s", compacta);
            free(compacta);
            n_pl++;
        }
    }
    printf("\n  ]\n");
    printf("}\n");

    free(ison);
    return 0;
}
