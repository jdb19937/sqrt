/*
 * helvea.c — torus planus corrugatus, bibliotheca communis
 *
 * Implementationes superficiei, camerae, illuminationis,
 * rasterizationis. Vide helvea.h pro interfacie.
 */

#include "helvea.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ================================================================
 * superficies tori Hevea — quattuor methodi
 * ================================================================ */

const char *helvea_nomina_methodorum[HELVEA_NUMERUS_METHODORUM] = {
    "Corrugata", "Iterata", "Spiralis", "Normalis"
};

/* --- torus sine corrugationis (basis) --- */

static vec3_t torus_basis(double u, double v,
                        double radius_maior, double radius_minor)
{
    double cu = cos(u), su = sin(u);
    double cv = cos(v), sv = sin(v);
    return (vec3_t){
        (radius_maior + radius_minor * cv) * cu,
        (radius_maior + radius_minor * cv) * su,
        radius_minor * sv
    };
}

/* --- methodus 0: corrugata (originalis) --- */

static vec3_t superficies_corrugata(double u, double v,
                                  double R, double r)
{
    double corrugatio = 0.0;
    double perturb_u = 0.0, perturb_v = 0.0;
    double amp_scala = r / HELVEA_RADIUS_MINOR;

    for (int k = 0; k < HELVEA_STRATA_CORRUG; k++) {
        double freq = 7.0 * pow(2.8, (double)k);
        double amp  = 0.035 * amp_scala / pow(1.8, (double)k);

        if (k % 2 == 0) {
            corrugatio += amp * sin(freq * u + 0.5 * k);
            perturb_v  += amp * 0.15 * cos(freq * u + 0.5 * k);
        } else {
            corrugatio += amp * sin(freq * v + 0.8 * k);
            perturb_u  += amp * 0.15 * cos(freq * v + 0.8 * k);
        }

        double amp2 = amp * 0.3;
        corrugatio += amp2 * sin(freq * u + 1.3 * k)
                           * sin(freq * v + 0.9 * k);
    }

    double uu = u + perturb_u;
    double vv = v + perturb_v;
    double r_eff = r + corrugatio;

    return (vec3_t){
        (R + r_eff * cos(vv)) * cos(uu),
        (R + r_eff * cos(vv)) * sin(uu),
        r_eff * sin(vv)
    };
}

/* --- methodus 1: iterata (Borrelli — integrationes convexae) ---
 *
 * Incipit cum toro ordinario. Per iterationes errorem metricum
 * computat et corrugationes addit quae errorem minuunt.
 * Unaquaeque iteratio frequentiam duplicat et amplitudinem
 * ex errore metricum derivat.
 */

static vec3_t superficies_iterata(double u, double v,
                                double R, double r)
{
    /* torus basalis */
    double cu = cos(u), su = sin(u);
    double cv = cos(v), sv = sin(v);

    double px = (R + r * cv) * cu;
    double py = (R + r * cv) * su;
    double pz = r * sv;

    /* metricum tori: g_uu = (R + r cos v)^2, g_vv = r^2
     * metricum planum: g_uu = r^2, g_vv = r^2
     * error in directione u: (R + r cos v)^2 - r^2 */

    double amp_scala = r / HELVEA_RADIUS_MINOR;

    for (int iter = 0; iter < HELVEA_STRATA_CORRUG; iter++) {
        /* frequentia geometrice crescens */
        double freq = 8.0 * pow(3.0, (double)iter);
        double inv_freq = 1.0 / freq;

        /* error metricus in directione u ad hanc scalam */
        double g_uu = (R + r * cv) * (R + r * cv);
        double g_target = r * r;
        double error = g_uu - g_target;
        if (error < 0) error = 0;

        /* amplitudo ex errore derivata */
        double amp = 0.5 * sqrt(error) * inv_freq * amp_scala * 0.15;

        /* corrugatio in directione u (toroidali) */
        double corr_u = amp * sin(freq * u);

        /* corrugatio in directione v (poloidali) — minor */
        double amp_v = amp * 0.6;
        double corr_v = amp_v * sin(freq * v + 1.5 * iter);

        /* normam localem approximare */
        double nx = cv * cu;
        double ny = cv * su;
        double nz = sv;

        /* punctum per normam perturbere */
        double delta = corr_u + corr_v;
        px += delta * nx;
        py += delta * ny;
        pz += delta * nz;
    }

    return (vec3_t){px, py, pz};
}

