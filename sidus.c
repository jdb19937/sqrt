/*
 * sidus.c — sidera singula, renderer
 *
 * Reddit singula sidera in fenestra 64x64.
 */

#include "sidus.h"
#include "instrumentum.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI_GRAECUM 3.14159265358979323846
#define DUO_PI     (2.0 * PI_GRAECUM)

#define FEN ASTRA_FENESTRA
#define SEMI (FEN / 2)

const char *astra_nomina_generum[SIDUS_NUMERUS] = {
    "Nanum Album",
    "Sequentia",
    "Gigas Rubrum",
    "Supergigas",
    "Neutronium",
    "Crystallinum",
    "Magnetar",
    "Galaxia",
    "Vagans"
};

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

/* Gaussiana per Box-Muller */
static double alea_gauss(void)
{
    double u1 = alea_f() + 1e-30;
    double u2 = alea_f();
    return sqrt(-2.0 * log(u1)) * cos(DUO_PI * u2);
}

/* ================================================================
 * temperatura ad colorem (approximatio Planck)
 * ================================================================ */

color_t astra_temperatura_ad_colorem(double kelvin)
{
    /* Approximatio Tanner Helland (2012) functionis Planckianae
     * per CIE 1931 2° standard observer.
     * Exacta intra 1% pro 1000K-40000K contra integrationem
     * spectri Planckiani B(λ,T) = 2hc²/λ⁵ · 1/(e^(hc/λkT)-1)
     * convolutam cum x̄(λ), ȳ(λ), z̄(λ) et transformatam per
     * sRGB matricem (Rec. 709 primariis). */
    double t = kelvin / 100.0;
    double r, g, b;

    if (t <= 66.0) {
        r = 255.0;
        g = 99.4708 * log(t) - 161.1196;
        if (g < 0) g = 0; if (g > 255) g = 255;
    } else {
        r = 329.699 * pow(t - 60.0, -0.1332);
        if (r < 0) r = 0; if (r > 255) r = 255;
        g = 288.122 * pow(t - 60.0, -0.0755);
        if (g < 0) g = 0; if (g > 255) g = 255;
    }

    if (t >= 66.0) {
        b = 255.0;
    } else if (t <= 19.0) {
        b = 0.0;
    } else {
        b = 138.518 * log(t - 10.0) - 305.045;
        if (b < 0) b = 0; if (b > 255) b = 255;
    }

    return (color_t){r / 255.0, g / 255.0, b / 255.0, 1.0};
}

/* ================================================================
 * sidus reddere
 * ================================================================ */

/* punctum Gaussianum in fenestra scribere */
static void fen_punctum(unsigned char *fen, double cx, double cy,
                        double radius, color_t col, double intensitas)
{
    int r0 = (int)(cy - radius * 3) - 1;
    int r1 = (int)(cy + radius * 3) + 2;
    int c0 = (int)(cx - radius * 3) - 1;
    int c1 = (int)(cx + radius * 3) + 2;
    if (r0 < 0) r0 = 0; if (r1 >= FEN) r1 = FEN - 1;
    if (c0 < 0) c0 = 0; if (c1 >= FEN) c1 = FEN - 1;

    double inv_r2 = 1.0 / (radius * radius + 0.01);

    for (int y = r0; y <= r1; y++) {
        for (int x = c0; x <= c1; x++) {
            double dx = x - cx, dy = y - cy;
            double d2 = dx * dx + dy * dy;
            double f = intensitas * exp(-d2 * inv_r2 * 0.5);
            if (f < 0.002) continue;

            int idx = (y * FEN + x) * 4;
            int r = (int)(fen[idx + 0] + col.r * f * 255);
            int g = (int)(fen[idx + 1] + col.g * f * 255);
            int b = (int)(fen[idx + 2] + col.b * f * 255);
            int a = (int)(fen[idx + 3] + f * 255);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            if (a > 255) a = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = (unsigned char)a;
        }
    }
}

/*
 * Spicula diffractionis — ex theoria Fraunhofer.
 * Aperturam non-circularem (obstructio araneo secundario)
 * PSF cum spiculis generat: Airy pattern convolutus cum
 * transformatione Fourier structurae aperturae.
 *
 * Newtonianum (4 bracchia): 4 spiculae orthogonales.
 * JWST (3 bracchia segmentorum hex): 6 spiculae principales + 2 minores.
 * Intensitas spicula ∝ 1/r (non exp), cum oscillationibus Airy.
 */
static void fen_spicula(unsigned char *fen, double cx, double cy,
                        double angulus, double longitudo, double latitudo,
                        color_t col, double intensitas)
{
    double dx = cos(angulus);
    double dy = sin(angulus);
    int n = (int)(longitudo * 3);
    if (n < 6) n = 6;

    for (int i = -n; i <= n; i++) {
        double t = (double)i / (double)n;
        double dist = fabs(t) * longitudo;

        /* profilo 1/r cum oscillationibus Airy */
        double env = intensitas / (1.0 + dist * 0.4);
        double airy = 1.0 + 0.3 * cos(dist * 2.5);
        double f = env * airy;
        if (f < 0.002) continue;

        double px = cx + t * longitudo * dx;
        double py = cy + t * longitudo * dy;
        fen_punctum(fen, px, py, latitudo, col, f);
    }
}

