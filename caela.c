/*
 * caela.c — caela stellarum: lector, scriptor, generator
 */

#include "formula.h"
#include "orbita.h"
#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DUO_PI
#define DUO_PI (2.0 * 3.14159265358979323846)
#endif

/* ================================================================
 * soles invenire
 * ================================================================ */

static void caela_soles_invenire(caela_t *c)
{
    c->sol_primus   = -1;
    c->sol_secundus = -1;
    double sc1      = -1, sc2 = -1;

    for (int i = 0; i < c->numerus_planetarum; i++) {
        if (c->planetae[i].planeta.qui != PLANETA_SOL)
            continue;
        double s = c->planetae[i].scala_naturalis;
        if (s > sc1) {
            c->sol_secundus = c->sol_primus;
            sc2 = sc1;
            c->sol_primus = i;
            sc1 = s;
        } else if (s > sc2) {
            c->sol_secundus = i;
            sc2 = s;
        }
    }
}

/* ================================================================
 * destructor
 * ================================================================ */

void caela_destruere(caela_t *c)
{
    if (!c)
        return;
    free(c->sidera);
    if (c->planetae) {
        for (int i = 0; i < c->numerus_planetarum; i++) {
            free(c->planetae[i].nomen);
            free(c->planetae[i].fenestra_cacata);
        }
        free(c->planetae);
    }
    free(c);
}

/* ================================================================
 * caela_ex_ison — legit caelam ex chorda ISON
 * ================================================================ */

static void lege_sidus(const char *raw, void *ctx_v)
{
    caela_t *c = (caela_t *)ctx_v;
    int n      = c->numerus_siderum;
    c->sidera  = realloc(c->sidera, (size_t)(n + 1) * sizeof(sidus_caeli_t));
    if (!c->sidera)
        return;

    sidus_caeli_t *sc = &c->sidera[n];
    sc->x = (int)ison_da_n(raw, "x", 0);
    sc->y = (int)ison_da_n(raw, "y", 0);

    char *sidus_raw = ison_da_crudum(raw, "sidus");
    if (sidus_raw) {
        sidus_ex_ison(&sc->sidus, sidus_raw);
        free(sidus_raw);
    } else {
        memset(&sc->sidus, 0, sizeof(sidus_t));
        sc->sidus.qui = SIDUS_SEQUENTIA;
        sc->sidus.ubi.sequentia.pro.magnitudo   = 6.0;
        sc->sidus.ubi.sequentia.pro.temperatura = 5000;
    }
    c->numerus_siderum = n + 1;
}

static void lege_planetam(const char *raw, void *ctx_v)
{
    caela_t *c  = (caela_t *)ctx_v;
    int n       = c->numerus_planetarum;
    c->planetae = realloc(c->planetae, (size_t)(n + 1) * sizeof(planeta_caeli_t));
    if (!c->planetae)
        return;

    planeta_caeli_t *pc = &c->planetae[n];
    memset(pc, 0, sizeof(*pc));
    pc->x     = ison_da_f(raw, "x", 0);
    pc->y     = ison_da_f(raw, "y", 0);
    pc->scala = ison_da_f(raw, "scala", 1.0);
    pc->nomen = ison_da_chordam(raw, "nomen");

    char *per_s = ison_da_crudum(raw, "perceptus");
    if (per_s) {
        pc->perceptus = planeta_perceptus_ex_ison(per_s);
        free(per_s);
    }

    char *pl_ison = ison_da_crudum(raw, "planeta");
    if (pl_ison) {
        planeta_t *tmp = planeta_ex_ison(pl_ison);
        free(pl_ison);
        if (tmp) {
            pc->planeta = *tmp;
            free(tmp);
        }
    }

    c->numerus_planetarum = n + 1;
}

caela_t *caela_ex_ison(const char *ison)
{
    caela_t *c = calloc(1, sizeof(caela_t));
    if (!c)
        return NULL;

    c->latitudo            = (int)ison_da_n(ison, "latitudo", 1024);
    c->altitudo            = (int)ison_da_n(ison, "altitudo", 512);
    c->galaxia_glow        = ison_da_f(ison, "galaxia_glow", 0);
    c->galaxia_rift        = ison_da_f(ison, "galaxia_rift", 0);
    c->galaxia_nebulae     = (int)ison_da_n(ison, "galaxia_nebulae", 0);
    c->inclinatio_galaxiae = ison_da_f(ison, "inclinatio_galaxiae", 0);

    ison_pro_quoque_elemento(ison, "sidera", lege_sidus, c);
    ison_pro_quoque_elemento(ison, "planetae", lege_planetam, c);

    caela_soles_invenire(c);
    return c;
}

