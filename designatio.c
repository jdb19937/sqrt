/*
 * designatio.c — designatio generationis universi: lector, scriptor, generator
 */

#include "designatio.h"
#include <ison/ison.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * lector
 * ================================================================ */

void designatio_ex_ison(designatio_t *d, const char *ison)
{
    memset(d, 0, sizeof(*d));
    d->latitudo            = (int)ison_da_n(ison, "latitudo", 4096);
    d->altitudo            = (int)ison_da_n(ison, "altitudo", 4096);
    d->numerus_stellarum   = (int)ison_da_n(ison, "numerus_stellarum", 10000);
    d->densitas_galaxiae   = ison_da_f(ison, "densitas_galaxiae", 0.5);
    d->inclinatio_galaxiae = ison_da_f(ison, "inclinatio_galaxiae", 0.0);
    d->latitudo_galaxiae   = ison_da_f(ison, "latitudo_galaxiae", 0.2);
    d->semen               = (unsigned int)ison_da_n(ison, "semen", 1);
    d->galaxia_glow        = ison_da_f(ison, "galaxia_glow", 0.0);
    d->galaxia_rift        = ison_da_f(ison, "galaxia_rift", 0.0);
    d->galaxia_nebulae     = (int)ison_da_n(ison, "galaxia_nebulae", 0);
    d->max_supergigantes   = (int)ison_da_n(ison, "max_supergigantes", 0);
    d->max_gigantes        = (int)ison_da_n(ison, "max_gigantes", 0);
    d->max_exotica         = (int)ison_da_n(ison, "max_exotica", 0);
    d->numerus_galaxiarum  = (int)ison_da_n(ison, "numerus_galaxiarum", 0);

}

/* ================================================================
 * scriptor
 * ================================================================ */

void designatio_in_ison(FILE *f, const designatio_t *d)
{
    fprintf(f, "{\n");
    fprintf(f, "  \"latitudo\": %d,\n", d->latitudo);
    fprintf(f, "  \"altitudo\": %d,\n", d->altitudo);
    fprintf(f, "  \"numerus_stellarum\": %d,\n", d->numerus_stellarum);
    fprintf(f, "  \"densitas_galaxiae\": %.2f,\n", d->densitas_galaxiae);
    fprintf(f, "  \"inclinatio_galaxiae\": %.2f,\n", d->inclinatio_galaxiae);
    fprintf(f, "  \"latitudo_galaxiae\": %.2f,\n", d->latitudo_galaxiae);
    fprintf(f, "  \"semen\": %u,\n", d->semen);
    fprintf(f, "  \"galaxia_glow\": %.1f,\n", d->galaxia_glow);
    fprintf(f, "  \"galaxia_rift\": %.1f,\n", d->galaxia_rift);
    fprintf(f, "  \"galaxia_nebulae\": %d,\n", d->galaxia_nebulae);
    fprintf(f, "  \"max_supergigantes\": %d,\n", d->max_supergigantes);
    fprintf(f, "  \"max_gigantes\": %d,\n", d->max_gigantes);
    fprintf(f, "  \"max_exotica\": %d,\n", d->max_exotica);
    fprintf(f, "  \"numerus_galaxiarum\": %d\n", d->numerus_galaxiarum);
    fprintf(f, "}\n");
}

/* ================================================================
 * generator — universalis ex designatione
 * ================================================================ */

static unsigned int des_semen = 1;
static unsigned int des_alea(void)
{
    des_semen ^= des_semen << 13;
    des_semen ^= des_semen >> 17;
    des_semen ^= des_semen << 5;
    return des_semen;
}
static double des_alea_f(void)
{
    return (double)(des_alea() & 0xFFFFFF) / (double)0x1000000;
}

static const char *des_syllabae_init[] = {
    "Al", "Be", "Ca", "De", "El", "Fa", "Ga", "He",
    "In", "Ju", "Ka", "Le", "Ma", "Ne", "Or", "Pa",
    "Ri", "Sa", "Te", "Ul", "Ve", "Za", "Ar", "Ce",
    "Di", "Fe", "Gi", "Ha", "Ke", "Li", "Mi", "No"
};
static const char *des_syllabae_med[] = {
    "ra", "li", "ta", "ni", "go", "be", "de", "mu",
    "si", "la", "na", "ri", "pe", "vo", "cu", "ma",
    "to", "ze", "phi", "the", "gi", "do", "ba", "fe",
    "nu", "ro", "chi", "le", "mi", "sa", "vi", "tu"
};
static const char *des_syllabae_fin[] = {
    "us", "is", "um", "ax", "on", "ar", "es", "or",
    "ix", "ae", "an", "os", "ur", "el", "as", "ia"
};

