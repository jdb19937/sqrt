/*
 * genera.c — campum ex sidere generare
 *
 * Ex proprietatibus sideris (et cosideris optionalis) systema
 * planetarium integrum proceduraliter generat. Reddit ISON
 * campi ad stdout — formatum idem ac terra.ison.
 *
 * Fundamenta astrophysica:
 *
 *   Massa stellaris ex temperatura superficiei derivatur per
 *   relationem empiricam M-T pro sequentia principali
 *   (Eker et al. 2018).
 *
 *   Luminositas ex relatione massa-luminositas:
 *   L ∝ M^3.5 pro M > 0.43 M☉ (Kuiper 1938, Duric 2004).
 *   L = 0.23 M^2.3 pro M < 0.43 M☉.
 *
 *   Zona habitabilis: r ≈ √(L/L☉) AU (Kasting et al. 1993).
 *   Linea glacialis: r ≈ 2.7√(L/L☉) AU (Hayashi 1981).
 *
 *   Numerus planetarum ex statisticis Keplerianis
 *   (Batalha et al. 2013): stellae FGK ~3-8, M ~2-6, OBA ~0-3.
 *
 *   Distributio orbitalis: progressio geometrica generalizata
 *   (regula Titii-Bode, Dermott 1968).
 *
 *   Compositio planetarum ex distantia determinatur:
 *   intra lineam glacialem saxosa, circa eam gaseosa, ultra glacialia.
 *
 * Usus:
 *   ./genera <semen>
 *   ./genera <sidus.ison> <semen>
 *   ./genera <sidus.ison> <cosidus.ison> <semen>
 */

#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DUO_PI          6.283185307179586
#define MAX_PLANETAE    10
#define LATITUDO_PRAEF  4096
#define ALTITUDO_PRAEF  4096

/* ================================================================
 * generans numerum pseudo-aleatorium (xorshift32)
 * ================================================================ */

static unsigned int semen_g = 1;

static unsigned int alea(void)
{
    semen_g ^= semen_g << 13;
    semen_g ^= semen_g >> 17;
    semen_g ^= semen_g << 5;
    return semen_g;
}

static double alea_f(void)
{
    return (double)(alea() & 0xFFFFFF) / (double)0x1000000;
}

static double alea_inter(double a, double b)
{
    return a + alea_f() * (b - a);
}

static int alea_n(int a, int b)
{
    if (b <= a)
        return a;
    return a + (int)(alea_f() * (b - a + 1));
}

/* ================================================================
 * physica stellaris
 *
 * Relatio M-T pro sequentia principali:
 *   T(K) → M(M☉) per interpolationem linearem inter classes.
 *
 * Relatio M-L (Kuiper 1938, Duric 2004):
 *   L = M^3.5   pro M > 0.43 M☉
 *   L = 0.23 M^2.3  pro M ≤ 0.43 M☉
 * ================================================================ */

typedef struct {
    char   genus[32];
    double massa;       /* M☉ */
    double luminositas; /* L☉ */
    double temperatura; /* K */
    int    classis;     /* 0=O, 1=B, 2=A, 3=F, 4=G, 5=K, 6=M */
} stella_t;

