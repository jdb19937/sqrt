/*
 * helvea.c — torus planus corrugatus, bibliotheca communis
 *
 * Implementationes superficiei, camerae, illuminationis,
 * rasterizationis. Vide helvea.h pro interfacie.
 */

#include "helvea.h"
#include "bessel.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ================================================================
 * superficies tori Hevea — tres methodi
 * ================================================================ */

const char *helvea_nomina_methodorum[HELVEA_NUMERUS_METHODORUM] = {
    "Borrelli", "Borrelliᵀ", "Planus"
};

int helvea_strata = 0; /* 0 = praefinitum per methodum */

/* --- torus revolutionis ordinarius (basis omnium methodorum) ---
 *
 * Parametrizatio canonica:
 *   f(u,v) = ((R + r cos v) cos u, (R + r cos v) sin u, r sin v)
 *
 * ubi u ∈ [0,2π) est angulus toroidalis (circa circulum magnum
 * radii R) et v ∈ [0,2π) est angulus poloidalis (circa tubum
 * radii r).
 *
 * Metricum inductum (prima forma fundamentalis):
 *   g_uu = (R + r cos v)²,  g_uv = 0,  g_vv = r²
 *
 * Hoc NON est isometricum toro plano [0,L)×[0,L) cum metrico
 * Euclidiano ds² = dx² + dy². Error metricus principalis est in
 * directione u: g_uu variat cum v, dum isometria g_uu = constans
 * requirit. Circa equatorem (v=0) torus "nimis longus" est;
 * circa interiorem (v=π) "nimis brevis".
 */

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