/* ================================================================
 * caela_in_ison — scribit caelam in chordam ISON
 * ================================================================ */

void planeta_caeli_in_ison(FILE *f, const planeta_caeli_t *pc)
{
    fprintf(f, "{\"x\":%.1f, \"y\":%.1f, \"scala\":%.3f", pc->x, pc->y, pc->scala);
    if (pc->nomen)
        fprintf(f, ", \"nomen\":\"%s\"", pc->nomen);
    fprintf(f, ", \"perceptus\": ");
    planeta_perceptus_in_ison(f, &pc->perceptus);
    fprintf(f, ", \"planeta\": ");
    planeta_in_ison(f, &pc->planeta);
    fprintf(f, "}");
}

char *caela_in_ison(const caela_t *c)
{
    /* scribit in plicam temporariam, deinde legit in chordam */
    char *result = NULL;
    size_t sz    = 0;
    FILE *f      = open_memstream(&result, &sz);
    if (!f)
        return NULL;

    fprintf(f, "{\n");
    fprintf(f, "  \"latitudo\": %d,\n", c->latitudo);
    fprintf(f, "  \"altitudo\": %d,\n", c->altitudo);
    fprintf(f, "  \"galaxia_glow\": %.4f,\n", c->galaxia_glow);
    fprintf(f, "  \"galaxia_rift\": %.4f,\n", c->galaxia_rift);
    fprintf(f, "  \"galaxia_nebulae\": %d,\n", c->galaxia_nebulae);
    fprintf(f, "  \"inclinatio_galaxiae\": %.4f,\n", c->inclinatio_galaxiae);

    /* sidera */
    fprintf(f, "  \"sidera\": [\n");
    for (int i = 0; i < c->numerus_siderum; i++) {
        const sidus_caeli_t *sc = &c->sidera[i];
        if (i > 0)
            fprintf(f, ",\n");
        fprintf(f, "    {\"x\": %d, \"y\": %d, \"sidus\": ", sc->x, sc->y);
        sidus_in_ison(f, &sc->sidus);
        fprintf(f, "}");
    }
    fprintf(f, "\n  ],\n");

    /* planetae */
    fprintf(f, "  \"planetae\": [\n");
    for (int i = 0; i < c->numerus_planetarum; i++) {
        if (i > 0)
            fprintf(f, ",\n");
        fprintf(f, "    ");
        planeta_caeli_in_ison(f, &c->planetae[i]);
    }
    fprintf(f, "\n  ]\n");
    fprintf(f, "}\n");

    fclose(f);
    return result;
}

/* ================================================================
 * caela_ex_formula — generat caelam ex formula
 * ================================================================ */

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

static void adde_sidus(
    caela_t *c, int x, int y, sidereus_t qui,
    double mag, double temp, double phase, double ang
) {
    int n     = c->numerus_siderum;
    c->sidera = realloc(c->sidera, (size_t)(n + 1) * sizeof(sidus_caeli_t));
    if (!c->sidera)
        return;

    sidus_caeli_t *sc = &c->sidera[n];
    sc->x = x;
    sc->y = y;
    memset(&sc->sidus, 0, sizeof(sidus_t));
    sc->sidus.qui = qui;
    sc->sidus.ubi.sequentia.pro.magnitudo   = mag;
    sc->sidus.ubi.sequentia.pro.temperatura = temp;

    if (qui == SIDUS_GALAXIA) {
        sc->sidus.ubi.galaxia.res.morphologia = (galaxia_morphologia_t)(int)phase;
        sc->sidus.ubi.galaxia.res.angulus     = ang;
    } else if (qui == SIDUS_MAGNETAR) {
        sc->sidus.ubi.magnetar.res.phase = phase;
    } else if (qui == SIDUS_VAGANS) {
        sc->sidus.ubi.vagans.res.phase   = phase;
        sc->sidus.ubi.vagans.res.angulus = ang;
    }

    c->numerus_siderum = n + 1;
}

