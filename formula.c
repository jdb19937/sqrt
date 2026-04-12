/*
 * formula.c — formula generationis caeli: lector et scriptor
 */

#include "formula.h"
#include "orbita.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * lector
 * ================================================================ */

static void lege_planetam(const char *raw, void *ctx_v)
{
    formula_t *f = (formula_t *)ctx_v;
    int n        = f->numerus_planetarum_formulae;
    f->planetae  = realloc(f->planetae, (size_t)(n + 1) * sizeof(planeta_formulae_t));
    if (!f->planetae)
        return;

    planeta_formulae_t *pf = &f->planetae[n];
    memset(pf, 0, sizeof(*pf));
    pf->nomen = ison_da_chordam(raw, "nomen");
    pf->scala = ison_da_f(raw, "scala", 1.0);

    char *pl_ison = ison_da_crudum(raw, "planeta");
    if (pl_ison) {
        planeta_t *tmp = planeta_ex_ison(pl_ison);
        free(pl_ison);
        if (tmp) {
            pf->planeta = *tmp;
            free(tmp);
        }
    }

    char *orb_ison = ison_da_crudum(raw, "orbita");
    if (orb_ison) {
        orbita_ex_ison(&pf->orbita, orb_ison);
        free(orb_ison);
    }

    f->numerus_planetarum_formulae = n + 1;
}

void formula_ex_ison(formula_t *f, const char *ison)
{
    memset(f, 0, sizeof(*f));
    f->latitudo            = (int)ison_da_n(ison, "latitudo", 1024);
    f->altitudo            = (int)ison_da_n(ison, "altitudo", 512);
    f->numerus_stellarum   = (int)ison_da_n(ison, "numerus_stellarum", 10000);
    f->densitas_galaxiae   = ison_da_f(ison, "densitas_galaxiae", 0.5);
    f->inclinatio_galaxiae = ison_da_f(ison, "inclinatio_galaxiae", 0.0);
    f->latitudo_galaxiae   = ison_da_f(ison, "latitudo_galaxiae", 0.2);
    f->semen               = (unsigned int)ison_da_n(ison, "semen", 1);
    f->galaxia_glow        = ison_da_f(ison, "galaxia_glow", 0.0);
    f->galaxia_rift        = ison_da_f(ison, "galaxia_rift", 0.0);
    f->galaxia_nebulae     = (int)ison_da_n(ison, "galaxia_nebulae", 0);
    f->max_supergigantes   = (int)ison_da_n(ison, "max_supergigantes", 0);
    f->max_gigantes        = (int)ison_da_n(ison, "max_gigantes", 0);
    f->max_exotica         = (int)ison_da_n(ison, "max_exotica", 0);
    f->numerus_planetarum  = (int)ison_da_n(ison, "numerus_planetarum", 0);
    f->planetae_temp_min   = ison_da_f(ison, "planetae_temp_min", 4000);
    f->planetae_temp_max   = ison_da_f(ison, "planetae_temp_max", 6000);
    f->numerus_galaxiarum  = (int)ison_da_n(ison, "numerus_galaxiarum", 0);
    f->max_galaxiae        = (int)ison_da_n(ison, "max_galaxiae", 0);

    ison_pro_quoque_elemento(ison, "planetae", lege_planetam, f);
}

/* ================================================================
 * scriptor
 * ================================================================ */