/* ================================================================
 * methodus Borrelli — approximatio immersionis Nash-Kuiper
 *
 * Referentiae:
 *
 *   Nash, J. (1954). "C¹ isometric imbeddings."
 *     Annals of Mathematics 60(3):383-396. doi:10.2307/1969840
 *     Demonstravit omnem varietatem Riemannianam brevem in Rⁿ
 *     isometrice immergi posse per immersionem C¹ dummodo n
 *     sufficiat. Resultatum paradoxale: superficies cum metrico
 *     arbitrario in spatium parvo immergi potest.
 *
 *   Kuiper, N.H. (1955). "On C¹-isometric imbeddings. I, II."
 *     Indag. Math. 17:545-556, 683-689.
 *     Extendit Nash ad codimensionem 1: sufficit R³ pro
 *     superficie. Ergo torus planus T² in R³ immergi potest
 *     — sed constructio non-explicita.
 *
 *   Gromov, M. (1986). "Partial Differential Relations."
 *     Springer. Introduxit integrationem convexam ut
 *     framework generalem pro h-principio: relationes
 *     differentialium partialium sub-determinatae saepe
 *     solutiones habent dummodo conditiones topologicae
 *     satisfiant.
 *
 *   Borrelli, V.; Jabrane, S.; Lazarus, F.; Thibert, B. (2012).
 *     "Flat tori in three-dimensional space and convex
 *     integration." PNAS 109(19):7218-7223.
 *     doi:10.1073/pnas.1118478109
 *     Prima visualizatio concreta immersionis Nash-Kuiper
 *     tori plani. Convertunt integrationem convexam in
 *     algorithmum explicitum. Codex: hevea-project.fr
 *
 * Fundamenta mathematica:
 *
 *   EMBEDDING BREVIS vs ISOMETRICA:
 *     Embedding f: M → R³ est "brevis" si |df(v)| < |v| pro
 *     omni v — distantias comprimit. Est "isometrica" si
 *     |df(v)| = |v| — distantias conservat. Nash-Kuiper
 *     demonstrat: si embedding brevis existit, embedding
 *     isometrica C¹ etiam existit (sed non C²!).
 *
 *   C¹ vs C²:
 *     Immersio C² tori plani in R³ impossibilis est: tensor
 *     curvaturaee est invarians isometricus C² varietatum
 *     Riemannianum (Theorema Egregium Gauss). Torus planus
 *     curvaturam K=0 ubique habet, sed superficies clausa
 *     in R³ puncta curaturae positivae habere debet. Ergo
 *     nulla immersio C² existit. Sed in regularitate C¹
 *     curvatura sensum amittit — vector normalis fractale
 *     exhibet et Gauss obstructio evanescit.
 *
 *   INTEGRATIO CONVEXA (algorithmus):
 *     Error metricus Δ = g_planum - f*g_Euclid decomponitur:
 *       Δ = Σⱼ ρⱼ lⱼ⊗lⱼ    (ρⱼ > 0)
 *     ubi lⱼ sunt 1-formae lineares quae conum positivum
 *     formarum bilinearium symmetricarum generant. Sufficiunt
 *     tres: S=3 per totum processum (Borrelli §2).
 *
 *     Pro unaquaque directione lⱼ, embedding corrugatur:
 *     punctum novum computatur per integrationem ODE cuius
 *     integrandus est:
 *
 *       h(s,t) = r · (cos(θ)·t + sin(θ)·n)
 *
 *     ubi θ(q,t) = α(q)·cos(2πNt) et:
 *       t = df(W)/|df(W)| — directio tangentialis normalizata
 *       n — normalis superficiei
 *       W = U + ζV — directio orthogonalizata
 *       r = √(|df(W)|² + ρⱼ/(V·V)) — radius corrugationis
 *       α = J₀⁻¹(|df(W)|/r) — angulus per inversam Bessel
 *
 *   CUR J₀:
 *     Post integrationem per unam periodum, derivata directio-
 *     nalis fit: df_new(W) = r·J₀(α)·t (Borrelli Thm 1 ii).
 *     Hoc quia ∫₀¹ cos(α·cos(2πt)) dt = J₀(α). Angulus α
 *     eligitur per J₀⁻¹ ut |df_new(W)| valorem desideratum
 *     attingat — errorem metricum in directione lⱼ corrigens.
 *
 *   FREQUENTIAE (ex codice Hevea et charta):
 *     N₁=12, N₂=80, N₃=500. Quarta N₄=9000 (charta §3).
 *     Frequentia crescens: C⁰ proximitas ‖f-f₀‖ ≤ C/N
 *     (Lemma 1). Amplitudo decrescit cum frequentia crescit.
 *     Quinta corrugatio amplitudines tam parvas habet ut
 *     invisibiles sint (hevea-project.fr).
 *
 *   CONVERGENTIA:
 *     Error Δₖ → 0 exponentialiter per δₖ = 1 - e⁻ᵞᵏ.
 *     In limite: embedding isometrica C¹. Vector normalis
 *     "fractale" exhibet — auto-similitudinem asymptoticam
 *     per producta infinita matricum rotationis (Borrelli §3).
 *
 *   DIRECTIONES in codice Hevea:
 *     l₁=(1,0), l₂=(1/√5,2/√5), l₃=(1/√5,-2/√5)
 *     cum nucleis V₁=(0,1), V₂=(-2,1), V₃=(2,1).
 *     (Charta descriptionem generalem dat cum directionibus
 *     U⁽¹⁾=(1,1), U⁽²⁾=(1,-1), U⁽³⁾=(0,√3); codex
 *     implementationem specifice optimatam adhibet.)
 *
 * Haec implementatio:
 *
 *   Approximatio closed-form — non integrat ODE sed
 *   dislocatio-nem per normam superficiei directe computat.
 *   Character essentialis servatur:
 *   - tres phasae in tribus directionibus (cyclantes ultra 3)
 *   - forma undae per J₀⁻¹ (non purus sinus)
 *   - amplitudo modulata per errorem metricum localem
 *   - frequentiae ex codice originali
 *
 *   Differentia a vero algorithmo: amplitudines calibratae
 *   sunt visualiter, non ex ODE integratione derivatae.
 *   Superficies non est vere isometrica sed characterem
 *   visibilem corrugationis Nash-Kuiper reddit.
 *
 * Transposita (HELVEA_BORRELLI_T):
 *
 *   Codex Hevea parametros (x,y) = (poloidalis, toroidalis)
 *   adhibet. Nostrum systema: u = toroidalis, v = poloidalis.
 *   HELVEA_BORRELLI transponit directiones ut cum imaginibus
 *   publicatis concordet. HELVEA_BORRELLI_T adhibet directiones
 *   non transpositas — orientatio altera, aeque valida.
 * ================================================================ */