caela_t *caela_ex_formula(const formula_t *f, int t)
{
    caela_t *c = calloc(1, sizeof(caela_t));
    if (!c)
        return NULL;

    c->latitudo            = f->latitudo;
    c->altitudo            = f->altitudo;
    c->galaxia_glow        = f->galaxia_glow;
    c->galaxia_rift        = f->galaxia_rift;
    c->galaxia_nebulae     = f->galaxia_nebulae;
    c->inclinatio_galaxiae = f->inclinatio_galaxiae;

    int num_stellarum  = f->numerus_stellarum;
    double dens_gal    = f->densitas_galaxiae;
    double inc_gal     = f->inclinatio_galaxiae;
    double lat_gal     = f->latitudo_galaxiae;
    unsigned int semen = f->semen;
    int max_sg         = f->max_supergigantes;
    int max_gi         = f->max_gigantes;
    int max_ex         = f->max_exotica;
    int num_planet     = f->numerus_planetarum;
    double pl_tmin     = f->planetae_temp_min;
    double pl_tmax     = f->planetae_temp_max;
    int num_galaxiarum = f->numerus_galaxiarum;
    int max_galaxiae   = f->max_galaxiae;

    semen_loc = semen;
    int n_gi  = 0, n_sg = 0, n_ex = 0;

    for (int i = 0; i < num_stellarum; i++) {
        double fx = alea_f() * c->latitudo;
        double fy = alea_f() * c->altitudo;

        double tx       = fx / c->latitudo;
        double ty       = fy / c->altitudo;
        double y_fascia = 0.5 + (inc_gal / 3.0) * sin(DUO_PI * tx);
        double dy       = ty - y_fascia;
        if (dy > 0.5)
            dy -= 1.0;
        if (dy < -0.5)
            dy += 1.0;
        double dist_gal = fabs(dy) / (lat_gal + 0.001);

        if (
            alea_f() > dens_gal * exp(-dist_gal * dist_gal * 8.0)
            + (1.0 - dens_gal)
        )
            continue;

        double r_mag = alea_f();
        double mag   = 6.0 - 6.0 * r_mag * r_mag * r_mag * r_mag;

        sidereus_t qui = SIDUS_SEQUENTIA;
        int genus_id   = 1;
        double gr      = alea_f();

        if (gr < 0.06)        {
            qui      = SIDUS_NANUM_ALBUM;
            genus_id = 0;
        }else if (gr < 0.96)   {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        }else if (gr < 0.98)   {
            qui      = SIDUS_GIGAS_RUBRUM;
            genus_id = 2;
        }else if (gr < 0.985)  {
            qui      = SIDUS_SUPERGIGAS;
            genus_id = 3;
        }else if (gr < 0.992)  {
            qui      = SIDUS_NEUTRONIUM;
            genus_id = 4;
        }else if (gr < 0.9995) {
            qui      = SIDUS_SEQUENTIA;
            genus_id = 1;
        }else if (gr < 0.99975){
            qui      = SIDUS_CRYSTALLINUM;
            genus_id = 5;
        }else                  {
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

        adde_sidus(c, (int)fx, (int)fy, qui, mag, temp, 0, 0);
    }

    /* galaxiae distantes */
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

            double cos_incl  = alea_f();
            double temp_code = cos_incl * 10000.0;
            double ang       = alea_f() * DUO_PI;
            int px           = (int)(alea_f() * c->latitudo);
            int py           = (int)(alea_f() * c->altitudo);

            adde_sidus(c, px, py, SIDUS_GALAXIA, mag, temp_code, morph_id, ang);
            n_gal++;
        }
    }

    /* vagantes (planetae simplices) */
    for (int i = 0; i < num_planet; i++) {
        double temp  = pl_tmin + alea_f() * (pl_tmax - pl_tmin);
        double mag   = 1.0 + alea_f() * 3.0;
        double phase = alea_f() * 0.45;
        double ang   = alea_f() * DUO_PI;
        int px       = (int)(alea_f() * c->latitudo);
        int py       = (int)(alea_f() * c->altitudo);

        adde_sidus(c, px, py, SIDUS_VAGANS, mag, temp, phase, ang);
    }

    /* planetae (ex formula, orbita ad t=0) */
    if (f->numerus_planetarum_formulae > 0) {
        c->numerus_planetarum = f->numerus_planetarum_formulae;
        c->planetae = calloc((size_t)c->numerus_planetarum, sizeof(planeta_caeli_t));
        if (c->planetae) {
            for (int i = 0; i < c->numerus_planetarum; i++) {
                const planeta_formulae_t *pf = &f->planetae[i];
                planeta_caeli_t *pc = &c->planetae[i];
                double x, y, z;
                orbita_computare(&pf->orbita, t, c->latitudo, c->altitudo, &x, &y, &z);
                pc->x               = x;
                pc->y               = y;
                pc->z               = z;
                pc->scala_naturalis = pf->scala;
                pc->scala           = pf->scala / (1.0 - z * 0.5);
                pc->nomen           = pf->nomen ? strdup(pf->nomen) : NULL;
                pc->planeta         = pf->planeta;
            }
        }
    }

    caela_soles_invenire(c);
    return c;
}