static int classis_ex_temperatura(double t)
{
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

static void stella_computa(const char *genus, double temp, stella_t *s)
{
    strncpy(s->genus, genus ? genus : "sequentia", 31);
    s->temperatura = temp;
    s->classis     = classis_ex_temperatura(temp);

    if (!genus || strcmp(genus, "sequentia") == 0) {
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

        if (s->massa < 0.43)
            s->luminositas = 0.23 * pow(s->massa, 2.3);
        else
            s->luminositas = pow(s->massa, 3.5);
    } else if (strcmp(genus, "nanum_album") == 0) {
        s->massa       = 0.6;
        /* R ≈ 0.01 R☉, L = R²(T/T☉)⁴ */
        s->luminositas = 0.0001 * pow(temp / 10000.0, 4.0);
    } else if (strcmp(genus, "gigas_rubrum") == 0) {
        s->massa       = 1.5;
        double r       = 30.0;
        s->luminositas = r * r * pow(temp / 5778.0, 4.0);
    } else if (strcmp(genus, "supergigas") == 0) {
        s->massa       = 15.0;
        double r       = 200.0;
        s->luminositas = r * r * pow(temp / 5778.0, 4.0);
    } else {
        /* neutronium, crystallinum, magnetar */
        s->massa       = 1.4;
        s->luminositas = 0.00001;
    }

    if (s->massa < 0.08)
        s->massa = 0.08;
    if (s->massa > 150.0)
        s->massa = 150.0;
    if (s->luminositas < 1e-6)
        s->luminositas = 1e-6;
}

/* ================================================================
 * zonae orbitales
 * ================================================================ */

typedef struct {
    double r_hab_int;   /* limen interius zonae habitabilis (AU) */
    double r_hab_ext;   /* limen exterius (AU) */
    double r_glacialis; /* linea glacialis (AU) */
} zonae_t;

static zonae_t zonae_computa(double lum)
{
    zonae_t z;
    double sl     = sqrt(lum);
    z.r_hab_int   = 0.75 * sl;
    z.r_hab_ext   = 1.80 * sl;
    z.r_glacialis = 2.70 * sl;
    return z;
}

/* ================================================================
 * nomina
 * ================================================================ */

static const char *nomina_adusta[]    = {
    "Fornax", "Candor", "Adustus", "Ignis", "Fervidus"
};
static const char *nomina_temp[]      = {
    "Viridis", "Pratum", "Hortus", "Oceanus", "Silva"
};
static const char *nomina_arida[]     = {
    "Desertum", "Sterilis", "Ferrugo", "Cineris", "Ruber"
};
static const char *nomina_venerea[]   = {
    "Velatus", "Opacus", "Calidus", "Nebulosa", "Inferna"
};
static const char *nomina_gaseosa[]   = {
    "Magnus", "Colossus", "Ventosus", "Turbidus", "Procella"
};
static const char *nomina_glacialia[] = {
    "Glacialis", "Gelidus", "Frigidus", "Hibernus", "Caeruleum"
};
static const char *nomina_parva[]     = {
    "Parvulus", "Remotus", "Extremus", "Umbratilis", "Exiguus"
};
static const char *nomina_stellarum[] = {
    "Ignis", "Lucifer", "Aurora", "Astrum", "Flamma",
    "Stella", "Lampas", "Lux", "Fax", "Iubar"
};

static int idx_ad = 0, idx_te = 0, idx_ar = 0, idx_ve = 0;
static int idx_ga = 0, idx_gl = 0, idx_pa = 0;

/* ================================================================
 * categoriae planetarum
 * ================================================================ */

typedef enum {
    CAT_ADUSTUM,     /* scorched (Mercurius) */
    CAT_TEMPERATUM,  /* habitable (Terra) */
    CAT_ARIDUM,      /* barren (Mars) */
    CAT_VENEREUM,    /* thick atmosphere (Venus) */
    CAT_GASEOSUM,    /* gas giant (Jupiter) */
    CAT_GLACIALE,    /* ice giant (Neptunus) */
    CAT_PARVUM       /* dwarf (Pluto) */
} categoria_t;

/* ================================================================
 * planeta generatum
 * ================================================================ */

typedef struct {
    char   nomen[64];
    char   genus[16];
    int    x, y;
    double scala;
    double radius;
    double inclinatio;
    double rotatio;
    unsigned int semen_p;

    /* compositio */
    double silicata, ferrum, sulphur, carbo;
    double glacies, glacies_co2, malachita;

    /* aqua */
    double aqua, aqua_profunditas;

    /* terrain */
    int    continentes;
    double tectonica, craterae, maria, vulcanismus;

    /* atmosphaera */
    double pressio_kPa;
    double n2, o2, co2, ch4, h2, he, nh3;
    double pulvis, nubes, polaris;

    /* fasciae */
    int    fasciae;
    double fasciae_contrast;

    /* maculae */
    int    maculae;
    double macula_lat, macula_lon, macula_radius, macula_obscuritas;

    /* sol */
    double fusio, temperatura, corona, granulatio;
    int    est_sol;

    /* perceptus */
    double asp_situs, asp_angulus, asp_lumen;
    double coasp_situs, coasp_angulus, coasp_lumen;
    double acuitas, detallum, granum;

    categoria_t cat;
} planeta_gen_t;

static void initia_planetam(planeta_gen_t *p)
{
    memset(p, 0, sizeof(*p));
    p->radius = 0.90;
}

/* ================================================================
 * generatores per categoriam
 * ================================================================ */

static void genera_adustum(planeta_gen_t *p)
{
    strcpy(p->genus, "saxosum");
    p->cat = CAT_ADUSTUM;
    strncpy(p->nomen, nomina_adusta[idx_ad++ % 5], 63);

    p->silicata = alea_inter(0.40, 0.80);
    p->ferrum   = alea_inter(0.10, 0.40);
    p->carbo    = alea_inter(0.0, 0.15);
    p->craterae = alea_inter(0.50, 0.90);
    p->maria    = alea_inter(0.0, 0.25);
    p->scala    = alea_inter(0.15, 0.30);
}

static void genera_temperatum(planeta_gen_t *p)
{
    strcpy(p->genus, "saxosum");
    p->cat = CAT_TEMPERATUM;
    strncpy(p->nomen, nomina_temp[idx_te++ % 5], 63);

    p->silicata         = alea_inter(0.20, 0.40);
    p->ferrum           = alea_inter(0.05, 0.15);
    p->malachita        = alea_inter(0.25, 0.55);
    p->glacies          = alea_inter(0.02, 0.10);
    p->aqua             = alea_inter(0.30, 0.80);
    p->aqua_profunditas = alea_inter(0.3, 0.7);
    p->continentes      = alea_n(2, 8);
    p->tectonica        = alea_inter(0.4, 0.9);
    p->pressio_kPa      = alea_inter(50, 200);
    p->n2               = alea_inter(0.60, 0.85);
    p->o2               = alea_inter(0.10, 0.30);
    p->nubes            = alea_inter(0.15, 0.60);
    p->polaris          = alea_inter(0.05, 0.20);
    p->scala            = alea_inter(0.30, 0.60);
    p->inclinatio       = alea_inter(0.1, 0.6);
}

static void genera_aridum(planeta_gen_t *p)
{
    strcpy(p->genus, "saxosum");
    p->cat = CAT_ARIDUM;
    strncpy(p->nomen, nomina_arida[idx_ar++ % 5], 63);

    p->silicata    = alea_inter(0.20, 0.40);
    p->ferrum      = alea_inter(0.30, 0.60);
    p->carbo       = alea_inter(0.0, 0.10);
    p->craterae    = alea_inter(0.20, 0.60);
    p->pressio_kPa = alea_inter(0.1, 5.0);
    p->co2         = alea_inter(0.85, 0.97);
    p->pulvis      = alea_inter(0.30, 0.70);
    p->polaris     = alea_inter(0.02, 0.10);
    p->scala       = alea_inter(0.20, 0.40);
    p->inclinatio  = alea_inter(0.1, 0.5);
}

static void genera_venereum(planeta_gen_t *p)
{
    strcpy(p->genus, "saxosum");
    p->cat = CAT_VENEREUM;
    strncpy(p->nomen, nomina_venerea[idx_ve++ % 5], 63);

    p->silicata    = alea_inter(0.40, 0.60);
    p->ferrum      = alea_inter(0.10, 0.25);
    p->vulcanismus = alea_inter(0.2, 0.6);
    p->tectonica   = alea_inter(0.3, 0.6);
    p->pressio_kPa = alea_inter(1000, 15000);
    p->co2         = alea_inter(0.85, 0.98);
    p->n2          = alea_inter(0.01, 0.10);
    p->nubes       = alea_inter(0.85, 1.0);
    p->pulvis      = alea_inter(0.2, 0.5);
    p->scala       = alea_inter(0.25, 0.45);
    p->inclinatio  = alea_inter(0.0, 0.3);
}

static void genera_gaseosum_p(planeta_gen_t *p)
{
    strcpy(p->genus, "gaseosum");
    p->cat = CAT_GASEOSUM;
    strncpy(p->nomen, nomina_gaseosa[idx_ga++ % 5], 63);

    p->h2               = alea_inter(0.82, 0.96);
    p->he               = 1.0 - p->h2 - alea_inter(0.0, 0.01);
    p->nh3              = alea_inter(0.0, 0.02);
    p->fasciae          = alea_n(6, 14);
    p->fasciae_contrast = alea_inter(0.30, 0.80);
    p->scala            = alea_inter(0.25, 0.55);
    p->inclinatio       = alea_inter(0.0, 0.6);

    if (alea_f() < 0.6) {
        p->maculae           = alea_n(1, 3);
        p->macula_lat        = alea_inter(-0.5, 0.5);
        p->macula_lon        = alea_inter(0.0, DUO_PI);
        p->macula_radius     = alea_inter(0.05, 0.20);
        p->macula_obscuritas = alea_inter(0.3, 0.9);
    }
}

static void genera_glaciale_p(planeta_gen_t *p)
{
    strcpy(p->genus, "glaciale");
    p->cat = CAT_GLACIALE;
    strncpy(p->nomen, nomina_glacialia[idx_gl++ % 5], 63);

    p->h2               = alea_inter(0.75, 0.85);
    p->he               = alea_inter(0.10, 0.20);
    p->ch4              = alea_inter(0.01, 0.03);
    p->fasciae          = alea_n(2, 8);
    p->fasciae_contrast = alea_inter(0.05, 0.30);
    p->scala            = alea_inter(0.20, 0.40);
    p->inclinatio       = alea_inter(0.0, 1.8);

    if (alea_f() < 0.3) {
        p->maculae           = 1;
        p->macula_lat        = alea_inter(-0.4, 0.4);
        p->macula_lon        = alea_inter(0.0, DUO_PI);
        p->macula_radius     = alea_inter(0.05, 0.15);
        p->macula_obscuritas = alea_inter(0.4, 0.8);
    }
}

static void genera_parvum_p(planeta_gen_t *p)
{
    strcpy(p->genus, "parvum");
    p->cat = CAT_PARVUM;
    strncpy(p->nomen, nomina_parva[idx_pa++ % 5], 63);

    p->glacies  = alea_inter(0.20, 0.50);
    p->silicata = alea_inter(0.15, 0.30);
    p->carbo    = alea_inter(0.10, 0.30);
    p->ferrum   = alea_inter(0.05, 0.20);
    p->craterae = alea_inter(0.20, 0.50);
    p->polaris  = alea_inter(0.05, 0.15);
    p->scala    = alea_inter(0.10, 0.20);

    /* atmosphaera tenuis rara (30%) */
    if (alea_f() < 0.3) {
        p->pressio_kPa = 0.001;
        p->n2          = alea_inter(0.50, 0.95);
        p->ch4         = 1.0 - p->n2;
    }

    p->inclinatio = alea_inter(0.0, 2.2);
}

/* ================================================================
 * sol planeta ex stella
 * ================================================================ */

static void genera_solem(
    planeta_gen_t *p, const stella_t *s,
    const char *nomen, int lat, int alt
) {
    initia_planetam(p);
    strcpy(p->genus, "sol");
    strncpy(p->nomen, nomen, 63);
    p->est_sol = 1;

    p->x       = alea_n(0, lat - 1);
    p->y       = alea_n(0, alt - 1);
    p->scala   = alea_inter(0.8, 1.2);
    p->semen_p = alea();

    p->fusio       = 1.0;
    p->temperatura = s->temperatura;

    /* corona — fortior in stellis calidis */
    {
        double base[] = {0.87, 0.80, 0.77, 0.67, 0.60, 0.50, 0.30};
        double var[]  = {0.08, 0.10, 0.08, 0.08, 0.10, 0.10, 0.10};
        int c     = s->classis;
        p->corona = base[c] + alea_inter(-var[c], var[c]);
    }

    /* granulatio — maior in stellis frigidis (convectio profunda) */
    {
        double base[] = {0.10, 0.12, 0.17, 0.30, 0.45, 0.55, 0.62};
        double var[]  = {0.05, 0.05, 0.08, 0.10, 0.10, 0.10, 0.13};
        int c         = s->classis;
        p->granulatio = base[c] + alea_inter(-var[c], var[c]);
    }

    p->h2 = 0.735;
    p->he = 0.248;

    /* maculae — plures in stellis K/M (convectio + dynamo) */
    {
        int mac_b[] = {0, 0, 0, 1, 2, 3, 2};
        int mac_v[] = {0, 1, 1, 2, 4, 5, 3};
        int c      = s->classis;
        p->maculae = mac_b[c] + alea_n(0, mac_v[c]);
    }
    if (p->maculae > 0) {
        p->macula_radius     = alea_inter(0.3, 0.9);
        p->macula_obscuritas = alea_inter(0.3, 0.7);
    }
}

/* ================================================================
 * perceptus (illuminatio)
 * ================================================================ */

/* angulus toroidalis a (x0,y0) ad (x1,y1) */
static double angulus_toroidalis(
    int x0, int y0, int x1, int y1,
    int lat, int alt
) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    if (dx >  lat / 2)
        dx -= lat;
    if (dx < -lat / 2)
        dx += lat;
    if (dy >  alt / 2)
        dy -= alt;
    if (dy < -alt / 2)
        dy += alt;
    return atan2((double)dy, (double)dx);
}