/* --- methodus 2: spiralis ---
 *
 * Corrugatio per lineas spirales (u + αv) currit.
 * Character minus graticula, magis organicus.
 */

static vec3_t superficies_spiralis(double u, double v,
                                 double R, double r)
{
    double corrugatio = 0.0;
    double perturb_u = 0.0, perturb_v = 0.0;
    double amp_scala = r / HELVEA_RADIUS_MINOR;

    /* ratio aurea pro angulo spiralis */
    static const double phi = 1.6180339887;

    for (int k = 0; k < HELVEA_STRATA_CORRUG; k++) {
        double freq = 7.0 * pow(2.8, (double)k);
        double amp  = 0.035 * amp_scala / pow(1.8, (double)k);

        /* duae spirales: una dexter, altera sinister */
        double theta_d = freq * (u + phi * v) + 0.7 * k;
        double theta_s = freq * (u - phi * v) + 1.1 * k;

        corrugatio += amp * 0.5 * sin(theta_d);
        corrugatio += amp * 0.5 * sin(theta_s);

        /* perturbatio tangentialis per derivatas spiralium */
        perturb_u += amp * 0.08 * cos(theta_d);
        perturb_v += amp * 0.08 * phi * cos(theta_d);
        perturb_u += amp * 0.08 * cos(theta_s);
        perturb_v -= amp * 0.08 * phi * cos(theta_s);

        /* compositio transversa */
        double amp2 = amp * 0.25;
        corrugatio += amp2 * sin(freq * 0.7 * (u + v) + 1.3 * k)
                           * sin(freq * 0.7 * (u - v) + 0.9 * k);
    }

    double uu = u + perturb_u;
    double vv = v + perturb_v;
    double r_eff = r + corrugatio;

    return (vec3_t){
        (R + r_eff * cos(vv)) * cos(uu),
        (R + r_eff * cos(vv)) * sin(uu),
        r_eff * sin(vv)
    };
}

/* --- methodus 3: normalis ---
 *
 * Incipit cum toro basali, perturbat puncta per normam
 * superficiei. Geometrice mundius — triangula minus deformat.
 */

static vec3_t superficies_normalis(double u, double v,
                                 double R, double r)
{
    vec3_t p = torus_basis(u, v, R, r);

    /* norma tori basalis */
    double cu = cos(u), su = sin(u);
    double cv = cos(v), sv = sin(v);
    vec3_t n = vec3(cv * cu, cv * su, sv);

    double amp_scala = r / HELVEA_RADIUS_MINOR;
    double dislocatio = 0.0;

    for (int k = 0; k < HELVEA_STRATA_CORRUG; k++) {
        double freq = 7.0 * pow(2.8, (double)k);
        double amp  = 0.035 * amp_scala / pow(1.8, (double)k);

        /* corrugationes in ambabus directionibus */
        if (k % 2 == 0) {
            dislocatio += amp * sin(freq * u + 0.5 * k);
        } else {
            dislocatio += amp * sin(freq * v + 0.8 * k);
        }

        /* compositio transversa */
        double amp2 = amp * 0.3;
        dislocatio += amp2 * sin(freq * u + 1.3 * k)
                           * sin(freq * v + 0.9 * k);
    }

    return summa(p, multiplicare(n, dislocatio));
}

/* ================================================================
 * selectio methodi et norma
 * ================================================================ */

vec3_t helvea_superficies(double u, double v,
                        double radius_maior, double radius_minor,
                        helvea_methodus_t methodus)
{
    switch (methodus) {
    case HELVEA_CORRUGATA:
        return superficies_corrugata(u, v, radius_maior, radius_minor);
    case HELVEA_ITERATA:
        return superficies_iterata(u, v, radius_maior, radius_minor);
    case HELVEA_SPIRALIS:
        return superficies_spiralis(u, v, radius_maior, radius_minor);
    case HELVEA_NORMALIS:
        return superficies_normalis(u, v, radius_maior, radius_minor);
    }
    return superficies_corrugata(u, v, radius_maior, radius_minor);
}