/* distantia toroidalis minima */
static double dist_toro(double x1, double y1, double x2, double y2, int lat, int alt)
{
    double dx = fabs(x1 - x2);
    double dy = fabs(y1 - y2);
    if (dx > lat * 0.5)
        dx = lat - dx;
    if (dy > alt * 0.5)
        dy = alt - dy;
    return sqrt(dx * dx + dy * dy);
}

/* angulus toroidalis (directio minima) */
static double ang_toro(double x1, double y1, double x2, double y2, int lat, int alt)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    if (dx >  lat * 0.5)
        dx -= lat;
    if (dx < -lat * 0.5)
        dx += lat;
    if (dy >  alt * 0.5)
        dy -= alt;
    if (dy < -alt * 0.5)
        dy += alt;
    return atan2(dy, dx);
}

static void computare_perceptum(
    planeta_caeli_t *pc, const planeta_caeli_t *sol, int lat, int alt
) {
    double dist = dist_toro(pc->x, pc->y, sol->x, sol->y, lat, alt);
    double ang  = ang_toro(pc->x, pc->y, sol->x, sol->y, lat, alt);
    double diag = sqrt((double)(lat * lat + alt * alt));

    /* z > 0 = propior, sol magis post = umbra maior
     * z < 0 = longinquior, sol magis ante = plenius illuminata */
    double prox   = 1.0 / (1.0 + dist / (diag * 0.15));
    double z_term = pc->z * 0.3;
    double situs  = prox * 0.5 + z_term;
    if (situs < 0.0)
        situs = 0.0;
    if (situs > 0.9)
        situs = 0.9;

    /* lumen inversus distantiae, proportionale luminositati solis */
    double sol_lum = sol->planeta.ubi.sol.res.luminositas;
    if (sol_lum < 0.01)
        sol_lum = 1.0;
    double lumen = sol_lum * 1.5 / (1.0 + dist / (diag * 0.3));

    pc->perceptus.aspectus.angulus = ang;
    pc->perceptus.aspectus.situs   = situs;
    pc->perceptus.aspectus.lumen   = lumen;

    static int dbg = 0;
    if (dbg++ % 300 == 0)
        fprintf(
            stderr, "  perceptum: %s dist=%.0f ang=%.2f z=%.2f prox=%.2f situs=%.2f lumen=%.2f\n",
            pc->nomen ? pc->nomen : "?", dist, ang, pc->z, prox, situs, lumen
        );
}

void caela_orbitas_applicare(caela_t *caela, const formula_t *f, int t)
{
    int n = caela->numerus_planetarum;
    if (n > f->numerus_planetarum_formulae)
        n = f->numerus_planetarum_formulae;

    /* positiones et z */
    for (int i = 0; i < n; i++) {
        double x, y, z;
        orbita_computare(
            &f->planetae[i].orbita, t,
            caela->latitudo, caela->altitudo, &x, &y, &z
        );
        caela->planetae[i].x     = x;
        caela->planetae[i].y     = y;
        caela->planetae[i].z     = z;
        caela->planetae[i].scala = caela->planetae[i].scala_naturalis / (1.0 - z * 0.5);
    }

    /* soles re-invenire (scala mutata) */
    caela_soles_invenire(caela);
    if (t % 60 == 0)
        fprintf(
            stderr, "t=%d sol_primus=%d sol_secundus=%d n_plan=%d\n",
            t, caela->sol_primus, caela->sol_secundus, caela->numerus_planetarum
        );

    /* perceptus ex solibus */
    int lat = caela->latitudo, alt = caela->altitudo;
    for (int i = 0; i < caela->numerus_planetarum; i++) {
        planeta_caeli_t *pc = &caela->planetae[i];
        if (pc->planeta.qui == PLANETA_SOL)
            continue;

        memset(&pc->perceptus, 0, sizeof(pc->perceptus));

        if (caela->sol_primus >= 0)
            computare_perceptum(pc, &caela->planetae[caela->sol_primus], lat, alt);

        if (caela->sol_secundus >= 0) {
            planeta_perceptus_t salva = pc->perceptus;
            computare_perceptum(pc, &caela->planetae[caela->sol_secundus], lat, alt);
            /* movere secundum ad coaspectus */
            salva.coaspectus = pc->perceptus.aspectus;
            pc->perceptus    = salva;
        }
    }
}