static void genera_perceptum(
    planeta_gen_t *p, double distantia,
    const stella_t *s,
    const planeta_gen_t *sol_p,
    const stella_t *co,
    const planeta_gen_t *cosol_p,
    double separatio, int habet_cosidus,
    int lat, int alt
) {
    /* aspectus primarius — angulus a planeta ad solem */
    p->asp_situs   = alea_inter(0.10, 0.65);
    p->asp_angulus = angulus_toroidalis(
        p->x, p->y, sol_p->x, sol_p->y, lat, alt
    );
    if (p->asp_angulus < 0)
        p->asp_angulus += DUO_PI;

    double lum_raw = s->luminositas / (distantia * distantia);
    p->asp_lumen   = 0.5 + 2.0 * lum_raw / (1.0 + lum_raw);
    if (p->asp_lumen > 3.0)
        p->asp_lumen = 3.0;
    if (p->asp_lumen < 0.3)
        p->asp_lumen = 0.3;

    /* coaspectus — angulus a planeta ad cosidus */
    if (habet_cosidus && co && cosol_p) {
        p->coasp_situs  = alea_inter(0.15, 0.70);
        p->coasp_angulus = angulus_toroidalis(
            p->x, p->y, cosol_p->x, cosol_p->y, lat, alt
        );
        if (p->coasp_angulus < 0)
            p->coasp_angulus += DUO_PI;

        double lum_co  = co->luminositas / (separatio * separatio);
        p->coasp_lumen = 0.3 + 1.5 * lum_co / (1.0 + lum_co);
        if (p->coasp_lumen > 2.5)
            p->coasp_lumen = 2.5;
        if (p->coasp_lumen < 0.1)
            p->coasp_lumen = 0.1;
    }

    p->acuitas  = alea_inter(0.5, 0.9);
    p->detallum = alea_inter(0.20, 0.50);
    p->granum   = alea_inter(0.05, 0.15);
}

