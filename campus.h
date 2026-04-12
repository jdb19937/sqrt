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

/* campum creare et destruere */
campus_t *campus_creare(int latitudo, int altitudo);
void campus_destruere(campus_t *c);

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

#include "caela.h"

/* quattuor phases redditionis */
void campus_sidera_reddere(campus_t *c, const caela_t *caela, const instrumentum_t *inst);
void campus_viam_lacteam_reddere(campus_t *c, const caela_t *caela);
void campus_planetas_reddere(campus_t *c, const caela_t *caela);
void campus_planetas_reddere_in_conspectu(campus_t *c, const caela_t *caela, int vx, int vy, int vw, int vh);

/* campum stellarum ex caela et instrumento reddere (omnes phases) */
campus_t *campus_ex_caela(
    const caela_t *caela,
    const instrumentum_t *inst
);

/* post-processare: effectus instrumenti applicare ad campum */
void campus_post_processare(
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