void formula_in_ison(FILE *f, const formula_t *form)
{
    fprintf(f, "{\n");
    fprintf(f, "  \"latitudo\": %d,\n", form->latitudo);
    fprintf(f, "  \"altitudo\": %d,\n", form->altitudo);
    fprintf(f, "  \"numerus_stellarum\": %d,\n", form->numerus_stellarum);
    fprintf(f, "  \"densitas_galaxiae\": %.2f,\n", form->densitas_galaxiae);
    fprintf(f, "  \"inclinatio_galaxiae\": %.2f,\n", form->inclinatio_galaxiae);
    fprintf(f, "  \"latitudo_galaxiae\": %.2f,\n", form->latitudo_galaxiae);
    fprintf(f, "  \"semen\": %u,\n", form->semen);
    fprintf(f, "  \"galaxia_glow\": %.1f,\n", form->galaxia_glow);
    fprintf(f, "  \"galaxia_rift\": %.1f,\n", form->galaxia_rift);
    fprintf(f, "  \"galaxia_nebulae\": %d,\n", form->galaxia_nebulae);
    fprintf(f, "  \"max_supergigantes\": %d,\n", form->max_supergigantes);
    fprintf(f, "  \"max_gigantes\": %d,\n", form->max_gigantes);
    fprintf(f, "  \"max_exotica\": %d,\n", form->max_exotica);
    fprintf(f, "  \"numerus_planetarum\": %d,\n", form->numerus_planetarum);
    fprintf(f, "  \"planetae_temp_min\": %.0f,\n", form->planetae_temp_min);
    fprintf(f, "  \"planetae_temp_max\": %.0f,\n", form->planetae_temp_max);
    fprintf(f, "  \"numerus_galaxiarum\": %d,\n", form->numerus_galaxiarum);
    fprintf(f, "  \"max_galaxiae\": %d,\n", form->max_galaxiae);

    fprintf(f, "  \"planetae\": [\n");
    for (int i = 0; i < form->numerus_planetarum_formulae; i++) {
        const planeta_formulae_t *pf = &form->planetae[i];
        if (i > 0)
            fprintf(f, ",\n");
        fprintf(f, "    {");
        if (pf->nomen)
            fprintf(f, "\"nomen\": \"%s\", ", pf->nomen);
        fprintf(f, "\"scala\": %.3f, ", pf->scala);
        fprintf(f, "\"planeta\": ");
        planeta_in_ison(f, &pf->planeta);
        fprintf(f, ", \"orbita\": ");
        orbita_in_ison(f, &pf->orbita);
        fprintf(f, "}");
    }
    fprintf(f, "\n  ]\n");
    fprintf(f, "}\n");
}

/* ================================================================
 * destructor
 * ================================================================ */

void formula_purgare(formula_t *f)
{
    if (f->planetae) {
        for (int i = 0; i < f->numerus_planetarum_formulae; i++)
            free(f->planetae[i].nomen);
        free(f->planetae);
        f->planetae = NULL;
    }
}

/* ================================================================
 * generator — logica procedurae ex genera.c
 * ================================================================ */

#include <math.h>

#define DUO_PI_F 6.283185307179586
#define MAX_PLANETAE 10
#define LATITUDO_PRAEF 4096
#define ALTITUDO_PRAEF 4096

static unsigned int gen_semen = 1;
static unsigned int gen_alea(void) {
    gen_semen ^= gen_semen << 13;
    gen_semen ^= gen_semen >> 17;
    gen_semen ^= gen_semen << 5;
    return gen_semen;
}
static double gen_alea_f(void) { return (double)(gen_alea() & 0xFFFFFF) / (double)0x1000000; }
static double gen_inter(double a, double b) { return a + gen_alea_f() * (b - a); }
static int gen_n(int a, int b) { return b <= a ? a : a + (int)(gen_alea_f() * (b - a + 1)); }

typedef struct {
    double massa, luminositas, temperatura;
    int classis;
    char genus[32];
} gen_stella_t;

static int gen_classis(double t) {
    if (t >= 30000)
        return 0;
    if (t >= 10000)
        return 1;
    if (t >= 7500)
        return 2;
    if (t >= 6000)
        return 3;
    if (t >= 5200)
        return 4;
    if (t >= 3700)
        return 5;
    return 6;
}