/* ================================================================
 * renderers per genus
 * ================================================================ */

static void reddere_nanum_album(unsigned char *fen,
                                const astra_sidus_t *s,
                                const astra_instrumentum_t *instr)
{
    color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.0;

    /* nucleus intensissimus, minimus */
    fen_punctum(fen, SEMI, SEMI, 0.6, col, luciditas * 1.5);

    /* halo tenuis caeruleus */
    color_t halo_col = {col.r * 0.6, col.g * 0.7, col.b * 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 2.5, halo_col, luciditas * 0.3);

    (void)instr;
}

static void reddere_sequentia(unsigned char *fen,
                              const astra_sidus_t *s,
                              const astra_instrumentum_t *instr)
{
    color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.0;

    /* nucleus */
    double r_nucl = 0.8 + luciditas * 0.3;
    if (r_nucl > 3.0) r_nucl = 3.0;
    fen_punctum(fen, SEMI, SEMI, r_nucl, col, luciditas);

    /* halo levis */
    if (luciditas > 0.3)
        fen_punctum(fen, SEMI, SEMI, r_nucl * 3.0, col, luciditas * 0.08);

    /* spiculae instrumenti */
    if (instr->spiculae > 0 && luciditas > 0.2) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                       + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(fen, SEMI, SEMI, ang,
                        instr->spiculae_long * luciditas,
                        0.4, col, luciditas * 0.25);
        }
    }

    /* halo instrumenti */
    if (instr->halo_vis > 0.01)
        fen_punctum(fen, SEMI, SEMI, instr->halo_radius,
                    col, instr->halo_vis * luciditas);
}

static void reddere_gigas_rubrum(unsigned char *fen,
                                 const astra_sidus_t *s,
                                 const astra_instrumentum_t *instr)
{
    color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.5;

    /* discus maior, limbi obscuriores (limb darkening) */
    double r_disc = 2.5 + luciditas * 1.5;
    if (r_disc > 8.0) r_disc = 8.0;

    /* centrum lucidum */
    fen_punctum(fen, SEMI, SEMI, r_disc * 0.4, col, luciditas * 1.2);
    /* discus latus */
    fen_punctum(fen, SEMI, SEMI, r_disc, col, luciditas * 0.5);
    /* halo rubrum */
    color_t halo = {col.r, col.g * 0.5, col.b * 0.3, 1.0};
    fen_punctum(fen, SEMI, SEMI, r_disc * 2.0, halo, luciditas * 0.06);

    /* spiculae */
    if (instr->spiculae > 0 && luciditas > 0.3) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                       + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(fen, SEMI, SEMI, ang,
                        instr->spiculae_long * luciditas * 0.7,
                        0.5, col, luciditas * 0.15);
        }
    }
}

static void reddere_supergigas(unsigned char *fen,
                               const astra_sidus_t *s,
                               const astra_instrumentum_t *instr)
{
    color_t col = astra_temperatura_ad_colorem(s->temperatura);

    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 4.0;

    /* discus vastus */
    double r_disc = 5.0 + luciditas * 3.0;
    if (r_disc > 15.0) r_disc = 15.0;

    fen_punctum(fen, SEMI, SEMI, r_disc * 0.3, col, luciditas * 1.5);
    fen_punctum(fen, SEMI, SEMI, r_disc * 0.7, col, luciditas * 0.6);
    fen_punctum(fen, SEMI, SEMI, r_disc, col, luciditas * 0.2);

    /* maculae (cellulae convectionis) — perturbationes coloris */
    semen_g = (unsigned int)(s->temperatura * 1000);
    for (int i = 0; i < 5; i++) {
        double mx = SEMI + alea_gauss() * r_disc * 0.4;
        double my = SEMI + alea_gauss() * r_disc * 0.4;
        double dx = mx - SEMI, dy = my - SEMI;
        if (dx * dx + dy * dy > r_disc * r_disc * 0.6) continue;
        color_t mc = {col.r * 0.7, col.g * 0.6, col.b * 0.5, 1.0};
        fen_punctum(fen, mx, my, r_disc * 0.2, mc, luciditas * 0.15);
    }

    /* spiculae longissimae */
    if (instr->spiculae > 0) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                       + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(fen, SEMI, SEMI, ang,
                        instr->spiculae_long * luciditas * 1.5,
                        0.6, col, luciditas * 0.2);
        }
    }

    if (instr->halo_vis > 0.01)
        fen_punctum(fen, SEMI, SEMI, instr->halo_radius * 2.0,
                    col, instr->halo_vis * luciditas * 0.5);
}