static vec3_t superficies_borrelli(double u, double v,
                                   double R, double r,
                                   int transpone)
{
    vec3_t p = torus_basis(u, v, R, r);

    double cu = cos(u), su = sin(u);
    double cv = cos(v), sv = sin(v);
    vec3_t n = vec3(cv * cu, cv * su, sv);

    /*
     * Error metricus tori revolutionis:
     * g_uu = (R + r cos v)², g_vv = r², g_uv = 0
     * Metricum planum (target): g_uu = g_vv = constans
     * Error principalis in directione u: (R + r cos v)² - r²
     *
     * In vero algorithmum, h = r·(cos(α·β)·z + sin(α·β)·n)
     * est integrandus (derivata embedding). Integratio per
     * unam periodum producit oscillationem amplitudinis ~r/(2πN).
     * Hic approximamus displacementum directe.
     */

    /* tres directiones corrugationis (in spatio parametrico [0,2π]²)
     *
     * Hevea codex parametros (x,y) = (poloidalis, toroidalis) adhibet.
     * Nostrum systema: u = toroidalis, v = poloidalis.
     * Ergo directiones transponendae: Hevea (a,b) → nostrum (b,a).
     *
     * Hevea: l₁=(1,0), l₂=(1/√5,2/√5), l₃=(1/√5,-2/√5)
     * Nostrum: l₁=(0,1), l₂=(2/√5,1/√5), l₃=(-2/√5,1/√5)
     */
    /* orientatio Hevea (transpone=0) vel transposita (transpone=1) */
    static const double hevea_u[3] = { 0.0,   2.0 / 2.236, -2.0 / 2.236 };
    static const double hevea_v[3] = { 1.0,   1.0 / 2.236,  1.0 / 2.236 };
    static const double trans_u[3] = { 1.0,   1.0 / 2.236,  1.0 / 2.236 };
    static const double trans_v[3] = { 0.0,   2.0 / 2.236, -2.0 / 2.236 };
    const double *dir_u = transpone ? trans_u : hevea_u;
    const double *dir_v = transpone ? trans_v : hevea_v;

    /* oscillationes per phasam.
     * Primae tres ex codice originali (12, 80, 500).
     * Ulteriores geometrice crescunt (×6.5 per phasam). */
    static const double N_oscil[HELVEA_STRATA_MAX] = {
        12.0, 80.0, 500.0, 9000.0, 60000.0, 400000.0, 2600000.0
    };

    /* amplitudo per phasam — decrescens */
    static const double amp_basis[HELVEA_STRATA_MAX] = {
        0.15, 0.025, 0.004, 0.0007, 0.00012, 0.00002, 0.0000035
    };

    int strata = helvea_strata > 0 ? helvea_strata : 3;
    if (strata > HELVEA_STRATA_MAX) strata = HELVEA_STRATA_MAX;

    double dislocatio = 0.0;

    for (int phase = 0; phase < strata; phase++) {
        double lu = dir_u[phase % 3];
        double lv = dir_v[phase % 3];
        double N = N_oscil[phase];

        /* parameter per directionem corrugationis */
        double t = lu * u + lv * v;

        /* error metricus modulat amplitudinem localiter:
         * corrugatio fortior ubi torus ab isometria magis deviat
         * (circa equatorem, ubi R + r cos v maximus) */
        double stretch = (R + r * cv) / (R + r);
        double amp = amp_basis[phase] * r * (0.3 + 0.7 * stretch);

        /* angulus per inversam Bessel J₀:
         * determinat formam undae — non purus sinus sed
         * "corrugatio truncata" characteristica Borrelli.
         * Ratio |z|/r_corr ex errore metrico. */
        double rho = stretch * stretch;
        double alpha = bessel_j0_inversa(0.3 + 0.6 * rho);

        /* oscillatio */
        double beta = cos(N * t);
        dislocatio += amp * sin(alpha * beta);
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
    case HELVEA_BORRELLI:
        return superficies_borrelli(u, v, radius_maior, radius_minor, 0);
    case HELVEA_BORRELLI_T:
        return superficies_borrelli(u, v, radius_maior, radius_minor, 1);
    case HELVEA_PLANUS:
        return torus_basis(u, v, radius_maior, radius_minor);
    }
    return torus_basis(u, v, radius_maior, radius_minor);
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
        {0.90, 0.80, 0.65, 1.0},
        {0.20, 0.30, 0.55, 1.0},
        {0.12, 0.10, 0.08, 1.0}
    };

    double mat_r = 0.78, mat_g = 0.55, mat_b = 0.28;
    color_t res = {mat_r * 0.06, mat_g * 0.06, mat_b * 0.06, 1.0};

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

