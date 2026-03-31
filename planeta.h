/*
 * planeta.h — renderer planetarum et lunarum
 *
 * Reddit corpora in fenestra 512×512 pixelorum.
 * Colores emergunt ex proprietatibus physicis: compositione
 * chimica superficiei, pressione atmosphaerica, etc.
 * Nullae overrides colorum — physica sola determinat apparentiam.
 */

#ifndef PLANETA_H
#define PLANETA_H

#include "perceptus.h"
#include "color.h"
#include "instrumentum.h"

#include <math.h>

/* ================================================================
 * constantes
 * ================================================================ */

#define PLANETA_FENESTRA  512
#define PLANETA_SEMI      256

/* ================================================================
 * typi
 * ================================================================ */

typedef enum {
    PLANETA_SAXOSUM,     /* Mercury, Venus, Terra, Mars, Luna, Io, Europa */
    PLANETA_GASEOSUM,    /* Jupiter, Saturnus */
    PLANETA_GLACIALE,    /* Uranus, Neptunus */
    PLANETA_PARVUM,      /* Pluto, Ceres, lunae parvae */
    PLANETA_SOL,         /* stella proxima — fusio completa */
    PLANETA_NEBULA       /* nubes gasei procedurale */
} planeta_genus_t;

typedef struct {
    planeta_genus_t genus;
    double     radius;           /* 0.0-1.0: fractio fenestrae */
    double     inclinatio;       /* inclinatio axialis (radiani) */
    double     rotatio;          /* longitudo centralis visibilis (radiani) */
    unsigned   semen;            /* semen procedurale */

    /* ============================================================
     * compositio superficiei (fractiones, summa ≤ 1)
     *
     * Color derivatur ex compositione:
     *   silicata   → griseum (SiO₂/MgSiO₃, Luna highlands)
     *   ferrum     → rubrum/brunneum (Fe₂O₃, Mars regolith)
     *   sulphur    → flavum/aurantium (S/SO₂, Io)
     *   carbo      → nigrum (C, carbones, asteroidea C-typus)
     *   glacies    → album/caerulescens (H₂O crystallina)
     *   glacies_co2 → album (CO₂ crystallina, Mars polus)
     *   malachita  → viridis (Cu₂CO₃(OH)₂ — vel quaelibet
     *                materia viridis: chlorophyllum, oxidum Cu)
     * ============================================================ */
    double     silicata;         /* SiO₂ / silicates */
    double     ferrum;           /* Fe₂O₃ / iron oxides */
    double     sulphur;          /* S / SO₂ */
    double     carbo;            /* C / carbonaceous */
    double     glacies;          /* H₂O ice */
    double     glacies_co2;      /* CO₂ ice */
    double     malachita;        /* green minerals / biology */

    /* ============================================================
     * aqua liquida
     * ============================================================ */
    double     aqua;             /* fractio superficiei aqua liquida 0..1 */
    double     aqua_profunditas; /* 0=vadosa(caerulea clara), 1=profunda(obscura) */

    /* ============================================================
     * terrain procedurale
     * ============================================================ */
    int        continentes;      /* numerus continentium (0=nullae, 1=pangaea) */
    double     scala;            /* scala featurum (0.5=parvae, 2.0=magnae) */
    double     tectonica;        /* complexitas litoralis 0..1 */
    double     craterae;         /* densitas craterarum 0..1 */
    double     maria;            /* fractio mariorum basalticorum 0..1 */
    double     vulcanismus;      /* activitas vulcanica 0..1 */

    /* ============================================================
     * atmosphaera (unitates physicae)
     *
     * Pressio in kPa: Terra=101, Mars=0.6, Venus=9200, Titan=147.
     * Compositio: fractiones (summa ~1). Color derivatur:
     *   n2+o2    → Rayleigh caeruleum (σ ∝ λ⁻⁴)
     *   co2      → per se incolor; cum pulvere → roseum
     *   ch4      → absorptio ad 619nm → caeruleum forte
     *   h2+he    → per se incolores; cum nh3 nubibus → cremeum
     *   pulvis   → Mie scattering → pallidum / coloratum
     * ============================================================ */
    double     pressio_kPa;      /* pressio superficiei */
    double     n2;               /* nitrogen */
    double     o2;               /* oxygen */
    double     co2;              /* carbon dioxide */
    double     ch4;              /* methane */
    double     h2;               /* hydrogen */
    double     he;               /* helium */
    double     nh3;              /* ammonia */
    double     pulvis;           /* aerosoli / pulvis 0..1 */
    double     nubes;            /* cooperimentum nubium 0..1 */

    /* ============================================================
     * glacies polaris
     * ============================================================ */
    double     polaris;          /* extensio calottum polarium 0..1 */

    /* ============================================================
     * fasciae (gigantes gaseousi)
     * ============================================================ */
    int        fasciae;          /* numerus fasciarum */
    double     fasciae_contrast; /* 0..1 */

    /* ============================================================
     * maculae (GRS, GDS, etc.)
     * ============================================================ */
    int        maculae;          /* numerus macularum */
    double     macula_lat;       /* latitudo principalis (-1..1) */
    double     macula_lon;       /* longitudo (radiani) */
    double     macula_radius;    /* magnitudo 0..1 */
    double     macula_obscuritas; /* <0=lucidior, >0=obscurior */

    /* ============================================================
     * fusio stellaris — transitus continuus ex planeta ad stellam
     *
     * fusio=0: planeta ordinarius (nullus effectus).
     * fusio=1: stella plena — limb darkening, corona, granulatio.
     * Gradus intermedii: nana brunnea, sub-stella, corpus ignescens.
     * Adhiberi potest cuique generi; maxime in PLANETA_GASEOSUM.
     *
     * PLANETA_SOL == PLANETA_GASEOSUM + fusio proxima 1.0, sed
     * renderitor separatus est ob varietatem visualem.
     *
     * Compositio stellae ex campis h2/he/ch4 derivatur (H-He normalis):
     *   h2 + he ~ 0.98: stella ordinaria (fusio = color Plancki)
     *   ch4/nh3: doppleriani stellae frigidae
     *   temperatura: praescriptio directa (0 = ex compositione)
     * ============================================================ */
    double     fusio;            /* intensitas fusionis [0,1] */
    double     temperatura;      /* temperatura photosphaericae (K); 0=ex compositione */
    double     luminositas;      /* multiplicator fulgoris (>1 = saturatio et fulgor) */
    double     corona;           /* extensio/intensitas coronae [0,1] */
    double     granulatio;       /* granulatio convectiva (cellulae) [0,1] */
} planeta_t;

/* ================================================================
 * functiones
 * ================================================================ */

void planeta_reddere(unsigned char *fenestra, const planeta_t *planeta,
                     const planeta_perceptus_t *perceptus);

planeta_t planeta_ex_ison(const char *ison);

void planeta_instrumentum_applicare(const planeta_t *planeta,
                                    instrumentum_t *instr);

#endif /* PLANETA_H */