static void reddere_neutronium(unsigned char *fen,
                               const astra_sidus_t *s,
                               const astra_instrumentum_t *instr)
{
    (void)instr;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 3.0;

    /* punctum album intensissimum */
    color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.4, album, luciditas * 2.0);

    /* annuli pulsantes — duae tenues fasciae */
    color_t cyan = {0.3, 0.7, 1.0, 1.0};
    double r1 = 2.0 + luciditas;
    double r2 = 4.0 + luciditas * 2.0;

    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            double dx = x - SEMI, dy = y - SEMI;
            double d = sqrt(dx * dx + dy * dy);
            double f1 = exp(-(d - r1) * (d - r1) * 2.0) * luciditas * 0.4;
            double f2 = exp(-(d - r2) * (d - r2) * 1.0) * luciditas * 0.15;
            double f = f1 + f2;
            if (f < 0.002) continue;

            int idx = (y * FEN + x) * 4;
            int r = (int)(fen[idx + 0] + cyan.r * f * 255);
            int g = (int)(fen[idx + 1] + cyan.g * f * 255);
            int b = (int)(fen[idx + 2] + cyan.b * f * 255);
            int a = (int)(fen[idx + 3] + f * 255);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            if (a > 255) a = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = (unsigned char)a;
        }
    }

    /* bipolar jets — duae spiculae oppositae */
    double ang_jet = alea_f() * PI_GRAECUM;
    color_t jet_col = {0.5, 0.6, 1.0, 1.0};
    fen_spicula(fen, SEMI, SEMI, ang_jet, 12.0 * luciditas, 0.3,
                jet_col, luciditas * 0.3);
    fen_spicula(fen, SEMI, SEMI, ang_jet + PI_GRAECUM, 12.0 * luciditas, 0.3,
                jet_col, luciditas * 0.3);
}

static void reddere_crystallinum(unsigned char *fen,
                                 const astra_sidus_t *s,
                                 const astra_instrumentum_t *instr)
{
    (void)instr;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.5;

    /* stella crystallina: globus Koosh — multa filamenta recta
     * ex centro irradiantia, diversis coloribus spectralibus */

    color_t col = astra_temperatura_ad_colorem(s->temperatura);

    /* nucleus album intensum */
    color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 1.2, album, luciditas * 1.0);
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 1.5);

    /* multa filamenta — ut globus Koosh */
    semen_g = (unsigned int)(s->temperatura * 137);
    int n_fil = 20 + (int)(luciditas * 15);
    if (n_fil > 60) n_fil = 60;

    /* 6 colores spectrales discreti */
    color_t spectra[6] = {
        {1.0, 0.15, 0.1, 1.0},   /* rubrum */
        {1.0, 0.6, 0.05, 1.0},   /* aurantiacum */
        {0.9, 1.0, 0.1, 1.0},    /* flavum */
        {0.1, 1.0, 0.3, 1.0},    /* viride */
        {0.15, 0.4, 1.0, 1.0},   /* caeruleum */
        {0.7, 0.15, 1.0, 1.0}    /* violaceum */
    };

    for (int i = 0; i < n_fil; i++) {
        /* angulus aleatorius — distributio uniformis */
        double ang = alea_f() * DUO_PI;
        double dx = cos(ang), dy = sin(ang);

        /* longitudo variat */
        double len = 3.0 + alea_f() * (8.0 + luciditas * 8.0);
        if (len > 26.0) len = 26.0;

        /* color spectrale — unum ex 6 */
        color_t fc = spectra[alea() % 6];

        /* intensitas decrescens ab centro */
        int n_steps = (int)(len * 3);
        if (n_steps < 4) n_steps = 4;

        for (int j = 0; j <= n_steps; j++) {
            double t = (double)j / (double)n_steps;
            double px = SEMI + t * len * dx;
            double py = SEMI + t * len * dy;

            /* intensitas: fortis prope centrum, evanescens */
            double f = luciditas * 0.4 * (1.0 - t) * (1.0 - t);
            fen_punctum(fen, px, py, 0.35, fc, f);
        }
    }

    /* halo tenuis multicolor circa centrum */
    fen_punctum(fen, SEMI, SEMI, 4.0, col, luciditas * 0.06);
}

/* Magnetar: stella neutronium cum B ~10^9-10^11 T.
 * Jets relativistici bipolares spirant circa lineas campi magnetici.
 * Materia in jet accelerata ad ~0.3c per processum Blandford-Znajek.
 * Emissio synchrotronum: spectrum continuum, polarizatum,
 * color caeruleum-album (potentia spectri ∝ ν^(-0.7)).
 * Precession geodaetica causat jets spirales (Lense-Thirring).
 * Halo birefringens: vacuum QED prope B_Schwinger lucem
 * in duos modos polares separat (Heisenberg-Euler 1936). */