/* ================================================================
 * numerus planetarum ex genere et classe sideris
 * ================================================================ */

static int genera_numerum(const char *genus, int classis)
{
    if (strcmp(genus, "nanum_album") == 0)
        return alea_n(0, 2);
    if (strcmp(genus, "gigas_rubrum") == 0)
        return alea_n(0, 4);
    if (strcmp(genus, "supergigas") == 0)
        return alea_n(0, 2);
    if (strcmp(genus, "neutronium") == 0)
        return alea_n(0, 1);
    if (strcmp(genus, "crystallinum") == 0)
        return alea_n(0, 1);
    if (strcmp(genus, "magnetar") == 0)
        return alea_n(0, 1);

    /* sequentia */
    switch (classis) {
    case 0: return alea_n(0, 1);
    case 1: return alea_n(0, 3);
    case 2: return alea_n(1, 4);
    case 3: return alea_n(2, 6);
    case 4: return alea_n(3, 8);
    case 5: return alea_n(3, 7);
    case 6: return alea_n(2, 6);
    }
    return alea_n(2, 5);
}

/* ================================================================
 * orbitae (progressio geometrica Titii-Bode)
 * ================================================================ */

static void genera_orbitae(
    double *dist, int n,
    double lum, const char *genus
) {
    double r0 = alea_inter(0.15, 0.45) * sqrt(lum);

    /* minima orbitae pro stellis evolutis */
    if (strcmp(genus, "gigas_rubrum") == 0 && r0 < 3.0)
        r0 = alea_inter(3.0, 8.0);
    else if (strcmp(genus, "supergigas") == 0 && r0 < 10.0)
        r0 = alea_inter(10.0, 30.0);
    else if (strcmp(genus, "nanum_album") == 0 && r0 < 2.0)
        r0 = alea_inter(2.0, 10.0);

    if (r0 < 0.02)
        r0 = 0.02;

    double ratio = alea_inter(1.4, 2.8);
    for (int i = 0; i < n; i++) {
        dist[i]  = r0 * pow(ratio, (double)i);
        dist[i] *= 1.0 + alea_inter(-0.15, 0.15);
    }
}