vec3_t helvea_norma(double u, double v,
                  double radius_maior, double radius_minor,
                  helvea_methodus_t methodus)
{
    double h = 5e-5;
    vec3_t du = differentia(
        helvea_superficies(u + h, v, radius_maior, radius_minor, methodus),
        helvea_superficies(u - h, v, radius_maior, radius_minor, methodus));
    vec3_t dv = differentia(
        helvea_superficies(u, v + h, radius_maior, radius_minor, methodus),
        helvea_superficies(u, v - h, radius_maior, radius_minor, methodus));
    return normalizare(productum_vectoriale(du, dv));
}

void helvea_superficiem_computare(vec3_t *puncta, vec3_t *normae,
                                  int gradus_u, int gradus_v,
                                  double radius_maior, double radius_minor,
                                  helvea_methodus_t methodus)
{
    for (int i = 0; i <= gradus_u; i++) {
        double u = DUO_PI * (double)i / (double)gradus_u;
        for (int j = 0; j <= gradus_v; j++) {
            double v = DUO_PI * (double)j / (double)gradus_v;
            size_t idx = (size_t)i * (gradus_v + 1) + j;
            puncta[idx] = helvea_superficies(u, v, radius_maior, radius_minor, methodus);
            normae[idx] = helvea_norma(u, v, radius_maior, radius_minor, methodus);
        }
    }

    /* vertices marginales copiare ut superficies claudatur —
     * corrugatio non necessarie periodica in [0,2π] */
    for (int j = 0; j <= gradus_v; j++) {
        size_t idx_fin = (size_t)gradus_u * (gradus_v + 1) + j;
        size_t idx_ini = (size_t)0 * (gradus_v + 1) + j;
        puncta[idx_fin] = puncta[idx_ini];
        normae[idx_fin] = normae[idx_ini];
    }
    for (int i = 0; i <= gradus_u; i++) {
        size_t idx_fin = (size_t)i * (gradus_v + 1) + gradus_v;
        size_t idx_ini = (size_t)i * (gradus_v + 1) + 0;
        puncta[idx_fin] = puncta[idx_ini];
        normae[idx_fin] = normae[idx_ini];
    }
}

/* ================================================================
 * camera perspectiva
 * ================================================================ */

camera_t helvea_cameram_constituere(vec3_t positio, vec3_t scopus)
{
    camera_t cam;
    cam.positio = positio;
    cam.ante    = normalizare(differentia(scopus, positio));

    vec3_t mundi_sursum = vec3(0.0, 0.0, 1.0);
    cam.dextrum = normalizare(productum_vectoriale(cam.ante, mundi_sursum));
    cam.sursum  = productum_vectoriale(cam.dextrum, cam.ante);
    cam.focalis = 1.6;
    return cam;
}

int helvea_proicere(const camera_t *cam, vec3_t p,
                    double *scr_x, double *scr_y, double *prof,
                    int latitudo, int altitudo)
{
    vec3_t d = differentia(p, cam->positio);
    double z = productum_scalare(d, cam->ante);
    if (z < 0.05) return 0;

    double f = cam->focalis / z;
    double scala = altitudo * 0.5;

    *scr_x = latitudo * 0.5 + productum_scalare(d, cam->dextrum) * f * scala;
    *scr_y = altitudo * 0.5 - productum_scalare(d, cam->sursum)  * f * scala;
    *prof  = z;
    return 1;
}

/* ================================================================
 * illuminatio simplex (Blinn-Phong, aurum)
 * ================================================================ */