static void reddere_magnetar(unsigned char *fen,
                              const astra_sidus_t *s,
                              const astra_instrumentum_t *instr)
{
    (void)instr;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 3.0;

    /* semen ex magnitudo et temperatura derivatum — unicum per sidus.
     * antea: semen ex sola temperatura, sed magnetares omnes T=5×10⁶ K
     * habent, ergo omnes eandem orientationem reddebant.
     * nunc magnitudinem addimus ut variatio inter magnetares oriatur. */
    semen_g = (unsigned int)(s->temperatura * 71
            + s->magnitudo * 100003 + s->phase * 70001);

    /* axis magneticus — angulus aleatorius per plenum circulum.
     * in magnetaribus veris, axis magneticus non cum axe rotationis
     * congruere necesse est (obliquitas typica 10°-60°, Lander & Jones
     * 2009). axis proiectus in planum caeli quemlibet angulum habere
     * potest, ergo DUO_PI non PI_GRAECUM adhibemus. */
    double axis_ang = alea_f() * DUO_PI;
    double ax = cos(axis_ang), ay = sin(axis_ang);

    /* variatio individualis magnetaris:
     * campus magneticus B variat 10⁹-10¹¹ T (Duncan & Thompson 1992).
     * B fortior → jets longiores et collimatiores (Bucciantini+ 2006).
     * periodus rotationis P = 2-12 s → precession rate variat.
     * aetas τ = P/(2Ṗ) ~ 10³-10⁵ a → jets seniores latiores et debiliores. */
    double jet_long = 16.0 + alea_f() * 20.0;    /* 16-36 pixeles */
    double jet_apertura = 3.0 + alea_f() * 6.0;  /* amplitudo spiralis */
    double jet_freq = 8.0 + alea_f() * 10.0;     /* frequentia spiralis */
    int    jet_fil = 2 + (int)(alea_f() * 3);     /* 2-4 filamenta */
    double halo_long = 2.0 + alea_f() * 3.0;     /* elongatio hali */

    /* nucleus: album intensum cum halo asymmetrico (birefringentia) */
    color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 2.0);

    /* halo elongatum per axem magneticum (birefringentia vacui).
     * QED in campo magnetico extremo (B > B_Schwinger = m²c³/eℏ =
     * 4.414×10⁹ T) vacuum birefringens fit: duae polarizationes
     * cum indicibus refractionis diversis propagantur (Heisenberg &
     * Euler 1936, Adler 1971). hoc imaginem stellae elongat
     * perpendiculare ad campum magneticum proiectum (Heyl & Shaviv
     * 2000, 2002). in data Chandra, RX J1856.5-3754 elongationem
     * consistentem cum birefringentia vacui ostendit (Mignani+ 2017,
     * confirmatio prima effectus QED in campo astronomico). */
    for (int i = -8; i <= 8; i++) {
        double t = (double)i / 8.0;
        double px = SEMI + t * halo_long * ax;
        double py = SEMI + t * halo_long * ay;
        double f = luciditas * 0.5 * exp(-t * t * 2.0);
        color_t hc = {0.7, 0.8, 1.0, 1.0};
        fen_punctum(fen, px, py, 1.0, hc, f);
    }

    /* jets bipolares relativistici spirantes circa lineas campi.
     *
     * mechanismus: materia accreti per campum magneticum ad polos
     * dirigitur (Goldreich & Julian 1969). particuli relativistici
     * in campo magnetico radiationem synchrotron emittunt cum
     * spectro potentiae F_ν ∝ ν^α ubi α ≈ -0.6 ad -0.8
     * (Rybicki & Lightman 1979). color ergo caeruleus-albus.
     *
     * precession Lense-Thirring (Lense & Thirring 1918):
     * stella neutronium rotans spatium-tempus circa se trahit
     * (frame dragging). axis praecessionis Ω_LT = 2GJ/(c²r³)
     * ubi J = momentum angulare. hoc causat jets spirales —
     * helicam cum radio crescente a polo.
     *
     * collimatio: jets magnetaris typice apertura ~5°-30°
     * (Granot+ 2017), strictiores quam jets AGN (~15°-60°). */
    color_t jet_colores[4] = {
        {0.4, 0.6, 1.0, 1.0},    /* synchrotron caeruleum */
        {0.7, 0.5, 1.0, 1.0},    /* violaceum */
        {0.3, 0.9, 0.9, 1.0},    /* cyaneum (Compton inversus) */
        {0.5, 0.4, 1.0, 1.0}     /* indicum */
    };

    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;

        for (int fil = 0; fil < jet_fil; fil++) {
            double phase_offset = DUO_PI * fil / (double)jet_fil;
            color_t jc = jet_colores[fil % 4];

            int n_steps = 120;
            for (int j = 0; j <= n_steps; j++) {
                double t = (double)j / (double)n_steps;
                double dist = t * jet_long;

                double px_ax = SEMI + dir * dist * ax;
                double py_ax = SEMI + dir * dist * ay;

                double spiral_r = t * t * jet_apertura;
                double spiral_ang = phase_offset + t * jet_freq;
                double perp_x = -ay, perp_y = ax;

                double sx = spiral_r * (cos(spiral_ang) * perp_x
                          - sin(spiral_ang) * dir * ax);
                double sy = spiral_r * (cos(spiral_ang) * perp_y
                          - sin(spiral_ang) * dir * ay);

                double px = px_ax + sx;
                double py = py_ax + sy;

                if (px < 0 || px >= FEN || py < 0 || py >= FEN) continue;

                double f = luciditas * 0.35 * (1.0 - t) * (1.0 - t * 0.5);
                fen_punctum(fen, px, py, 0.4 + t * 0.3, jc, f);
            }
        }
    }

    /* nodi lucidi in jets (shocks interni).
     * Rees (1978) primus proposuit fluctuationes in fluxu jet
     * shells cum velocitatibus diversis creare quae collisionibus
     * "shocks internos" producunt. hoc in blazaribus (Marscher &
     * Gear 1985) et in GRBs (Rees & Mészáros 1994) confirmatum.
     * in magnetaribus, giant flares (ut SGR 1806-20, Palmer+ 2005)
     * nodos similes per eruptiones in jets creant. */
    int n_nodi = 2 + (int)(alea_f() * 3);
    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;
        for (int k = 0; k < n_nodi; k++) {
            double t = 0.15 + alea_f() * 0.6;
            double dist = t * jet_long * 0.9;
            double px = SEMI + dir * dist * ax;
            double py = SEMI + dir * dist * ay;
            fen_punctum(fen, px, py, 0.8 + alea_f() * 0.5, album,
                        luciditas * (0.2 + alea_f() * 0.3));
        }
    }
}

