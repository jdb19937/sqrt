/*
 * helvea.h — torus planus corrugatus, bibliotheca communis
 *
 * Superficies tori Hevea et illuminatio.
 * Omnia programmata tori hanc bibliothecam adhibent.
 */

#ifndef HELVEA_H
#define HELVEA_H

#include "pictura.h"

#define HELVEA_STRATA_MAX       7
#define HELVEA_RADIUS_MAIOR     1.0
#define HELVEA_RADIUS_MINOR     0.42

/* profunditas detalli — numerus phasium corrugationis Borrelli.
 * 1..HELVEA_STRATA_MAX. Originale Borrelli: 3 phasae.
 * Programmata hunc valorem mutant pro UI. */
extern int helvea_strata;

/* ================================================================
 * superficies tori Hevea
 * ================================================================ */

typedef enum {
    HELVEA_BORRELLI,    /* approximatio Nash-Kuiper (Borrelli 2012) */
    HELVEA_BORRELLI_T,  /* Borrelli transposita (orientatio altera) */
    HELVEA_PLANUS       /* torus revolutionis ordinarius (sine corrugationibus) */
} helvea_methodus_t;

#define HELVEA_NUMERUS_METHODORUM 3

extern const char *helvea_nomina_methodorum[HELVEA_NUMERUS_METHODORUM];

/*
 * helvea_superficies — punctum superficiei.
 * methodus: algorithmum corrugationis eligit.
 */
vec3_t helvea_superficies(
    double u, double v,
    double radius_maior, double radius_minor,
    helvea_methodus_t methodus
);

/* norma per differentias finitas centrales */
vec3_t helvea_norma(
    double u, double v,
    double radius_maior, double radius_minor,
    helvea_methodus_t methodus
);

/*
 * superficiem praecomputare.
 * vertices marginales copiantur ut superficies claudatur
 * (corrugatio non est periodica in [0,2π]).
 */
void helvea_superficiem_computare(
    vec3_t *puncta, vec3_t *normae,
    int gradus_u, int gradus_v,
    double radius_maior, double radius_minor,
    helvea_methodus_t methodus
);

/* ================================================================
 * illuminatio simplex (Blinn-Phong, aurum)
 * ================================================================ */

color_t helvea_illuminare(vec3_t punct, vec3_t norm, vec3_t oculus);

/* ================================================================
 * themata — colores et modi lucis
 *
 * Systema thematum sine ulla dependentia SDL.
 * Programmata thema per indicem eligunt, deinde
 * helvea_illuminare_thema() ut illuminare_fn transmittunt.
 * ================================================================ */

typedef enum {
    LUX_IRIDESCENS,     /* pellicula tenuis — color ex angulo visus */
    LUX_PLANUS,         /* diffusum matte (half-Lambert) */
    LUX_TABULATA,       /* cel shading — gradus discreti */
    LUX_RAMPA           /* color ex NdotL in rampam 4 colorum */
} helvea_modus_lucis_t;

typedef struct {
    const char          *nomen;
    helvea_modus_lucis_t modus;
    int                  pfx;           /* vexilla post-effectuum (bit OR) */
    double               ir_freq;
    color_t                ir_phase;
    double               ir_saturatio;
    color_t                materia;
    color_t                fresnel_color;
    double               fresnel_vis;
    double               spec_potentia;
    double               spec_vis;
    double               ambiens;
    int                  cel_gradus;
    int                  posteriza_niv;
    color_t                rampa[4];
} helvea_thema_t;

/* vexilla post-effectuum (combinabilia per OR) */
#define HELVEA_PFX_NULLUS    0
#define HELVEA_PFX_LINEAE    1
#define HELVEA_PFX_POSTERIZA 2
#define HELVEA_PFX_NIGRESCO  4
#define HELVEA_PFX_GRANUM    8

/* tabula thematum et numerus */
extern const helvea_thema_t helvea_themata[];
extern const int helvea_numerus_thematum;

/* thema activum — programmata hunc indicem ponunt ante illuminationem */
extern int helvea_index_thematis;

/*
 * helvea_illuminare_thema — illuminatio secundum thema activum.
 * Signaturam illuminare_fn habet, ergo directe
 * ut callback transmitti potest.
 */
color_t helvea_illuminare_thema(vec3_t punct, vec3_t norm, vec3_t oculus);

#endif /* HELVEA_H */