color_t helvea_illuminare(vec3_t punct, vec3_t norm, vec3_t oculus)
{
    static const vec3_t lux_dir[3] = {
        { 1.0, -0.6,  1.8},
        {-0.7,  0.8,  0.4},
        { 0.2, -1.0, -0.2}
    };
    static const color_t lux_intens[3] = {
        {0.90, 0.80, 0.65},
        {0.20, 0.30, 0.55},
        {0.12, 0.10, 0.08}
    };

    double mat_r = 0.78, mat_g = 0.55, mat_b = 0.28;
    color_t res = {mat_r * 0.06, mat_g * 0.06, mat_b * 0.06};

    vec3_t ad_oculum = normalizare(differentia(oculus, punct));
    if (productum_scalare(norm, ad_oculum) < 0.0)
        norm = multiplicare(norm, -1.0);

    for (int i = 0; i < 3; i++) {
        vec3_t ld = normalizare(lux_dir[i]);
        double NdotL = productum_scalare(norm, ld);
        if (NdotL < 0.0) NdotL = 0.0;

        vec3_t semi = normalizare(summa(ld, ad_oculum));
        double NdotH = productum_scalare(norm, semi);
        if (NdotH < 0.0) NdotH = 0.0;
        double spec = pow(NdotH, 60.0);

        res.r += mat_r * NdotL * lux_intens[i].r + spec * lux_intens[i].r * 0.45;
        res.g += mat_g * NdotL * lux_intens[i].g + spec * lux_intens[i].g * 0.45;
        res.b += mat_b * NdotL * lux_intens[i].b + spec * lux_intens[i].b * 0.45;
    }

    double fresnel = 1.0 - fabs(productum_scalare(norm, ad_oculum));
    fresnel = fresnel * fresnel * fresnel * 0.35;
    res.r += fresnel * 0.4;
    res.g += fresnel * 0.5;
    res.b += fresnel * 0.65;

    return res;
}

/* ================================================================
 * themata
 * ================================================================ */

int helvea_index_thematis = 0;

#define RAMPA_NUL {{0,0,0},{0,0,0},{0,0,0},{0,0,0}}

const int helvea_numerus_thematum = 16;