/*
 * reddere_galaxia — galaxia distans in fenestra 64×64.
 *
 * Galaxiae ut objecta extendida redduntur, non ut puncta.
 * Morphologia (in campo "phase" sideris) formam determinat:
 *
 *   GALAXIA_ELLIPTICA:
 *     Profilo de Vaucouleurs (1948, "R^{1/4} law"):
 *       I(r) = I_e · exp(-7.67·((r/r_e)^{1/4} - 1))
 *     ubi I_e = luminositas superficialis ad r_e.
 *     Sérsic (1963) generalizavit: exp(-b_n·((r/r_e)^{1/n}-1))
 *     ubi n=4 dat de Vaucouleurs, n=1 dat exponentialem.
 *     Ellipticitas ex inclinatione et forma intrinseca:
 *     galaxiae vere triaxiales (Binney 1978, Franx+ 1991).
 *     Colores: rubri-aurei, T_eff effectivus ~4000-5000 K
 *     (dominantur stellae K gigantes, Worthey 1994).
 *
 *   GALAXIA_SPIRALIS:
 *     Nucleus (bulge): profilo Sérsic n~2-3.
 *     Discus: profilo exponentiale I(r) = I_0·exp(-r/h_r)
 *     ubi h_r = scala longitudinis (Freeman 1970).
 *     Brachia: perturbatio logarithmica r(θ) = a·exp(b·θ)
 *     (Ringermacher & Mead 2009). numerus brachiorum typice
 *     m=2 (dominans) vel m=4 (flocculent).
 *     Face-on: brachia caerulea visibilia.
 *     Edge-on: discus tenuis cum fascia pulveris centrali
 *     (sicut NGC 891, van der Kruit & Searle 1981).
 *     Inclinatio continua inter haec extrema.
 *
 *   GALAXIA_SPIRALIS_BARRATA:
 *     Ut spiralis sed cum barra stellari centrali.
 *     Barra: structura elongata cum profilo Ferrers (1877):
 *       ρ(m) ∝ (1 - m²)^n ubi m = distantia normalizata.
 *     Brachia ex extremis barrae oriuntur, non ex nucleo.
 *     Resonantiae orbitales (Contopoulos & Papayannopoulos
 *     1980): orbita x1 sustinet barram, x2 perpendicularis
 *     destruit eam — transitus inter regimina determinat
 *     longitudinem barrae.
 *
 *   GALAXIA_LENTICULARIS:
 *     Discus sine brachiis + nucleus prominens.
 *     Profilo compositus: Sérsic (nucleus, n~2-4) +
 *     exponentiale (discus). similaris spirali edge-on
 *     sed sine fascia pulveris et sine nodis formationis.
 *     Laurikainen+ (2010): pleraeque S0 habent etiam
 *     structuras barrae debiles ("S0/a").
 *
 *   GALAXIA_IRREGULARIS:
 *     Sine symmetria regulari. nodi starburst (luminosi,
 *     caerulei) distributi asymmetrice. Hunter & Elmegreen
 *     (2004): formatio stellarum in irregularibus non in
 *     brachiis sed in regionibus stochasticis compressionis.
 *     Exempla: LMC (cum structura barrae residua), SMC,
 *     NGC 1427A (in Fornax, cum cauda mareali).
 *
 * Distributio magnitudinis sequitur Schechter (1976):
 *   φ(L) ∝ (L/L*)^α · exp(-L/L*), α ≈ -1.25.
 *   implementatio: mag = mag_min + Δmag · (1-r^0.4)
 *   ubi r uniformis — pleraeque debiles, paucae lucidae.
 */