static char *des_nomen_fac(void)
{
    char buf[32];
    const char *init = des_syllabae_init[des_alea() % 32];
    const char *fin  = des_syllabae_fin[des_alea() % 16];
    int n_med = 1 + (int)(des_alea() % 3);  /* 1-3 syllabae mediae */
    int pos = 0;
    for (int i = 0; init[i]; i++)
        buf[pos++] = init[i];
    for (int m = 0; m < n_med; m++) {
        const char *med = des_syllabae_med[des_alea() % 32];
        for (int i = 0; med[i]; i++)
            buf[pos++] = med[i];
    }
    for (int i = 0; fin[i]; i++)
        buf[pos++] = fin[i];
    buf[pos] = '\0';
    return strdup(buf);
}

static void adde_stellam(
    universalis_t *u, int ux, int vy, sidereus_t qui,
    double mag, double temp, double phase, double ang,
    char *nomen
) {
    sidus_universalis_t su;
    su.u     = ux;
    su.v     = vy;
    su.nomen = nomen;
    memset(&su.sidus, 0, sizeof(sidus_t));
    su.sidus.qui = qui;
    su.sidus.ubi.sequentia.pro.magnitudo   = mag;
    su.sidus.ubi.sequentia.pro.temperatura = temp;

    if (qui == SIDUS_MAGNETAR) {
        su.sidus.ubi.magnetar.res.phase = phase;
    } else if (qui == SIDUS_VAGANS) {
        su.sidus.ubi.vagans.res.phase   = phase;
        su.sidus.ubi.vagans.res.angulus = ang;
    }

    int n      = u->numerus_stellarum;
    u->stellae = realloc(u->stellae, (size_t)(n + 1) * sizeof(sidus_universalis_t));
    if (!u->stellae)
        return;
    u->stellae[n]        = su;
    u->numerus_stellarum = n + 1;
}

static void adde_galaxiam(
    universalis_t *u, int ux, int vy,
    double mag, double temp, int morph_id, double ang,
    char *nomen
) {
    sidus_universalis_t su;
    su.u     = ux;
    su.v     = vy;
    su.nomen = nomen;
    memset(&su.sidus, 0, sizeof(sidus_t));
    su.sidus.qui = SIDUS_GALAXIA;
    su.sidus.ubi.galaxia.res.morphologia = (galaxia_morphologia_t)morph_id;
    su.sidus.ubi.galaxia.res.angulus     = ang;
    su.sidus.ubi.sequentia.pro.magnitudo   = mag;
    su.sidus.ubi.sequentia.pro.temperatura = temp;

    int n       = u->numerus_galaxiarum;
    u->galaxiae = realloc(u->galaxiae, (size_t)(n + 1) * sizeof(sidus_universalis_t));
    if (!u->galaxiae)
        return;
    u->galaxiae[n]        = su;
    u->numerus_galaxiarum = n + 1;
}