const helvea_thema_t helvea_themata[16] = {
    /* --- iridescentes --- */
    { "Oleum", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        4.0, {0.0, 2.1, 4.2}, 0.75,
        {0.15, 0.12, 0.18}, {0.40, 0.50, 0.70}, 0.55,
        50.0, 0.40, 0.03, 0, 0, RAMPA_NUL
    },
    { "Pavo", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        3.0, {3.8, 0.0, 1.2}, 0.70,
        {0.05, 0.18, 0.12}, {0.20, 0.65, 0.40}, 0.50,
        70.0, 0.50, 0.03, 0, 0, RAMPA_NUL
    },
    { "Concha", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        2.5, {0.8, 2.6, 4.8}, 0.55,
        {0.75, 0.72, 0.78}, {0.85, 0.75, 0.90}, 0.60,
        90.0, 0.55, 0.06, 0, 0, RAMPA_NUL
    },
    { "Scarabaeus", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        5.0, {2.8, 0.5, 4.0}, 0.80,
        {0.08, 0.10, 0.04}, {0.30, 0.50, 0.15}, 0.45,
        60.0, 0.45, 0.02, 0, 0, RAMPA_NUL
    },
    { "Nebula", LUX_IRIDESCENS, HELVEA_PFX_NIGRESCO,
        3.5, {4.5, 1.5, 0.0}, 0.70,
        {0.06, 0.04, 0.14}, {0.50, 0.30, 0.90}, 0.60,
        45.0, 0.50, 0.02, 0, 0, RAMPA_NUL
    },
    { "Opalis", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        6.0, {0.0, 1.5, 3.5}, 0.50,
        {0.82, 0.80, 0.85}, {0.90, 0.85, 0.95}, 0.55,
        100.0, 0.60, 0.07, 0, 0, RAMPA_NUL
    },
    { "Petroleum", LUX_IRIDESCENS, HELVEA_PFX_GRANUM,
        3.2, {3.5, 0.8, 2.0}, 0.85,
        {0.03, 0.03, 0.05}, {0.25, 0.60, 0.55}, 0.65,
        40.0, 0.35, 0.01, 0, 0, RAMPA_NUL
    },
    { "Bulla", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        7.0, {0.0, 2.1, 4.2}, 0.60,
        {0.50, 0.50, 0.55}, {0.80, 0.85, 1.00}, 0.70,
        120.0, 0.65, 0.05, 0, 0, RAMPA_NUL
    },
    /* --- rampa --- */
    { "Ignis", LUX_RAMPA, HELVEA_PFX_NULLUS,
        0, {0,0,0}, 0,
        {0,0,0}, {0.95, 0.70, 0.30}, 0.30,
        0, 0, 0.0, 0, 0,
        {{0.15, 0.02, 0.20}, {0.90, 0.15, 0.05},
         {1.00, 0.65, 0.00}, {1.00, 1.00, 0.70}}
    },
    { "Oceanus", LUX_RAMPA, HELVEA_PFX_NULLUS,
        0, {0,0,0}, 0,
        {0,0,0}, {0.60, 0.85, 0.95}, 0.35,
        0, 0, 0.0, 0, 0,
        {{0.05, 0.02, 0.15}, {0.00, 0.20, 0.55},
         {0.10, 0.75, 0.60}, {0.85, 0.95, 0.70}}
    },
    { "Autumnus", LUX_RAMPA, HELVEA_PFX_GRANUM,
        0, {0,0,0}, 0,
        {0,0,0}, {0.50, 0.35, 0.20}, 0.20,
        0, 0, 0.0, 0, 0,
        {{0.20, 0.05, 0.15}, {0.80, 0.25, 0.05},
         {0.95, 0.60, 0.10}, {0.40, 0.75, 0.15}}
    },
    { "Glacialis", LUX_RAMPA, HELVEA_PFX_NIGRESCO,
        0, {0,0,0}, 0,
        {0,0,0}, {0.80, 0.90, 1.00}, 0.45,
        0, 0, 0.0, 0, 0,
        {{0.05, 0.00, 0.20}, {0.15, 0.30, 0.70},
         {0.50, 0.80, 0.95}, {0.95, 0.95, 1.00}}
    },
    /* --- cel / tabulata --- */
    { "Pictura", LUX_TABULATA, HELVEA_PFX_LINEAE | HELVEA_PFX_POSTERIZA,
        0, {0,0,0}, 0,
        {0.95, 0.45, 0.15}, {0.10, 0.08, 0.15}, 0.20,
        20.0, 0.25, 0.08, 4, 6, RAMPA_NUL
    },
    { "Manga", LUX_TABULATA, HELVEA_PFX_LINEAE,
        0, {0,0,0}, 0,
        {0.90, 0.30, 0.50}, {0.15, 0.10, 0.20}, 0.25,
        30.0, 0.20, 0.06, 3, 0, RAMPA_NUL
    },
    /* --- mixta --- */
    { "Vitrum", LUX_IRIDESCENS, HELVEA_PFX_LINEAE | HELVEA_PFX_NIGRESCO,
        4.5, {1.0, 3.0, 5.0}, 0.65,
        {0.30, 0.30, 0.35}, {0.70, 0.75, 0.90}, 0.60,
        80.0, 0.55, 0.04, 0, 0, RAMPA_NUL
    },
    { "Somnium", LUX_IRIDESCENS,
        HELVEA_PFX_POSTERIZA | HELVEA_PFX_NIGRESCO | HELVEA_PFX_GRANUM,
        2.0, {0.5, 2.5, 4.0}, 0.60,
        {0.20, 0.15, 0.30}, {0.55, 0.40, 0.70}, 0.50,
        35.0, 0.30, 0.03, 0, 5, RAMPA_NUL
    }
};

/* luces scaenae (communes) */
static const vec3_t lux_cruda[3] = {
    { 1.0, -0.6,  1.8},
    {-0.7,  0.8,  0.4},
    { 0.2, -1.0, -0.2}
};
static const color_t lux_intens_th[3] = {
    {0.95, 0.90, 0.75},
    {0.25, 0.35, 0.60},
    {0.15, 0.12, 0.10}
};