#define RAMPA_NUL {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}

const int helvea_numerus_thematum = 16;

const helvea_thema_t helvea_themata[16] = {
    /* --- mixta (primum) --- */
    { "Vitrum", LUX_IRIDESCENS, HELVEA_PFX_LINEAE | HELVEA_PFX_NIGRESCO,
        4.5, {1.0, 3.0, 5.0, 1.0}, 0.65,
        {0.30, 0.30, 0.35, 1.0}, {0.70, 0.75, 0.90, 1.0}, 0.60,
        80.0, 0.55, 0.04, 0, 0, RAMPA_NUL
    },
    /* --- iridescentes --- */
    { "Oleum", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        4.0, {0.0, 2.1, 4.2, 1.0}, 0.75,
        {0.15, 0.12, 0.18, 1.0}, {0.40, 0.50, 0.70, 1.0}, 0.55,
        50.0, 0.40, 0.03, 0, 0, RAMPA_NUL
    },
    { "Pavo", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        3.0, {3.8, 0.0, 1.2, 1.0}, 0.70,
        {0.05, 0.18, 0.12, 1.0}, {0.20, 0.65, 0.40, 1.0}, 0.50,
        70.0, 0.50, 0.03, 0, 0, RAMPA_NUL
    },
    { "Concha", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        2.5, {0.8, 2.6, 4.8, 1.0}, 0.55,
        {0.75, 0.72, 0.78, 1.0}, {0.85, 0.75, 0.90, 1.0}, 0.60,
        90.0, 0.55, 0.06, 0, 0, RAMPA_NUL
    },
    { "Scarabaeus", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        5.0, {2.8, 0.5, 4.0, 1.0}, 0.80,
        {0.08, 0.10, 0.04, 1.0}, {0.30, 0.50, 0.15, 1.0}, 0.45,
        60.0, 0.45, 0.02, 0, 0, RAMPA_NUL
    },
    { "Nebula", LUX_IRIDESCENS, HELVEA_PFX_NIGRESCO,
        3.5, {4.5, 1.5, 0.0, 1.0}, 0.70,
        {0.06, 0.04, 0.14, 1.0}, {0.50, 0.30, 0.90, 1.0}, 0.60,
        45.0, 0.50, 0.02, 0, 0, RAMPA_NUL
    },
    { "Opalis", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        6.0, {0.0, 1.5, 3.5, 1.0}, 0.50,
        {0.82, 0.80, 0.85, 1.0}, {0.90, 0.85, 0.95, 1.0}, 0.55,
        100.0, 0.60, 0.07, 0, 0, RAMPA_NUL
    },
    { "Petroleum", LUX_IRIDESCENS, HELVEA_PFX_GRANUM,
        3.2, {3.5, 0.8, 2.0, 1.0}, 0.85,
        {0.03, 0.03, 0.05, 1.0}, {0.25, 0.60, 0.55, 1.0}, 0.65,
        40.0, 0.35, 0.01, 0, 0, RAMPA_NUL
    },
    { "Bulla", LUX_IRIDESCENS, HELVEA_PFX_NULLUS,
        7.0, {0.0, 2.1, 4.2, 1.0}, 0.60,
        {0.50, 0.50, 0.55, 1.0}, {0.80, 0.85, 1.00, 1.0}, 0.70,
        120.0, 0.65, 0.05, 0, 0, RAMPA_NUL
    },
    /* --- rampa --- */
    { "Ignis", LUX_RAMPA, HELVEA_PFX_NULLUS,
        0, {0,0,0, 1.0}, 0,
        {0,0,0, 1.0}, {0.95, 0.70, 0.30, 1.0}, 0.30,
        0, 0, 0.0, 0, 0,
        {{0.15, 0.02, 0.20, 1.0}, {0.90, 0.15, 0.05, 1.0},
         {1.00, 0.65, 0.00, 1.0}, {1.00, 1.00, 0.70, 1.0}}
    },
    { "Oceanus", LUX_RAMPA, HELVEA_PFX_NULLUS,
        0, {0,0,0, 1.0}, 0,
        {0,0,0, 1.0}, {0.60, 0.85, 0.95, 1.0}, 0.35,
        0, 0, 0.0, 0, 0,
        {{0.05, 0.02, 0.15, 1.0}, {0.00, 0.20, 0.55, 1.0},
         {0.10, 0.75, 0.60, 1.0}, {0.85, 0.95, 0.70, 1.0}}
    },
    { "Autumnus", LUX_RAMPA, HELVEA_PFX_GRANUM,
        0, {0,0,0, 1.0}, 0,
        {0,0,0, 1.0}, {0.50, 0.35, 0.20, 1.0}, 0.20,
        0, 0, 0.0, 0, 0,
        {{0.20, 0.05, 0.15, 1.0}, {0.80, 0.25, 0.05, 1.0},
         {0.95, 0.60, 0.10, 1.0}, {0.40, 0.75, 0.15, 1.0}}
    },
    { "Glacialis", LUX_RAMPA, HELVEA_PFX_NIGRESCO,
        0, {0,0,0, 1.0}, 0,
        {0,0,0, 1.0}, {0.80, 0.90, 1.00, 1.0}, 0.45,
        0, 0, 0.0, 0, 0,
        {{0.05, 0.00, 0.20, 1.0}, {0.15, 0.30, 0.70, 1.0},
         {0.50, 0.80, 0.95, 1.0}, {0.95, 0.95, 1.00, 1.0}}
    },
    /* --- cel / tabulata --- */
    { "Pictura", LUX_TABULATA, HELVEA_PFX_LINEAE | HELVEA_PFX_POSTERIZA,
        0, {0,0,0, 1.0}, 0,
        {0.95, 0.45, 0.15, 1.0}, {0.10, 0.08, 0.15, 1.0}, 0.20,
        20.0, 0.25, 0.08, 4, 6, RAMPA_NUL
    },
    { "Manga", LUX_TABULATA, HELVEA_PFX_LINEAE,
        0, {0,0,0, 1.0}, 0,
        {0.90, 0.30, 0.50, 1.0}, {0.15, 0.10, 0.20, 1.0}, 0.25,
        30.0, 0.20, 0.06, 3, 0, RAMPA_NUL
    },
    /* --- mixta --- */
    { "Somnium", LUX_IRIDESCENS,
        HELVEA_PFX_POSTERIZA | HELVEA_PFX_NIGRESCO | HELVEA_PFX_GRANUM,
        2.0, {0.5, 2.5, 4.0, 1.0}, 0.60,
        {0.20, 0.15, 0.30, 1.0}, {0.55, 0.40, 0.70, 1.0}, 0.50,
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
    {0.95, 0.90, 0.75, 1.0},
    {0.25, 0.35, 0.60, 1.0},
    {0.15, 0.12, 0.10, 1.0}
};

static color_t iridescentia(double t, const helvea_thema_t *th)
{
    double phase = t * th->ir_freq * PI_GRAECUM;
    return (color_t){
        0.5 + 0.5 * cos(phase + th->ir_phase.r),
        0.5 + 0.5 * cos(phase + th->ir_phase.g),
        0.5 + 0.5 * cos(phase + th->ir_phase.b),
        1.0
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

    color_t res = {mat.r * th->ambiens, mat.g * th->ambiens, mat.b * th->ambiens, 1.0};

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