static void gen_stella_computa(double temp, gen_stella_t *s) {
    s->temperatura = temp;
    s->classis     = gen_classis(temp);
    if (temp < 3700)
        s->massa = 0.08 + 0.37 * (temp - 2400.0) / 1300.0;
    else if (temp < 5200)
        s->massa = 0.45 + 0.35 * (temp - 3700.0) / 1500.0;
    else if (temp < 6000)
        s->massa = 0.80 + 0.24 * (temp - 5200.0) / 800.0;
    else if (temp < 7500)
        s->massa = 1.04 + 0.36 * (temp - 6000.0) / 1500.0;
    else if (temp < 10000)
        s->massa = 1.40 + 0.70 * (temp - 7500.0) / 2500.0;
    else if (temp < 30000)
        s->massa = 2.10 + 13.9 * (temp - 10000.0) / 20000.0;
    else
        s->massa = 16.0 + 34.0 * (temp - 30000.0) / 20000.0;
    if (s->massa < 0.08)
        s->massa = 0.08;
    if (s->massa > 150.0)
        s->massa = 150.0;
    s->luminositas = s->massa < 0.43 ? 0.23 * pow(s->massa, 2.3) : pow(s->massa, 3.5);
    if (s->luminositas < 1e-6)
        s->luminositas = 1e-6;
}

static int gen_num_planetae(int classis) {
    switch (classis) {
    case 0: return gen_n(0, 1);
    case 1: return gen_n(0, 3);
    case 2: return gen_n(1, 4);
    case 3: return gen_n(2, 6);
    case 4: return gen_n(3, 8);
    case 5: return gen_n(3, 7);
    case 6: return gen_n(2, 6);
    }
    return gen_n(2, 5);
}

typedef enum {
    GEN_ADUSTUM, GEN_TEMPERATUM, GEN_ARIDUM, GEN_VENEREUM,
    GEN_GASEOSUM, GEN_GLACIALE, GEN_PARVUM
} gen_cat_t;

static gen_cat_t gen_categoria(double d, double lum) {
    double sl        = sqrt(lum);
    double r_hab_int = 0.75 * sl;
    double r_hab_ext = 1.80 * sl;
    double rf        = 2.70 * sl;

    if (d < rf) {
        if (d < r_hab_int * 0.5)
            return GEN_ADUSTUM;
        if (d >= r_hab_int && d <= r_hab_ext) {
            double r = gen_alea_f();
            if (r < 0.40)
                return GEN_TEMPERATUM;
            if (r < 0.65)
                return GEN_ARIDUM;
            return GEN_VENEREUM;
        }
        if (d < r_hab_int)
            return gen_alea_f() < 0.6 ? GEN_ADUSTUM : GEN_VENEREUM;
        return GEN_ARIDUM;
    }
    if (d < 2.5 * rf)
        return GEN_GASEOSUM;
    if (d < 6.0 * rf)
        return gen_alea_f() < 0.65 ? GEN_GLACIALE : GEN_GASEOSUM;
    return GEN_PARVUM;
}

static const char *gen_nomina[] = {
    "Fornax", "Candor", "Viridis", "Pratum", "Desertum",
    "Velatus", "Magnus", "Glacialis", "Parvulus", "Remotus",
    "Ignis", "Lucifer", "Aurora", "Astrum", "Flamma"
};

/* conversio planeta_gen interna → planeta_formulae_t */
static void gen_adde_planetam(
    formula_t *f, const char *nomen, double scala,
    planetarius_t qui, int lat, int alt
) {
    int n       = f->numerus_planetarum_formulae;
    f->planetae = realloc(f->planetae, (size_t)(n + 1) * sizeof(planeta_formulae_t));
    if (!f->planetae)
        return;

    planeta_formulae_t *pf = &f->planetae[n];
    memset(pf, 0, sizeof(*pf));
    pf->nomen       = strdup(nomen);
    pf->scala       = scala;
    pf->planeta.qui = qui;

    /* orbita praefinita — positio aleata, quiescens */
    pf->orbita.cx           = gen_inter(0, lat);
    pf->orbita.cy           = gen_inter(0, alt);
    pf->orbita.periodus     = 360;
    pf->orbita.frequentia_x = 1;
    pf->orbita.frequentia_y = 1;

    f->numerus_planetarum_formulae = n + 1;
}