static color_t iridescentia(double t, const helvea_thema_t *th)
{
    double phase = t * th->ir_freq * PI_GRAECUM;
    return (color_t){
        0.5 + 0.5 * cos(phase + th->ir_phase.r),
        0.5 + 0.5 * cos(phase + th->ir_phase.g),
        0.5 + 0.5 * cos(phase + th->ir_phase.b)
    };
}

static inline double tabulare(double v, int g)
{
    return floor(v * (double)g + 0.5) / (double)g;
}

color_t helvea_illuminare_thema(vec3_t punct, vec3_t norm, vec3_t oculus)
{
    const helvea_thema_t *th = &helvea_themata[helvea_index_thematis];

    vec3_t ad_oculum = normalizare(differentia(oculus, punct));
    if (productum_scalare(norm, ad_oculum) < 0.0)
        norm = multiplicare(norm, -1.0);

    double NdotV = productum_scalare(norm, ad_oculum);
    if (NdotV < 0.0) NdotV = 0.0;

    color_t mat = th->materia;
    if (th->modus == LUX_IRIDESCENS) {
        color_t ir = iridescentia(NdotV, th);
        double s = th->ir_saturatio;
        mat.r = mat.r * (1.0 - s) + ir.r * s;
        mat.g = mat.g * (1.0 - s) + ir.g * s;
        mat.b = mat.b * (1.0 - s) + ir.b * s;
    }

    if (th->modus == LUX_RAMPA) {
        vec3_t ld = normalizare(lux_cruda[0]);
        double NdotL = productum_scalare(norm, ld);
        if (NdotL < 0.0) NdotL = 0.0;

        double t3 = NdotL * 3.0;
        int seg = (int)t3;
        if (seg > 2) seg = 2;
        double f = t3 - (double)seg;
        f = f * f * (3.0 - 2.0 * f);
        f = f * f * (3.0 - 2.0 * f);

        const color_t *ca = &th->rampa[seg];
        const color_t *cb = &th->rampa[seg + 1];
        color_t res;
        res.r = ca->r * (1.0 - f) + cb->r * f;
        res.g = ca->g * (1.0 - f) + cb->g * f;
        res.b = ca->b * (1.0 - f) + cb->b * f;

        double fresnel = 1.0 - NdotV;
        fresnel = fresnel * fresnel * fresnel * th->fresnel_vis;
        res.r += fresnel * th->fresnel_color.r;
        res.g += fresnel * th->fresnel_color.g;
        res.b += fresnel * th->fresnel_color.b;
        return res;
    }

    color_t res = {mat.r * th->ambiens, mat.g * th->ambiens, mat.b * th->ambiens};

    for (int i = 0; i < 3; i++) {
        vec3_t ld = normalizare(lux_cruda[i]);
        double NdotL = productum_scalare(norm, ld);
        if (NdotL < 0.0) NdotL = 0.0;

        vec3_t semi = normalizare(summa(ld, ad_oculum));
        double NdotH = productum_scalare(norm, semi);
        if (NdotH < 0.0) NdotH = 0.0;
        double spec = pow(NdotH, th->spec_potentia);

        if (th->modus == LUX_TABULATA) {
            NdotL = tabulare(NdotL, th->cel_gradus);
            spec  = (NdotH > 0.9) ? 1.0 : 0.0;
        }
        if (th->modus == LUX_PLANUS)
            NdotL = NdotL * 0.5 + 0.5;

        res.r += mat.r * NdotL * lux_intens_th[i].r
               + spec * lux_intens_th[i].r * th->spec_vis;
        res.g += mat.g * NdotL * lux_intens_th[i].g
               + spec * lux_intens_th[i].g * th->spec_vis;
        res.b += mat.b * NdotL * lux_intens_th[i].b
               + spec * lux_intens_th[i].b * th->spec_vis;
    }

    double fresnel = 1.0 - NdotV;
    fresnel = fresnel * fresnel * fresnel * th->fresnel_vis;
    res.r += fresnel * th->fresnel_color.r;
    res.g += fresnel * th->fresnel_color.g;
    res.b += fresnel * th->fresnel_color.b;

    return res;
}