/* ================================================================
 * categoria ex distantia et zonis
 * ================================================================ */

static categoria_t determina_categoriam(double d, const zonae_t *z)
{
    double rf = z->r_glacialis;

    if (d < rf) {
        if (d < z->r_hab_int * 0.5)
            return CAT_ADUSTUM;

        if (d >= z->r_hab_int && d <= z->r_hab_ext) {
            double r = alea_f();
            if (r < 0.40)
                return CAT_TEMPERATUM;
            if (r < 0.65)
                return CAT_ARIDUM;
            return CAT_VENEREUM;
        }

        if (d < z->r_hab_int)
            return (alea_f() < 0.6) ? CAT_ADUSTUM : CAT_VENEREUM;

        return CAT_ARIDUM;
    }

    if (d < 2.5 * rf)
        return CAT_GASEOSUM;

    if (d < 6.0 * rf)
        return (alea_f() < 0.65) ? CAT_GLACIALE : CAT_GASEOSUM;

    return CAT_PARVUM;
}

/* ================================================================
 * temperatura aleatoriam (pro sidere non dato)
 * ================================================================ */

static double genera_temperaturam(void)
{
    double r = alea_f();
    if (r < 0.20)
        return alea_inter(2400, 3700);
    if (r < 0.45)
        return alea_inter(3700, 5200);
    if (r < 0.70)
        return alea_inter(5200, 6000);
    if (r < 0.85)
        return alea_inter(6000, 7500);
    if (r < 0.95)
        return alea_inter(7500, 10000);
    if (r < 0.99)
        return alea_inter(10000, 30000);
    return alea_inter(30000, 50000);
}

/* ================================================================
 * emissio ISON — planeta singulum
 * ================================================================ */