static void reddere_galaxia(unsigned char *fen,
                             const astra_sidus_t *s,
                             const astra_instrumentum_t *instr)
{
    (void)instr;

    /* semen unicum ex omnibus proprietatibus */
    semen_g = (unsigned int)(s->temperatura * 137
            + s->magnitudo * 100003 + s->phase * 7001
            + s->angulus_phase * 50021);

    galaxia_morphologia_t morph = (galaxia_morphologia_t)(int)s->phase;
    double luciditas = pow(10.0, -s->magnitudo * 0.4) * 2.5;
    double ang = s->angulus_phase;   /* angulus positionis in caelo */
    double ca = cos(ang), sa = sin(ang);

    /* inclinatio ad lineam visus: 0 = face-on, π/2 = edge-on.
     * distributio uniformis in cos(i) (orientationes aleatoriae
     * in spatio 3D, Hubble 1926). ergo i = arccos(r) ubi r ∈ [0,1].
     * implementamus per "temperatura" campi: T/10000 dat cos(i). */
    double cos_incl = s->temperatura / 10000.0;
    if (cos_incl < 0.05) cos_incl = 0.05;
    if (cos_incl > 1.0) cos_incl = 1.0;

    /* radius effectivus — galaxiae distantes parvae,
     * proximae (lucidiores) maiores. */
    double r_eff = 1.5 + luciditas * 3.0;
    if (r_eff > 20.0) r_eff = 20.0;

    /* ellipticitas apparens ex inclinatione:
     * discus circularis inclinatus: b/a = cos(i).
     * elliptica intrinseca: b/a ∈ [0.3, 1.0]. */
    double ba_ratio;  /* b/a axis ratio */
    double ell_intrin = 0.7 + alea_f() * 0.3;  /* ellipticae: 0.7-1.0 */

    switch (morph) {
    case GALAXIA_ELLIPTICA:
        ba_ratio = ell_intrin;
        break;
    case GALAXIA_LENTICULARIS:
        ba_ratio = cos_incl * 0.9 + 0.1;
        break;
    default:  /* spirales, barratae, irregulares */
        ba_ratio = cos_incl * 0.95 + 0.05;
        break;
    }

    /* color dominans per morphologiam.
     * ellipticae et lenticulares: stellae veteres, rubrae-aureae.
     *   metalicitas alta, [Fe/H] ~ 0 ad +0.3 (Thomas+ 2005).
     * spirales: nucleus aureus, brachia caerulea (O/B stellae).
     * irregulares: caerulescentes (formatio stellarum activa). */
    color_t col_nuc, col_ext;
    switch (morph) {
    case GALAXIA_ELLIPTICA:
        col_nuc = (color_t){1.0, 0.85, 0.6, 1.0};  /* aureum */
        col_ext = (color_t){0.9, 0.75, 0.55, 1.0};
        break;
    case GALAXIA_LENTICULARIS:
        col_nuc = (color_t){1.0, 0.88, 0.65, 1.0};
        col_ext = (color_t){0.85, 0.78, 0.6, 1.0};
        break;
    case GALAXIA_SPIRALIS:
    case GALAXIA_SPIRALIS_BARRATA:
        col_nuc = (color_t){1.0, 0.9, 0.65, 1.0};  /* nucleus aureus */
        col_ext = (color_t){0.6, 0.75, 1.0, 1.0};  /* brachia caerulea */
        break;
    case GALAXIA_IRREGULARIS:
        col_nuc = (color_t){0.7, 0.8, 1.0, 1.0};   /* caerulea */
        col_ext = (color_t){0.6, 0.7, 1.0, 1.0};
        break;
    default:
        col_nuc = (color_t){1.0, 0.9, 0.7, 1.0};
        col_ext = (color_t){0.8, 0.8, 0.8, 1.0};
        break;
    }

    /* variatio individualis coloris */
    double dR = (alea_f() - 0.5) * 0.15;
    double dG = (alea_f() - 0.5) * 0.10;
    double dB = (alea_f() - 0.5) * 0.10;
    col_nuc.r += dR; col_nuc.g += dG; col_nuc.b += dB;
    col_ext.r += dR * 0.5; col_ext.g += dG * 0.5; col_ext.b += dB * 0.5;

    /* --- redditio per morphologiam --- */

    if (morph == GALAXIA_ELLIPTICA) {
        /* profilo de Vaucouleurs: I ∝ exp(-7.67·((r/r_e)^0.25 - 1)).
         * simplificamus ad Gaussianam pro galaxiis parvis,
         * cum forma elliptica ex ba_ratio. */
        for (int dy = -30; dy <= 30; dy++) {
            for (int dx = -30; dx <= 30; dx++) {
                /* coordinatae rotatae per angulum positionis */
                double lx = dx * ca + dy * sa;
                double ly = -dx * sa + dy * ca;
                /* ellipticitas */
                double rx = lx / r_eff;
                double ry = ly / (r_eff * ba_ratio);
                double r2 = rx * rx + ry * ry;
                double r = sqrt(r2);
                if (r > 5.0) continue;
                /* Sérsic n=4 approximatum: exp(-7.67·(r^0.25 - 1)) */
                double sersic = exp(-7.67 * (pow(r + 0.01, 0.25) - 1.0));
                double f = luciditas * sersic;
                if (f < 0.002) continue;
                /* color: gradiens ab aureo (centrum) ad rubescente (extra) */
                color_t c;
                double t = r / 3.0; if (t > 1.0) t = 1.0;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.6, c, f);
            }
        }
    } else if (morph == GALAXIA_SPIRALIS || morph == GALAXIA_SPIRALIS_BARRATA) {
        /* nucleus (bulge) — Sérsic n~2 */
        double r_bulge = r_eff * 0.3;
        fen_punctum(fen, SEMI, SEMI, r_bulge, col_nuc, luciditas * 1.5);

        /* barra (solum pro barratis) */
        if (morph == GALAXIA_SPIRALIS_BARRATA) {
            double bar_len = r_eff * (0.4 + alea_f() * 0.3);
            double bar_wid = r_eff * 0.12;
            for (int i = -40; i <= 40; i++) {
                double t = (double)i / 40.0;
                double bx = t * bar_len;
                double by = 0;
                /* rotare per ang + applicare inclinationem */
                double px = bx * ca - by * sa;
                double py = (bx * sa + by * ca) * ba_ratio;
                double dist = fabs(t);
                double f = luciditas * 0.6 * exp(-dist * dist * 3.0);
                if (f < 0.002) continue;
                fen_punctum(fen, SEMI + px, SEMI + py, bar_wid * ba_ratio,
                            col_nuc, f);
            }
        }

        /* brachia spiralia — r(θ) = a·exp(b·θ), spirala logarithmica.
         * pitch angle typice 10°-30° (Kennicutt 1981):
         *   tan(pitch) = 1/(b·r), ergo b = 1/tan(pitch).
         *   Sa: pitch ~10° (stricta), Sc: pitch ~25° (laxa).
         * duo brachia (m=2) dominans modus per theoriam Lin-Shu. */
        int n_brachia = 2;
        double pitch = 0.3 + alea_f() * 0.4;  /* 0.3-0.7 rad (~17°-40°) */
        double r_start = r_eff * 0.25;
        if (morph == GALAXIA_SPIRALIS_BARRATA)
            r_start = r_eff * 0.45;  /* brachia ex extremis barrae */
        double arm_bright = 0.5 + alea_f() * 0.3;

        for (int arm = 0; arm < n_brachia; arm++) {
            double theta0 = DUO_PI * arm / n_brachia + alea_f() * 0.3;
            int n_steps = 200;
            for (int j = 0; j < n_steps; j++) {
                double t = (double)j / (double)n_steps;
                double theta = theta0 + t * DUO_PI / tan(pitch);
                double r = r_start + t * r_eff * 1.8;

                /* spirala logarithmica cum perturbatione */
                double perturb = alea_f() * 0.15 - 0.075;
                double x_arm = r * cos(theta + perturb);
                double y_arm = r * sin(theta + perturb);

                /* proiectio cum inclinatione et rotatione */
                double px = x_arm * ca - y_arm * sa;
                double py = (x_arm * sa + y_arm * ca) * ba_ratio;

                if (fabs(px) > 28 || fabs(py) > 28) continue;

                double fade = (1.0 - t) * (1.0 - t * 0.3);
                double f = luciditas * arm_bright * fade;
                if (f < 0.002) continue;

                /* regiones HII in brachiis (Kennicutt 1998):
                 * formatio stellarum in undis densitatis.
                 * nodi lucidi stochastice distributi. */
                double nod = 1.0;
                if (alea_f() < 0.08) nod = 1.5 + alea_f();
                double wid = 0.5 + t * 0.8;
                fen_punctum(fen, SEMI + px, SEMI + py, wid,
                            col_ext, f * nod);
            }
        }

        /* discus diffusus exponentialis (Freeman 1970) */
        for (int dy = -25; dy <= 25; dy++) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca + dy * sa;
                double ly = (-dx * sa + dy * ca) / (ba_ratio + 0.01);
                double r = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 3.0) continue;
                double disc = exp(-r / r_eff) * 0.3;
                double f = luciditas * disc;
                if (f < 0.002) continue;
                double t = r / (r_eff * 2.5); if (t > 1.0) t = 1.0;
                color_t c;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, c, f);
            }
        }

        /* fascia pulveris pro edge-on (cos_incl < 0.3):
         * in galaxiis edge-on, fascia obscura centralis visibilis
         * (exemplum classicum: NGC 891, van der Kruit & Searle 1981).
         * extinctio per pulverem: A_V ≈ τ_V / (cos(i) + 0.1). */
        if (cos_incl < 0.3) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca;
                double ly = dx * sa * ba_ratio;
                double dist = fabs((double)dx) / (r_eff * 2.0);
                if (dist > 1.2) continue;
                double f = 0.4 * (1.0 - dist) * (0.3 - cos_incl) / 0.3;
                if (f < 0.01) continue;
                int px = SEMI + (int)(lx + 0.5);
                int py = SEMI + (int)(ly + 0.5);
                if (px < 0 || px >= FEN || py < 0 || py >= FEN) continue;
                /* obscurare pixeles existentes */
                int idx = (py * FEN + px) * 4;
                fen[idx + 0] = (unsigned char)(fen[idx + 0] * (1.0 - f));
                fen[idx + 1] = (unsigned char)(fen[idx + 1] * (1.0 - f));
                fen[idx + 2] = (unsigned char)(fen[idx + 2] * (1.0 - f));
            }
        }

    } else if (morph == GALAXIA_LENTICULARIS) {
        /* nucleus prominens Sérsic n~3 */
        fen_punctum(fen, SEMI, SEMI, r_eff * 0.35, col_nuc, luciditas * 1.8);

        /* discus diffusus sine brachiis — exponentiale */
        for (int dy = -25; dy <= 25; dy++) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca + dy * sa;
                double ly = (-dx * sa + dy * ca) / (ba_ratio + 0.01);
                double r = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 3.0) continue;
                double disc = exp(-r / r_eff) * 0.5;
                double f = luciditas * disc;
                if (f < 0.002) continue;
                double t = r / (r_eff * 2.0); if (t > 1.0) t = 1.0;
                color_t c;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, c, f);
            }
        }

    } else {
        /* GALAXIA_IRREGULARIS — nodi stochastici (Hunter & Elmegreen 2004).
         * distributio spatiosa non symmetrica, nodi luminosi
         * (regiones HII gigantes, 30 Doradus in LMC exemplum)
         * cum fundo diffuso debili. */
        double offset_x = (alea_f() - 0.5) * r_eff * 0.3;
        double offset_y = (alea_f() - 0.5) * r_eff * 0.3;

        /* fundus diffusus asymmetricus */
        for (int dy = -20; dy <= 20; dy++) {
            for (int dx = -20; dx <= 20; dx++) {
                double lx = dx - offset_x;
                double ly = dy - offset_y;
                double r = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 2.5) continue;
                double f = luciditas * 0.3 * exp(-r / r_eff);
                if (f < 0.002) continue;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, col_ext, f);
            }
        }

        /* nodi starburst — 3-8 regiones luminosae */
        int n_nodi = 3 + (int)(alea_f() * 6);
        for (int k = 0; k < n_nodi; k++) {
            double nx = (alea_f() - 0.5) * r_eff * 2.0;
            double ny = (alea_f() - 0.5) * r_eff * 2.0;
            double nr = 0.5 + alea_f() * 1.5;
            double nf = luciditas * (0.3 + alea_f() * 0.5);
            /* colores: caerulei (formatio stellarum) vel rosei (Hα) */
            color_t nc;
            if (alea_f() < 0.6)
                nc = (color_t){0.5, 0.7, 1.0, 1.0};  /* OB stellae */
            else
                nc = (color_t){1.0, 0.5, 0.6, 1.0};  /* HII Hα */
            fen_punctum(fen, SEMI + nx, SEMI + ny, nr, nc, nf);
        }
    }
}