void formula_generare(
    formula_t *f, unsigned int semen,
    const sidus_t *sidus, const sidus_t *cosidus
) {
    gen_semen = semen ? semen : 1;
    f->semen  = 1;

    int lat = LATITUDO_PRAEF;
    int alt = ALTITUDO_PRAEF;

    /* stella ex sidere */
    double temp = sidus->ubi.sequentia.pro.temperatura;
    gen_stella_t stella;
    gen_stella_computa(temp, &stella);

    /* parametri campi */
    f->latitudo            = lat;
    f->altitudo            = alt;
    f->numerus_stellarum   = 40000 + (int)(gen_alea_f() * 110000);
    f->densitas_galaxiae   = gen_inter(0.30, 0.90);
    f->inclinatio_galaxiae = gen_inter(0.0, 1.20);
    f->latitudo_galaxiae   = gen_inter(0.08, 0.25);
    f->galaxia_glow        = gen_inter(0.0, 1.0);
    f->galaxia_rift        = gen_inter(0.0, 0.8);
    f->galaxia_nebulae     = gen_n(0, 8);
    f->max_supergigantes   = gen_n(0, 3);
    f->max_gigantes        = gen_n(2, 10);
    f->max_exotica         = gen_n(0, 2);
    f->numerus_planetarum  = 0;
    f->planetae_temp_min   = stella.temperatura * 0.7;
    f->planetae_temp_max   = stella.temperatura * 1.1;
    if (f->planetae_temp_min < 2400)
        f->planetae_temp_min = 2400;
    if (f->planetae_temp_max < f->planetae_temp_min + 500)
        f->planetae_temp_max = f->planetae_temp_min + 500;
    f->numerus_galaxiarum = gen_n(50, 400);
    f->max_galaxiae       = 0;

    /* sol */
    {
        gen_adde_planetam(f, "Sol", gen_inter(0.8, 1.2), PLANETA_SOL, lat, alt);
        planeta_formulae_t *pf = &f->planetae[f->numerus_planetarum_formulae - 1];
        sol_t *s = &pf->planeta.ubi.sol;
        s->pro.radius = 0.88;
        s->pro.semen = gen_alea();
        s->res.fusio = 1.0;
        s->res.temperatura = stella.temperatura;
        s->res.luminositas = 1.0;

        double base_c[] = {0.87, 0.80, 0.77, 0.67, 0.60, 0.50, 0.30};
        double var_c[]  = {0.08, 0.10, 0.08, 0.08, 0.10, 0.10, 0.10};
        s->res.corona = base_c[stella.classis] + gen_inter(-var_c[stella.classis], var_c[stella.classis]);

        double base_g[] = {0.10, 0.12, 0.17, 0.30, 0.45, 0.55, 0.62};
        double var_g[]  = {0.05, 0.05, 0.08, 0.10, 0.10, 0.10, 0.13};
        s->res.granulatio = base_g[stella.classis] + gen_inter(-var_g[stella.classis], var_g[stella.classis]);

        s->res.h2 = 0.735;
        s->res.he = 0.248;

        int mac_b[] = {0, 0, 0, 1, 2, 3, 2};
        int mac_v[] = {0, 1, 1, 2, 4, 5, 3};
        s->res.maculae = mac_b[stella.classis] + gen_n(0, mac_v[stella.classis]);
        if (s->res.maculae > 0) {
            s->res.macula_radius     = gen_inter(0.3, 0.9);
            s->res.macula_obscuritas = gen_inter(0.3, 0.7);
        }
    }

    /* cosidus — secundus sol */
    if (cosidus) {
        double cotemp = cosidus->ubi.sequentia.pro.temperatura;
        gen_stella_t costella;
        gen_stella_computa(cotemp, &costella);

        gen_adde_planetam(f, "Sol Minor", gen_inter(0.3, 0.7), PLANETA_SOL, lat, alt);
        planeta_formulae_t *pf = &f->planetae[f->numerus_planetarum_formulae - 1];
        sol_t *s = &pf->planeta.ubi.sol;
        s->pro.radius = 0.88;
        s->pro.semen = gen_alea();
        s->res.fusio = 1.0;
        s->res.temperatura = cotemp;
        s->res.luminositas = costella.luminositas / stella.luminositas;
        if (s->res.luminositas > 1.0)
            s->res.luminositas = 1.0;
        s->res.corona     = gen_inter(0.3, 0.6);
        s->res.granulatio = gen_inter(0.1, 0.5);
        s->res.h2         = 0.735;
        s->res.he         = 0.248;

        /* cosol orbitat circa sol primarium */
        pf->orbita.cx = f->planetae[0].orbita.cx;
        pf->orbita.cy = f->planetae[0].orbita.cy;
        pf->orbita.amplitudo_x = gen_inter(200, 600);
        pf->orbita.amplitudo_y = gen_inter(200, 600);
        pf->orbita.phase_y = 1.5708;
        pf->orbita.periodus = gen_n(300, 800);
    }

    /* orbitae */
    int num_planet = gen_num_planetae(stella.classis);
    if (num_planet > MAX_PLANETAE)
        num_planet = MAX_PLANETAE;

    double orbitae[MAX_PLANETAE];
    {
        double r0 = gen_inter(0.15, 0.45) * sqrt(stella.luminositas);
        if (r0 < 0.02)
            r0 = 0.02;
        double ratio = gen_inter(1.4, 2.8);
        for (int i = 0; i < num_planet; i++) {
            orbitae[i] = r0 * pow(ratio, (double)i);
            orbitae[i] *= 1.0 + gen_inter(-0.15, 0.15);
        }
    }

    /* sol positio pro orbitas */
    double sol_cx = f->planetae[0].orbita.cx;
    double sol_cy = f->planetae[0].orbita.cy;

    /* genera planetas */
    for (int i = 0; i < num_planet; i++) {
        gen_cat_t cat = gen_categoria(orbitae[i], stella.luminositas);

        const char *nomen = gen_nomina[(gen_alea() % 15)];
        double scala;
        planetarius_t qui;

        switch (cat) {
        case GEN_ADUSTUM:
        case GEN_ARIDUM:
        case GEN_VENEREUM:
        case GEN_TEMPERATUM:
            qui   = PLANETA_SAXOSUM;
            scala = gen_inter(0.15, 0.50);
            break;
        case GEN_GASEOSUM:
            qui   = PLANETA_GASEOSUM;
            scala = gen_inter(0.25, 0.55);
            break;
        case GEN_GLACIALE:
            qui   = PLANETA_GLACIALE;
            scala = gen_inter(0.20, 0.40);
            break;
        case GEN_PARVUM:
            qui   = PLANETA_PARVUM;
            scala = gen_inter(0.10, 0.20);
            break;
        }

        gen_adde_planetam(f, nomen, scala, qui, lat, alt);
        planeta_formulae_t *pf = &f->planetae[f->numerus_planetarum_formulae - 1];

        /* planetella */
        pf->planeta.ubi.saxosum.pro.radius     = 0.90;
        pf->planeta.ubi.saxosum.pro.inclinatio = gen_inter(0.0, 0.6);
        pf->planeta.ubi.saxosum.pro.rotatio    = gen_alea_f() * DUO_PI_F;
        pf->planeta.ubi.saxosum.pro.semen      = gen_alea();

        /* genus-specifici campi */
        switch (cat) {
        case GEN_ADUSTUM: {
                saxosculum_t *r = &pf->planeta.ubi.saxosum.res;
                r->silicata     = gen_inter(0.40, 0.80);
                r->ferrum       = gen_inter(0.10, 0.40);
                r->carbo        = gen_inter(0.0, 0.15);
                r->craterae     = gen_inter(0.50, 0.90);
                break;
            }
        case GEN_TEMPERATUM: {
                saxosculum_t *r = &pf->planeta.ubi.saxosum.res;
                r->silicata = gen_inter(0.20, 0.40);
                r->ferrum = gen_inter(0.05, 0.15);
                r->malachita = gen_inter(0.25, 0.55);
                r->glacies = gen_inter(0.02, 0.10);
                r->aqua = gen_inter(0.30, 0.80);
                r->aqua_profunditas = gen_inter(0.3, 0.7);
                r->continentes = gen_n(2, 8);
                r->tectonica = gen_inter(0.4, 0.9);
                r->pressio_kPa = gen_inter(50, 200);
                r->n2 = gen_inter(0.60, 0.85);
                r->o2 = gen_inter(0.10, 0.30);
                r->nubes = gen_inter(0.15, 0.60);
                r->polaris = gen_inter(0.05, 0.20);
                break;
            }
        case GEN_ARIDUM: {
                saxosculum_t *r = &pf->planeta.ubi.saxosum.res;
                r->silicata = gen_inter(0.20, 0.40);
                r->ferrum = gen_inter(0.30, 0.60);
                r->carbo = gen_inter(0.0, 0.10);
                r->craterae = gen_inter(0.20, 0.60);
                r->pressio_kPa = gen_inter(0.1, 5.0);
                r->co2 = gen_inter(0.85, 0.97);
                r->pulvis = gen_inter(0.30, 0.70);
                r->polaris = gen_inter(0.02, 0.10);
                break;
            }
        case GEN_VENEREUM: {
                saxosculum_t *r = &pf->planeta.ubi.saxosum.res;
                r->silicata = gen_inter(0.40, 0.60);
                r->ferrum = gen_inter(0.10, 0.25);
                r->vulcanismus = gen_inter(0.2, 0.6);
                r->tectonica = gen_inter(0.3, 0.6);
                r->pressio_kPa = gen_inter(1000, 15000);
                r->co2 = gen_inter(0.85, 0.98);
                r->n2 = gen_inter(0.01, 0.10);
                r->nubes = gen_inter(0.85, 1.0);
                r->pulvis = gen_inter(0.2, 0.5);
                break;
            }
        case GEN_GASEOSUM: {
                gaseosculum_t *r = &pf->planeta.ubi.gaseosum.res;
                r->h2 = gen_inter(0.82, 0.96);
                r->he = 1.0 - r->h2 - gen_inter(0.0, 0.01);
                r->nh3 = gen_inter(0.0, 0.02);
                r->fasciae = gen_n(6, 14);
                r->fasciae_contrast = gen_inter(0.30, 0.80);
                if (gen_alea_f() < 0.6) {
                    r->maculae = gen_n(1, 3);
                    r->macula_lat = gen_inter(-0.5, 0.5);
                    r->macula_lon = gen_inter(0.0, DUO_PI_F);
                    r->macula_radius = gen_inter(0.05, 0.20);
                    r->macula_obscuritas = gen_inter(0.3, 0.9);
                }
                break;
            }
        case GEN_GLACIALE: {
                glaciellum_t *r = &pf->planeta.ubi.glaciale.res;
                r->h2 = gen_inter(0.75, 0.85);
                r->he = gen_inter(0.10, 0.20);
                r->ch4 = gen_inter(0.01, 0.03);
                r->fasciae = gen_n(2, 8);
                r->fasciae_contrast = gen_inter(0.05, 0.30);
                if (gen_alea_f() < 0.3) {
                    r->maculae       = 1;
                    r->macula_lat    = gen_inter(-0.4, 0.4);
                    r->macula_lon    = gen_inter(0.0, DUO_PI_F);
                    r->macula_radius = gen_inter(0.05, 0.15);
                }
                break;
            }
        case GEN_PARVUM: {
                parvulum_t *r = &pf->planeta.ubi.parvum.res;
                r->glacies    = gen_inter(0.20, 0.50);
                r->silicata   = gen_inter(0.15, 0.30);
                r->carbo      = gen_inter(0.10, 0.30);
                r->ferrum     = gen_inter(0.05, 0.20);
                r->craterae   = gen_inter(0.20, 0.50);
                r->polaris    = gen_inter(0.05, 0.15);
                if (gen_alea_f() < 0.3) {
                    r->pressio_kPa = 0.001;
                    r->n2 = gen_inter(0.50, 0.95);
                    r->ch4 = 1.0 - r->n2;
                }
                break;
            }
        }

        /* orbita — interiores circunt solem, exteriores lineares */
        double rf = 2.70 * sqrt(stella.luminositas);
        if (orbitae[i] < 2.5 * rf) {
            /* interiores: elliptica circa solem in plano aleato */
            double amp_major = 500.0 + orbitae[i] * 400.0;
            if (amp_major > 1800)
                amp_major = 1800;
            double amp_minor = gen_inter(200, 400);
            double phase     = gen_alea_f() * DUO_PI_F;

            /* planum aleatum: maioritas in x-z, y-z, vel inclinatum.
             * z syncatum cum axe maiore: phase_z = phase_maior + π/2.
             * sic planeta transit ante solem (z>0) quando super sole,
             * et post solem (z<0) quando redit. z crossat 0 ad latera. */
            double planum = gen_alea_f();
            if (planum < 0.6) {
                /* x-z planum */
                pf->orbita.cx = sol_cx;
                pf->orbita.cy = sol_cy + gen_inter(-60, 60);
                pf->orbita.amplitudo_x = amp_major;
                pf->orbita.amplitudo_y = amp_minor;
                pf->orbita.phase_z = phase + 1.5708;  /* sync cum x */
            } else if (planum < 0.85) {
                /* y-z planum */
                pf->orbita.cx = sol_cx + gen_inter(-60, 60);
                pf->orbita.cy = sol_cy;
                pf->orbita.amplitudo_x = amp_minor;
                pf->orbita.amplitudo_y = amp_major;
                pf->orbita.phase_z = phase + 1.5708 + 1.5708;  /* sync cum y (= phase_y + π/2) */
            } else {
                /* planum inclinatum */
                pf->orbita.cx = sol_cx;
                pf->orbita.cy = sol_cy;
                pf->orbita.amplitudo_x = amp_major * gen_inter(0.5, 1.0);
                pf->orbita.amplitudo_y = amp_major * gen_inter(0.3, 0.7);
                pf->orbita.phase_z = phase + 1.5708;  /* sync cum x */
            }
            double phase_diff      = gen_inter(1.1, 2.0);  /* ~63°-115°: ellipsis rotata sed non degeneras */
            pf->orbita.phase_x     = phase;
            pf->orbita.phase_y     = phase + phase_diff;
            pf->orbita.periodus    = 120 + (int)(orbitae[i] * 80);
            pf->orbita.amplitudo_z = gen_inter(0.3, 0.6);
        } else {
            /* exteriores: lineares wrapping, sinusoide in y */
            pf->orbita.cx = 0;
            pf->orbita.cy = sol_cy + gen_inter(-500, 500);
            pf->orbita.revolutiones_x = gen_n(1, 2);
            pf->orbita.amplitudo_y = gen_inter(60, 200);
            pf->orbita.frequentia_y = gen_n(1, 4);
            pf->orbita.periodus = 250 + gen_n(0, 500);
            /* aliqui ante solem (+z), aliqui post (-z) */
            pf->orbita.cz = gen_alea_f() < 0.5
                ? gen_inter(0.2, 0.8)
                : gen_inter(-0.8, -0.2);
        }
    }
}