static void emitte_planetam(
    const planeta_gen_t *p,
    int habet_cosidus, int est_ultima
) {
    printf("    {\n");
    printf("      \"nomen\": \"%s\",\n", p->nomen);
    printf("      \"genus\": \"%s\",\n", p->genus);
    printf(
        "      \"x\": %d, \"y\": %d, \"scala\": %.2f,\n",
        p->x, p->y, p->scala
    );

    /* perceptus — non pro sole */
    if (!p->est_sol) {
        printf("      \"perceptus\": {\n");
        if (habet_cosidus) {
            printf(
                "        \"aspectus\":   "
                "{ \"situs\": %.2f, \"angulus\": %.2f, \"lumen\": %.1f },\n",
                p->asp_situs, p->asp_angulus, p->asp_lumen
            );
            printf(
                "        \"coaspectus\": "
                "{ \"situs\": %.2f, \"angulus\": %.2f, \"lumen\": %.1f },\n",
                p->coasp_situs, p->coasp_angulus, p->coasp_lumen
            );
        } else {
            printf(
                "        \"aspectus\": "
                "{ \"situs\": %.2f, \"angulus\": %.2f, \"lumen\": %.1f },\n",
                p->asp_situs, p->asp_angulus, p->asp_lumen
            );
        }
        printf(
            "        \"acuitas\": %.1f, \"detallum\": %.2f, \"granum\": %.2f\n",
            p->acuitas, p->detallum, p->granum
        );
        printf("      },\n");
    }

    printf("      \"radius\": %.2f,\n", p->radius);

    if (!p->est_sol) {
        printf("      \"inclinatio\": %.4f,\n", p->inclinatio);
        printf("      \"rotatio\": %.1f,\n", p->rotatio);
    }

    /* --- sol --- */
    if (p->est_sol) {
        printf("      \"fusio\": 1.0,\n");
        printf("      \"temperatura\": %.0f,\n", p->temperatura);
        printf("      \"corona\": %.2f,\n", p->corona);
        printf("      \"granulatio\": %.2f,\n", p->granulatio);
        printf("      \"h2\": %.3f,\n", p->h2);
        printf("      \"he\": %.3f,\n", p->he);
        if (p->maculae > 0) {
            printf("      \"maculae\": %d,\n", p->maculae);
            printf("      \"macula_radius\": %.1f,\n", p->macula_radius);
            printf("      \"macula_obscuritas\": %.2f,\n", p->macula_obscuritas);
        }

    /* --- saxosum / parvum --- */
    } else if (
        strcmp(p->genus, "saxosum") == 0
        || strcmp(p->genus, "parvum") == 0
    ) {
        if (p->silicata > 0.005)
            printf("      \"silicata\": %.2f,\n", p->silicata);
        if (p->ferrum > 0.005)
            printf("      \"ferrum\": %.2f,\n", p->ferrum);
        if (p->sulphur > 0.005)
            printf("      \"sulphur\": %.2f,\n", p->sulphur);
        if (p->carbo > 0.005)
            printf("      \"carbo\": %.2f,\n", p->carbo);
        if (p->glacies > 0.005)
            printf("      \"glacies\": %.2f,\n", p->glacies);
        if (p->malachita > 0.005)
            printf("      \"malachita\": %.2f,\n", p->malachita);

        if (p->aqua > 0.005) {
            printf("      \"aqua\": %.2f,\n", p->aqua);
            printf("      \"aqua_profunditas\": %.1f,\n", p->aqua_profunditas);
        }

        if (p->continentes > 0)
            printf("      \"continentes\": %d,\n", p->continentes);
        if (p->tectonica > 0.005)
            printf("      \"tectonica\": %.1f,\n", p->tectonica);
        if (p->craterae > 0.005)
            printf("      \"craterae\": %.2f,\n", p->craterae);
        if (p->maria > 0.005)
            printf("      \"maria\": %.2f,\n", p->maria);
        if (p->vulcanismus > 0.005)
            printf("      \"vulcanismus\": %.1f,\n", p->vulcanismus);

        if (p->pressio_kPa >= 0.1)
            printf("      \"pressio_kPa\": %.1f,\n", p->pressio_kPa);
        else if (p->pressio_kPa > 0.00005)
            printf("      \"pressio_kPa\": %.3f,\n", p->pressio_kPa);
        else
            printf("      \"pressio_kPa\": 0.0,\n");

        if (p->n2 > 0.005)
            printf("      \"n2\": %.2f,\n", p->n2);
        if (p->o2 > 0.005)
            printf("      \"o2\": %.2f,\n", p->o2);
        if (p->co2 > 0.005)
            printf("      \"co2\": %.3f,\n", p->co2);
        if (p->ch4 > 0.005)
            printf("      \"ch4\": %.2f,\n", p->ch4);
        if (p->pulvis > 0.005)
            printf("      \"pulvis\": %.1f,\n", p->pulvis);
        if (p->nubes > 0.005)
            printf("      \"nubes\": %.2f,\n", p->nubes);
        if (p->polaris > 0.005)
            printf("      \"polaris\": %.2f,\n", p->polaris);

    /* --- gaseosum / glaciale --- */
    } else {
        printf("      \"pressio_kPa\": 0,\n");
        printf("      \"h2\": %.2f,\n", p->h2);
        printf("      \"he\": %.2f,\n", p->he);
        if (p->ch4 > 0.005)
            printf("      \"ch4\": %.3f,\n", p->ch4);
        if (p->nh3 > 0.005)
            printf("      \"nh3\": %.3f,\n", p->nh3);
        if (p->fasciae > 0) {
            printf("      \"fasciae\": %d,\n", p->fasciae);
            printf("      \"fasciae_contrast\": %.2f,\n", p->fasciae_contrast);
        }
        if (p->maculae > 0) {
            printf("      \"maculae\": %d,\n", p->maculae);
            printf("      \"macula_lat\": %.2f,\n", p->macula_lat);
            printf("      \"macula_lon\": %.1f,\n", p->macula_lon);
            printf("      \"macula_radius\": %.2f,\n", p->macula_radius);
            printf("      \"macula_obscuritas\": %.1f,\n", p->macula_obscuritas);
        }
    }

    /* semen — semper ultimum, sine virgula */
    printf("      \"semen\": %u\n", p->semen_p);

    if (est_ultima)
        printf("    }\n");
    else
        printf("    },\n");
}