universalis_t *universalis_ex_designatione(const designatio_t *d)
{
    universalis_t *u = calloc(1, sizeof(universalis_t));
    if (!u)
        return NULL;

    u->latitudo = d->latitudo;
    u->altitudo = d->altitudo;

    int num_stellarum  = d->numerus_stellarum;
    double dens_gal    = d->densitas_galaxiae;
    double inc_gal     = d->inclinatio_galaxiae;
    double lat_gal     = d->latitudo_galaxiae;
    int max_sg         = d->max_supergigantes;
    int max_gi         = d->max_gigantes;
    int max_ex         = d->max_exotica;
    int num_galaxiarum = d->numerus_galaxiarum;

    des_semen = d->semen;
    int n_gi  = 0, n_sg = 0, n_ex = 0;

    /* stellae */
    for (int i = 0; i < num_stellarum; i++) {
        double fx = des_alea_f() * u->latitudo;
        double fy = des_alea_f() * u->altitudo;

        double tx       = fx / u->latitudo;
        double ty       = fy / u->altitudo;
        double y_fascia = 0.5 + (inc_gal / 3.0) * sin(DUO_PI * tx);
        double dy       = ty - y_fascia;
        if (dy > 0.5)
            dy -= 1.0;
        if (dy < -0.5)
            dy += 1.0;
        double dist_gal = fabs(dy) / (lat_gal + 0.001);

        if (
            des_alea_f() > dens_gal * exp(-dist_gal * dist_gal * 8.0)
            + (1.0 - dens_gal)
        )
            continue;

        double r_mag = des_alea_f();
        double mag   = 6.0 - 6.0 * r_mag * r_mag * r_mag * r_mag;

        sidereus_t qui = SIDUS_SEQUENTIA;
        int genus_id   = 1;
        double gr      = des_alea_f();

        if (gr < 0.06) {
            qui      = SIDUS_NANUM_ALBUM;
            genus_id = 0;
        } else if (gr < 0.96) {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        } else if (gr < 0.98) {
            qui      = SIDUS_GIGAS_RUBRUM;
            genus_id = 2;
        } else if (gr < 0.985) {
            qui      = SIDUS_SUPERGIGAS;
            genus_id = 3;
        } else if (gr < 0.992) {
            qui      = SIDUS_NEUTRONIUM;
            genus_id = 4;
        } else if (gr < 0.9995) {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        } else if (gr < 0.99975) {
            qui      = SIDUS_CRYSTALLINUM;
            genus_id = 5;
        } else {
            qui      = SIDUS_MAGNETAR;
            genus_id = 6;
        }

        if (genus_id == 2 && max_gi > 0 && n_gi >= max_gi) {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        }
        if (genus_id == 3 && max_sg > 0 && n_sg >= max_sg) {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        }
        if (genus_id >= 4 && genus_id <= 6 && max_ex > 0 && n_ex >= max_ex) {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        }

        double temp;
        {
            double tr = des_alea_f();
            if (tr < 0.40)
                temp = 2400 + des_alea_f() * 1300;
            else if (tr < 0.76)
                temp = 3700 + des_alea_f() * 1500;
            else if (tr < 0.88)
                temp = 5200 + des_alea_f() * 800;
            else if (tr < 0.95)
                temp = 6000 + des_alea_f() * 1500;
            else if (tr < 0.98)
                temp = 7500 + des_alea_f() * 2500;
            else
                temp = 10000 + des_alea_f() * 20000;
        }

        switch (genus_id) {
        case 0: temp = 4000 + des_alea_f() * 30000;
            mag = 4.0 + des_alea_f() * 2.0;
            break;
        case 2: temp = 2500 + des_alea_f() * 2500;
            mag = 1.5 + des_alea_f() * 2.5;
            n_gi++;
            break;
        case 3: temp = 3000 + des_alea_f() * 25000;
            mag = 0.5 + des_alea_f() * 1.5;
            n_sg++;
            break;
        case 4: temp = 500000;
            mag = 3.0 + des_alea_f() * 3.0;
            n_ex++;
            break;
        case 5: temp = 6000 + des_alea_f() * 10000;
            mag = 2.0 + des_alea_f() * 3.0;
            n_ex++;
            break;
        case 6: temp = 5000000;
            mag = 1.0 + des_alea_f() * 2.0;
            n_ex++;
            break;
        default: break;
        }

        adde_stellam(
            u, (int)fx, (int)fy, qui, mag, temp, 0, 0,
            des_nomen_fac()
        );
    }

    /* galaxiae distantes */
    {
        for (int i = 0; i < num_galaxiarum; i++) {

            double r_mag = des_alea_f();
            double mag   = 3.0 + 3.0 * (1.0 - pow(r_mag, 0.4));

            int morph_id;
            double mr = des_alea_f();
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

            double cos_incl  = des_alea_f();
            double temp_code = cos_incl * 10000.0;
            double ang       = des_alea_f() * DUO_PI;
            int px           = (int)(des_alea_f() * u->latitudo);
            int py           = (int)(des_alea_f() * u->altitudo);

            adde_galaxiam(
                u, px, py, mag, temp_code, morph_id, ang,
                des_nomen_fac()
            );
        }
    }

    return u;
}