/* ================================================================
 * tabula imaginis
 * ================================================================ */

void helvea_tabulam_purgare(helvea_tabula_t *t)
{
    size_t n_pix = (size_t)t->latitudo * t->altitudo;

    for (size_t i = 0; i < n_pix; i++)
        t->profunditatis[i] = 1e30;

    if (t->bytes_pixel == 4) {
        for (size_t i = 0; i < n_pix; i++) {
            size_t base = i * 4;
            t->imaginis[base + 0] = 14;   /* B */
            t->imaginis[base + 1] = 8;    /* G */
            t->imaginis[base + 2] = 8;    /* R */
            t->imaginis[base + 3] = 255;  /* A */
        }
    } else {
        for (size_t i = 0; i < n_pix; i++) {
            size_t base = i * 3;
            t->imaginis[base + 0] = 8;
            t->imaginis[base + 1] = 8;
            t->imaginis[base + 2] = 14;
        }
    }
}

void helvea_pixel_rgb(helvea_tabula_t *t, int x, int y,
                      double prof, color_t c)
{
    if (x < 0 || x >= t->latitudo || y < 0 || y >= t->altitudo) return;
    int idx = y * t->latitudo + x;
    if (prof >= t->profunditatis[idx]) return;
    t->profunditatis[idx] = prof;

    int base = idx * 3;
    t->imaginis[base + 0] = gamma_corrigere(c.r);
    t->imaginis[base + 1] = gamma_corrigere(c.g);
    t->imaginis[base + 2] = gamma_corrigere(c.b);
}

void helvea_pixel_bgra(helvea_tabula_t *t, int x, int y,
                       double prof, color_t c)
{
    if (x < 0 || x >= t->latitudo || y < 0 || y >= t->altitudo) return;
    int idx = y * t->latitudo + x;
    if (prof >= t->profunditatis[idx]) return;
    t->profunditatis[idx] = prof;

    int base = idx * 4;
    t->imaginis[base + 0] = gamma_corrigere(c.b);
    t->imaginis[base + 1] = gamma_corrigere(c.g);
    t->imaginis[base + 2] = gamma_corrigere(c.r);
    t->imaginis[base + 3] = 255;
}

/* ================================================================
 * fundum bitmap toroidale
 * ================================================================ */

void helvea_fundum_implere(helvea_tabula_t *t,
                           const unsigned char *bitmap,
                           int bm_latitudo, int bm_altitudo,
                           int delta_x, int delta_y)
{
    /* delta in [0, bm) normalizare */
    delta_x = ((delta_x % bm_latitudo) + bm_latitudo) % bm_latitudo;
    delta_y = ((delta_y % bm_altitudo) + bm_altitudo) % bm_altitudo;

    for (int y = 0; y < t->altitudo; y++) {
        int by = (y + delta_y) % bm_altitudo;
        for (int x = 0; x < t->latitudo; x++) {
            int bx = (x + delta_x) % bm_latitudo;
            int bm_idx = (by * bm_latitudo + bx) * 3;
            unsigned char r = bitmap[bm_idx + 0];
            unsigned char g = bitmap[bm_idx + 1];
            unsigned char b = bitmap[bm_idx + 2];

            size_t idx = (size_t)y * t->latitudo + x;
            if (t->bytes_pixel == 4) {
                size_t base = idx * 4;
                t->imaginis[base + 0] = b;
                t->imaginis[base + 1] = g;
                t->imaginis[base + 2] = r;
                t->imaginis[base + 3] = 255;
            } else {
                size_t base = idx * 3;
                t->imaginis[base + 0] = r;
                t->imaginis[base + 1] = g;
                t->imaginis[base + 2] = b;
            }
        }
    }
}

/* ================================================================
 * rasterizatio triangulorum
 * ================================================================ */

