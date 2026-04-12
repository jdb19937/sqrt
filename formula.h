/*
 * formula.h — formula generationis caeli
 *
 * Parametri ex quibus caela stellarum generatur:
 * numerus stellarum, densitas, semen, etc.
 * Planetae transcribuntur in caelam ut sunt.
 */

#ifndef FORMULA_H
#define FORMULA_H

#include "caela.h"
#include "orbita.h"

/* planeta in formula — corpus cum orbita */
typedef struct {
    char      *nomen;
    double     scala;       /* scala naturalis redditionis ad z=0 */
    planeta_t  planeta;
    orbita_t   orbita;
} planeta_formulae_t;

typedef struct {
    int          latitudo;
    int          altitudo;
    int          numerus_stellarum;
    double       densitas_galaxiae;
    double       inclinatio_galaxiae;
    double       latitudo_galaxiae;
    unsigned int semen;

    /* via lactea */
    double       galaxia_glow;
    double       galaxia_rift;
    int          galaxia_nebulae;

    /* limites per genus (0 = illimitatum) */
    int          max_supergigantes;
    int          max_gigantes;
    int          max_exotica;

    /* vagantes (planetae simplices ut sidera) */
    int          numerus_planetarum;
    double       planetae_temp_min;
    double       planetae_temp_max;

    /* galaxiae distantes */
    int          numerus_galaxiarum;
    int          max_galaxiae;

    /* planetae (corpora — positio et perceptus in caela adduntur) */
    int                  numerus_planetarum_formulae;
    planeta_formulae_t  *planetae;
} formula_t;

/* legit formulam ex chorda ISON */
void formula_ex_ison(formula_t *f, const char *ison);

/* scribit formulam in FILE */
void formula_in_ison(FILE *f, const formula_t *form);

/* generat caelam ex formula ad passum temporis t (0 = initiale) */
caela_t *caela_ex_formula(const formula_t *formula, int t);

/* recomputat x, y, scala planetarum in caela ex orbitis formulae ad passum t */
void caela_orbitas_applicare(caela_t *caela, const formula_t *formula, int t);

/* liberat interna formulae (non formulam ipsam) */
void formula_purgare(formula_t *f);

#endif /* FORMULA_H */