/* ================================================================
 * main
 * ================================================================ */

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(
            stderr,
            "Usus: genera <semen>\n"
            "       genera <sidus.ison> <semen>\n"
            "       genera <sidus.ison> <cosidus.ison> <semen>\n"
        );
        return 1;
    }

    /* ultimum argumentum semper semen */
    unsigned int semen = (unsigned int)strtoul(argv[argc - 1], NULL, 10);
    semen_g = semen;

    char *sidus_ison   = NULL;
    char *cosidus_ison = NULL;

    if (argc >= 3) {
        sidus_ison = ison_lege_plicam(argv[1]);
        if (!sidus_ison) {
            fprintf(stderr, "ERRATUM: %s legere non possum\n", argv[1]);
            return 1;
        }
    }
    if (argc >= 4) {
        cosidus_ison = ison_lege_plicam(argv[2]);
        if (!cosidus_ison) {
            fprintf(stderr, "ERRATUM: %s legere non possum\n", argv[2]);
            free(sidus_ison);
            return 1;
        }
    }

    /* --- sidus primarium --- */

    stella_t stella;
    char nomen_stellae[64];

    if (sidus_ison) {
        char *genus = ison_da_chordam(sidus_ison, "qui");
        double temp = ison_da_f(sidus_ison, "ubi.pro.temperatura", 0);
        if (temp < 100)
            temp = genera_temperaturam();

        char *nom = ison_da_chordam(sidus_ison, "nomen");
        if (nom) {
            strncpy(nomen_stellae, nom, 63);
            nomen_stellae[63] = '\0';
            free(nom);
        } else {
            snprintf(
                nomen_stellae, 64, "%s",
                nomina_stellarum[alea() % 10]
            );
        }

        stella_computa(genus, temp, &stella);
        free(genus);
    } else {
        /* sidus aleatorium */
        double r = alea_f();
        const char *genus;
        if (r < 0.95)
            genus = "sequentia";
        else if (r < 0.97)
            genus = "gigas_rubrum";
        else if (r < 0.98)
            genus = "supergigas";
        else if (r < 0.99)
            genus = "nanum_album";
        else
            genus = "sequentia";

        double temp = genera_temperaturam();
        if (strcmp(genus, "gigas_rubrum") == 0)
            temp = alea_inter(2500, 5000);
        else if (strcmp(genus, "supergigas") == 0)
            temp = alea_inter(3000, 25000);
        else if (strcmp(genus, "nanum_album") == 0)
            temp = alea_inter(5000, 40000);

        stella_computa(genus, temp, &stella);
        snprintf(nomen_stellae, 64, "%s", nomina_stellarum[alea() % 10]);
    }

    /* --- cosidus --- */

    stella_t costella;
    char nomen_costellae[64] = {0};
    int habet_cosidus = 0;
    double separatio  = 0;

    if (cosidus_ison) {
        habet_cosidus = 1;

        char *genus = ison_da_chordam(cosidus_ison, "qui");
        double temp = ison_da_f(cosidus_ison, "ubi.pro.temperatura", 0);
        if (temp < 100)
            temp = genera_temperaturam();

        char *nom = ison_da_chordam(cosidus_ison, "nomen");
        if (nom) {
            strncpy(nomen_costellae, nom, 63);
            nomen_costellae[63] = '\0';
            free(nom);
        } else {
            snprintf(nomen_costellae, 64, "%s Minor", nomen_stellae);
        }

        separatio = ison_da_f(cosidus_ison, "separatio", 0);
        if (separatio < 0.1)
            separatio = alea_inter(10, 200);

        stella_computa(genus, temp, &costella);
        free(genus);
    }

    /* --- zonae --- */

    int latitudo  = LATITUDO_PRAEF;
    int altitudo  = ALTITUDO_PRAEF;
    zonae_t zonae = zonae_computa(stella.luminositas);

    /* --- orbitae --- */

    int num_planet = genera_numerum(stella.genus, stella.classis);
    if (num_planet > MAX_PLANETAE)
        num_planet = MAX_PLANETAE;

    double orbitae[MAX_PLANETAE];
    genera_orbitae(orbitae, num_planet, stella.luminositas, stella.genus);

    /* in systemate binario, planetae ultra zonam stabilem removentur
     * (Holman & Wiegert 1999: r_stab ≈ separatio / 3) */
    if (habet_cosidus) {
        double r_stab = separatio / 3.0;
        int n         = 0;
        for (int i = 0; i < num_planet; i++)
            if (orbitae[i] < r_stab)
                orbitae[n++] = orbitae[i];
        num_planet = n;
    }

    /* --- genera planetae --- */

    planeta_gen_t planetae[MAX_PLANETAE + 2];
    int n_planetae = 0;

    /* sol primarius */
    genera_solem(
        &planetae[n_planetae++], &stella, nomen_stellae,
        latitudo, altitudo
    );

    /* cosidus */
    if (habet_cosidus) {
        genera_solem(
            &planetae[n_planetae], &costella, nomen_costellae,
            latitudo, altitudo
        );
        /* cosidus scala minor si obscurior */
        if (costella.luminositas < stella.luminositas)
            planetae[n_planetae].scala = alea_inter(0.3, 0.7);
        n_planetae++;
    }

    /* planetae per orbitam */
    for (int i = 0; i < num_planet; i++) {
        planeta_gen_t *p = &planetae[n_planetae];
        initia_planetam(p);

        categoria_t cat = determina_categoriam(orbitae[i], &zonae);
        switch (cat) {
        case CAT_ADUSTUM:    genera_adustum(p); break;
        case CAT_TEMPERATUM: genera_temperatum(p); break;
        case CAT_ARIDUM:     genera_aridum(p); break;
        case CAT_VENEREUM:   genera_venereum(p); break;
        case CAT_GASEOSUM:   genera_gaseosum_p(p); break;
        case CAT_GLACIALE:   genera_glaciale_p(p); break;
        case CAT_PARVUM:     genera_parvum_p(p); break;
        }

        /* communia */
        p->rotatio = alea_f() * DUO_PI;
        p->semen_p = alea();
        p->x       = alea_n(0, latitudo - 1);
        p->y       = alea_n(0, altitudo - 1);

        genera_perceptum(
            p, orbitae[i], &stella, &planetae[0],
            habet_cosidus ? &costella : NULL,
            habet_cosidus ? &planetae[1] : NULL,
            separatio, habet_cosidus,
            latitudo, altitudo
        );

        n_planetae++;
    }

    /* --- parametri campi stellarum (fundus caeli) --- */

    int    num_stellarum = 40000 + (int)(alea_f() * 110000);
    double dens_gal      = alea_inter(0.30, 0.90);
    double incl_gal      = alea_inter(0.0, 1.20);
    double lat_gal       = alea_inter(0.08, 0.25);
    double glow          = alea_inter(0.0, 1.0);
    double rift          = alea_inter(0.0, 0.8);
    int    nebulae       = alea_n(0, 8);
    int    max_sg        = alea_n(0, 3);
    int    max_gi        = alea_n(2, 10);
    int    max_ex        = alea_n(0, 2);
    int    num_gal       = alea_n(50, 400);

    /* temperatura planetarum vagantium ex sidere */
    double pl_tmin = stella.temperatura * 0.7;
    double pl_tmax = stella.temperatura * 1.1;
    if (pl_tmin < 2400)
        pl_tmin = 2400;
    if (pl_tmax < pl_tmin + 500)
        pl_tmax = pl_tmin + 500;

    /* --- emitte ISON --- */

    printf("{\n");
    printf("  \"latitudo\": %d,\n", latitudo);
    printf("  \"altitudo\": %d,\n", altitudo);
    printf("  \"numerus_stellarum\": %d,\n", num_stellarum);
    printf("  \"densitas_galaxiae\": %.2f,\n", dens_gal);
    printf("  \"inclinatio_galaxiae\": %.2f,\n", incl_gal);
    printf("  \"latitudo_galaxiae\": %.2f,\n", lat_gal);
    printf("  \"semen\": %u,\n", semen);
    printf("  \"galaxia_glow\": %.1f,\n", glow);
    printf("  \"galaxia_rift\": %.1f,\n", rift);
    printf("  \"galaxia_nebulae\": %d,\n", nebulae);
    printf("  \"max_supergigantes\": %d,\n", max_sg);
    printf("  \"max_gigantes\": %d,\n", max_gi);
    printf("  \"max_exotica\": %d,\n", max_ex);
    printf("  \"numerus_planetarum\": 0,\n");
    printf("  \"planetae_temp_min\": %d,\n", (int)pl_tmin);
    printf("  \"planetae_temp_max\": %d,\n", (int)pl_tmax);
    printf("  \"numerus_galaxiarum\": %d,\n", num_gal);
    printf("  \"max_galaxiae\": 0,\n");
    printf("\n");
    printf("  \"planetae\": [\n");

    for (int i = 0; i < n_planetae; i++)
        emitte_planetam(&planetae[i], habet_cosidus, i == n_planetae - 1);

    printf("  ]\n");
    printf("}\n");

    free(sidus_ison);
    free(cosidus_ison);
    return 0;
}