void helvea_triangulum_reddere(
    helvea_tabula_t *t,
    double sx0, double sy0, double sz0, vec3_t p0, vec3_t n0,
    double sx1, double sy1, double sz1, vec3_t p1, vec3_t n1,
    double sx2, double sy2, double sz2, vec3_t p2, vec3_t n2,
    vec3_t oculus,
    helvea_illuminare_fn illum_fn,
    helvea_pixel_fn pixel_fn)
{
    double area = (sx1 - sx0) * (sy2 - sy0) - (sx2 - sx0) * (sy1 - sy0);
    if (fabs(area) < 0.5) return;
    /* backface culling */
    if (area > 0.0) return;
    double inv_area = 1.0 / area;

    int min_x = (int)floor(fmin(fmin(sx0, sx1), sx2));
    int max_x = (int)ceil (fmax(fmax(sx0, sx1), sx2));
    int min_y = (int)floor(fmin(fmin(sy0, sy1), sy2));
    int max_y = (int)ceil (fmax(fmax(sy0, sy1), sy2));

    if (min_x < 0) min_x = 0;
    if (max_x >= t->latitudo) max_x = t->latitudo - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= t->altitudo) max_y = t->altitudo - 1;

    for (int y = min_y; y <= max_y; y++) {
        double py = y + 0.5;
        for (int x = min_x; x <= max_x; x++) {
            double px = x + 0.5;

            double w0 = ((sx1 - px) * (sy2 - py) - (sx2 - px) * (sy1 - py))
                        * inv_area;
            double w1 = ((sx2 - px) * (sy0 - py) - (sx0 - px) * (sy2 - py))
                        * inv_area;
            double w2 = 1.0 - w0 - w1;

            if (w0 < -0.001 || w1 < -0.001 || w2 < -0.001) continue;

            double z = w0 * sz0 + w1 * sz1 + w2 * sz2;

            vec3_t punct = summa(summa(multiplicare(p0, w0),
                                     multiplicare(p1, w1)),
                               multiplicare(p2, w2));
            vec3_t norm  = normalizare(
                         summa(summa(multiplicare(n0, w0),
                                     multiplicare(n1, w1)),
                               multiplicare(n2, w2)));

            color_t c = illum_fn(punct, norm, oculus);
            pixel_fn(t, x, y, z, c);
        }
    }
}

/* ================================================================
 * scaenam reddere
 * ================================================================ */

void helvea_scaenam_reddere(
    helvea_tabula_t *t,
    const vec3_t *puncta, const vec3_t *normae,
    int gradus_u, int gradus_v,
    const camera_t *cam,
    helvea_illuminare_fn illum_fn,
    helvea_pixel_fn pixel_fn)
{
    for (int i = 0; i < gradus_u; i++) {
        for (int j = 0; j < gradus_v; j++) {
            size_t vi[4] = {
                (size_t)i       * (gradus_v + 1) + j,
                (size_t)(i + 1) * (gradus_v + 1) + j,
                (size_t)(i + 1) * (gradus_v + 1) + (j + 1),
                (size_t)i       * (gradus_v + 1) + (j + 1)
            };

            double sx[4], sy[4], sz[4];
            int omnes_visibiles = 1;
            for (int k = 0; k < 4; k++) {
                if (!helvea_proicere(cam, puncta[vi[k]],
                                     &sx[k], &sy[k], &sz[k],
                                     t->latitudo, t->altitudo)) {
                    omnes_visibiles = 0;
                    break;
                }
            }
            if (!omnes_visibiles) continue;

            helvea_triangulum_reddere(t,
                sx[0], sy[0], sz[0], puncta[vi[0]], normae[vi[0]],
                sx[1], sy[1], sz[1], puncta[vi[1]], normae[vi[1]],
                sx[2], sy[2], sz[2], puncta[vi[2]], normae[vi[2]],
                cam->positio, illum_fn, pixel_fn);

            helvea_triangulum_reddere(t,
                sx[0], sy[0], sz[0], puncta[vi[0]], normae[vi[0]],
                sx[2], sy[2], sz[2], puncta[vi[2]], normae[vi[2]],
                sx[3], sy[3], sz[3], puncta[vi[3]], normae[vi[3]],
                cam->positio, illum_fn, pixel_fn);
        }
    }
}
