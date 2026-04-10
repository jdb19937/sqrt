/*
 * campus.h — campus stellarum, generatio et compositio
 */

#ifndef CAMPUS_H
#define CAMPUS_H

/* campus stellarum (bitmap toroidalis) — ante alias inclusiones
 * quia instrumentum.h et planeta.h hunc typum referunt */
typedef struct {
    unsigned char *pixels;   /* RGB, latitudo * altitudo * 3 */
    int            latitudo;
    int            altitudo;
} campus_t;

#include "tessera.h"
#include "instrumentum.h"

/* ================================================================
 * campus stellarum
 *
 * Fundamenta cosmologica:
 *
 *   Distributio stellarum in caelo non uniformis est sed structuram
 *   hierarchicam reflectit a primordiis cosmicis derivatam:
 *
 *   Fluctuationes quanticae in campo inflatonis (Guth 1981, Linde 1982)
 *   per inflationem cosmicam amplificatae sunt in perturbationes
 *   densitatis macroscopicas. Spectrum potentiae P(k) ∝ k^(n_s-1)
 *   ubi n_s ≈ 0.965 (Planck 2018) — fere invariantem sed non exacte,
 *   indicans campum inflaton non perfecte liberum fuisse.
 *
 *   Hae perturbationes per instabilitatem gravitationalem Jeans
 *   (λ_J = c_s √(π/Gρ)) in structuras hierarchicas collapserunt:
 *   filamenta → nodi → halos → galaxiae → stellae.
 *   Hoc est "cosmic web" (Bond, Kofman & Pogosyan 1996).
 *
 *   In theoria chordarum (string landscape, Bousso & Polchinski 2000),
 *   constantiae physicae (α_EM, m_e/m_p, Λ) ex selectione vacui
 *   in ~10^500 possibilibus dependent. Principium anthropicum
 *   (Weinberg 1987) limitat Λ ad valorem observatum ~10^(-122) M_Pl^4
 *   — si Λ maior esset, nulla structura formaretur.
 *
 *   Nucleosynthesis primordialis (Alpher, Bethe, Gamow 1948):
 *   ratio H/He ~75/25 per massam, determinata per η = n_B/n_γ ≈ 6×10^(-10).
 *   Haec compositio primaeva stellarum spectra et evolutionem regit:
 *   - Populatio III (Z=0): primae stellae, massivissimae, calidissimae
 *   - Populatio II (Z bassa): halo galacticum, globulares
 *   - Populatio I (Z alta): discus, brachia spiralia, Sol
 *
 *   Fascia galatctica (Via Lactea) structuram disci reflectit:
 *   scala altitudinis h_z ≈ 300 pc (discus tenuis), profilo sech²(z/h_z).
 *   Nucleus lucidior quia densitas stellaris ∝ exp(-R/R_d) ubi R_d ≈ 2.6 kpc.
 *   Great Rift ex nubibus molecularibus (H₂, CO, pulvis silicatum)
 *   cum extinctione A_V ≈ 1 mag/kpc in plano.
 *
 *   Topologia toroidalis campi stellarum non est mera commoditas
 *   computationis: cosmologia inflationaria universum spatialiter
 *   planum praedicit (Ω_k = 0.001 ± 0.002, Planck 2018).
 *   Universum finitum cum topologia toroidali T³ cum omnibus
 *   observationibus concordat et suppressionem anomalam potentiae
 *   CMB in scalis magnis naturaliter explicat (Luminet et al. 2003).
 * ================================================================ */

/* parametri generationis campi stellarum */
typedef struct {
    int    numerus_stellarum;     /* stellae totales tentandae */
    double densitas_galaxiae;     /* 0..1: intensitas concentrationis in fascia */
    double inclinatio_galaxiae;   /* angulus fasciae (radianes) */
    double latitudo_galaxiae;     /* latitudo fasciae (fractio altitudinis) */
    unsigned int semen;           /* semen aleatorium */

    /* via lactea — glow diffusum */
    double galaxia_glow;          /* 0..1: intensitas glow (0 = nullum) */
    double galaxia_rift;          /* 0..1: intensitas fasciae pulveris */
    int    galaxia_nebulae;       /* numerus nebulosarum emissionis */

    /* limites per genus (0 = illimitatum) */
    int    max_supergigantes;
    int    max_gigantes;
    int    max_exotica;           /* crystallinum + magnetar + neutronium */

    /* planetae */
    int    numerus_planetarum;
    double planetae_temp_min;     /* temperatura minima planetarum */
    double planetae_temp_max;

    /* galaxiae distantes */
    int    numerus_galaxiarum;    /* galaxiae totales tentandae */
    int    max_galaxiae;          /* maximum galaxiarum positarum (0 = illimitatum) */
} campus_parametri_t;

/* campum creare et destruere */
campus_t *campus_creare(int latitudo, int altitudo);
void campus_destruere(campus_t *c);

/* campum stellarum generare — instrumentum separatum a campo */
void campus_generare(
    campus_t *c, const campus_parametri_t *p,
    const instrumentum_t *instrumentum
);

/* pixel in campum toroidalem scribere (coordinatae modulantur) */
void campus_pixel_scribere(
    campus_t *c, int x, int y,
    unsigned char r, unsigned char g, unsigned char b
);

/*
 * campus_regio_vacua — inspicit an regio circa (cx,cy) satis obscura sit.
 * radius: distantia minimalis in pixelis.
 * reddit 1 si regio vacua, 0 si iam stella praesens.
 */
int campus_regio_vacua(const campus_t *c, int cx, int cy, int radius);

/* sidus (64×64) in campum inserere ad positionem (x,y) — toroidale */
void sidus_in_campum(
    campus_t *c, int cx, int cy,
    const unsigned char *fenestra
);

/*
 * planeta (256×256) in campum inserere ad positionem (x,y) — toroidale.
 * scala: fractio magnitudinis (1.0 = 256px, 0.5 = 128px).
 * Planetae redduntur ultimi quia proximi sunt observatori.
 */
void planeta_in_campum(
    campus_t *c, int cx, int cy,
    const unsigned char *fenestra, double scala
);

/*
 * campus_ex_isonl_reddere — campum stellarum ex ISONL et instrumento reddit.
 * Legit ISONL (stellae fixae ex caele), applicat instrumentum opticum,
 * reddit campum paratum. Vocans campum per campus_destruere liberet.
 * Reddit NULL si error.
 */
campus_t *campus_ex_isonl_reddere(
    const char *via_isonl,
    const char *via_instrumentum
);

campus_t *campus_ex_ison_reddere(
    const char *via_ison,
    const char *via_instrumentum
);

/* post-processare: effectus instrumenti applicare ad campum */
void isonl_post_processare(
    campus_t *c,
    const instrumentum_t *inst
);

/* tabula dynamica: campum cum effectibus temporalibus reddere */
campus_t *campus_tabulam_dynamicam(
    const campus_t *basis,
    const instrumentum_t *inst,
    int tabula,
    int scala,
    double dx, double dy
);

#endif /* CAMPUS_H */
