/*
 * caela.h — caela stellarum: structura dati
 *
 * Caela continet campum stellarum cum sideribus, planetis,
 * et proprietatibus Viae Lacteae. Potest ex ISON legi,
 * in ISON scribi, vel ex parametris campi generari.
 */

#ifndef CAELA_H
#define CAELA_H

#include "tessera.h"

/* sidus positum in caelo */
typedef struct {
    int     x;
    int     y;
    sidus_t sidus;
} sidus_caeli_t;

/* planeta positum in caelo */
typedef struct {
    double              x, y, z;
    double              scala;          /* scala reddita = scala_naturalis * f(z) */
    double              scala_naturalis;/* scala ad z=0 ex formula */
    char               *nomen;
    planeta_perceptus_t perceptus;
    planeta_t           planeta;
    unsigned char      *fenestra_cacata;/* 512x512 RGBA, corpus sine illuminatione */
} planeta_caeli_t;

/* caela — campus stellarum */
typedef struct {
    int     latitudo;
    int     altitudo;

    /* Via Lactea */
    double  galaxia_glow;
    double  galaxia_rift;
    int     galaxia_nebulae;
    double  inclinatio_galaxiae;

    /* sidera */
    int              numerus_siderum;
    sidus_caeli_t   *sidera;

    /* planetae */
    int              numerus_planetarum;
    planeta_caeli_t *planetae;

    /* soles cacati (indices in planetae[], -1 si absens) */
    int     sol_primus;
    int     sol_secundus;
} caela_t;

/* legit caelam ex chorda ISON. vocans per caela_destruere liberet. */
caela_t *caela_ex_ison(const char *ison);

/* scribit caelam in chordam ISON. vocans per free() liberet. */
char *caela_in_ison(const caela_t *caela);


/* scribit planeta_caeli in FILE */
void planeta_caeli_in_ison(FILE *f, const planeta_caeli_t *pc);

/* liberat caelam et omnia interna */
void caela_destruere(caela_t *caela);

#endif /* CAELA_H */