static void reddere_vagans(unsigned char *fen,
                            const astra_sidus_t *s,
                            const astra_instrumentum_t *instr)
{
    (void)instr;
    color_t col = astra_temperatura_ad_colorem(s->temperatura);

    /* vagans: discus maior, matte, cum falce (phase) */
    double radius = 8.0 + pow(10.0, -s->magnitudo * 0.4) * 6.0;
    if (radius > 25.0) radius = 25.0;

    double cos_ph = cos(s->angulus_phase);
    double sin_ph = sin(s->angulus_phase);

    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            double dx = x - SEMI, dy = y - SEMI;
            double d2 = dx * dx + dy * dy;
            double r2 = radius * radius;
            if (d2 > r2) continue;

            /* norma hemisphaerica */
            double nz = sqrt(1.0 - d2 / r2);
            double nx = dx / radius;
            double ny = dy / radius;

            /* illuminatio per directionem phase */
            double illum = nx * cos_ph + ny * sin_ph;
            /* phase: 0=plenus (totus illuminatus), 1=novus (obscurus) */
            double terminator = (1.0 - s->phase * 2.0);
            double vis = illum - terminator;

            double f;
            if (vis > 0.05) {
                /* facies illuminata — Lambert matte */
                double lambert = nz * 0.5 + illum * 0.4 + 0.1;
                if (lambert < 0) lambert = 0;
                f = lambert;
            } else if (vis > -0.05) {
                /* terminator — transitio levis */
                f = (vis + 0.05) * 10.0 * 0.3;
                if (f < 0) f = 0;
            } else {
                /* facies obscura — paene nigra */
                f = 0.02;
            }

            /* limb darkening */
            f *= (0.6 + 0.4 * nz);

            int idx = (y * FEN + x) * 4;
            int r = (int)(col.r * f * 255);
            int g = (int)(col.g * f * 255);
            int b = (int)(col.b * f * 255);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = 255;
        }
    }
}

/* ================================================================
 * dispatcher
 * ================================================================ */

void astra_sidus_reddere(unsigned char *fenestra,
                         const astra_sidus_t *sidus,
                         const astra_instrumentum_t *instrumentum)
{
    memset(fenestra, 0, FEN * FEN * 4);

    switch (sidus->genus) {
    case SIDUS_NANUM_ALBUM:
        reddere_nanum_album(fenestra, sidus, instrumentum);
        break;
    case SIDUS_SEQUENTIA:
        reddere_sequentia(fenestra, sidus, instrumentum);
        break;
    case SIDUS_GIGAS_RUBRUM:
        reddere_gigas_rubrum(fenestra, sidus, instrumentum);
        break;
    case SIDUS_SUPERGIGAS:
        reddere_supergigas(fenestra, sidus, instrumentum);
        break;
    case SIDUS_NEUTRONIUM:
        reddere_neutronium(fenestra, sidus, instrumentum);
        break;
    case SIDUS_CRYSTALLINUM:
        reddere_crystallinum(fenestra, sidus, instrumentum);
        break;
    case SIDUS_MAGNETAR:
        reddere_magnetar(fenestra, sidus, instrumentum);
        break;
    case SIDUS_GALAXIA:
        reddere_galaxia(fenestra, sidus, instrumentum);
        break;
    case SIDUS_VAGANS:
        reddere_vagans(fenestra, sidus, instrumentum);
        break;
    default:
        break;
    }
}
