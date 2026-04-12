/*
 * universalis.h — universalis: structura dati
 *
 * Universalis continet campum universi in toro quadrato
 * cum sideribus (systemata solaria). Similis caelae sed
 * ex perspectiva totius universi, non unius systematis.
 */

#ifndef UNIVERSALIS_H
#define UNIVERSALIS_H

#include "tessera.h"

/* sidus positum in universo */
typedef struct {
    int     u;      /* coordinata horizontalis, < 4096 */
    int     v;      /* coordinata verticalis, < 4096 */
    char   *nomen;
    sidus_t sidus;
} sidus_universalis_t;

/* universalis — campus universi */
typedef struct {
    int     latitudo;
    int     altitudo;

    /* stellae (galacticae) */
    int                    numerus_stellarum;
    sidus_universalis_t   *stellae;

    /* galaxiae (extragalacticae) */
    int                    numerus_galaxiarum;
    sidus_universalis_t   *galaxiae;
} universalis_t;

/* legit universalem ex chorda ISON. vocans per universalis_destruere liberet. */
universalis_t *universalis_ex_ison(const char *ison);

/* scribit universalem in chordam ISON. vocans per free() liberet. */
char *universalis_in_ison(const universalis_t *u);

/* liberat universalem et omnia interna */
void universalis_destruere(universalis_t *u);

/* resultatum quaerendi — stella cum distantia */
typedef struct {
    int    index;       /* index in u->stellae */
    double distantia;   /* distantia toroidalis */
} sidus_propinquum_t;

/*
 * universalis_propinqua — invenit usque ad n stellas propinquissimas
 *
 * Quaerit stellas (non galaxias) in universali quae a puncto (pu, pv)
 * distant non plus quam d. Redit usque ad n stellas, ordinatas per
 * distantiam. Resultata scribuntur in res[] (>= n elementa).
 * Redit numerum stellarum inventarum (0..n).
 */
int universalis_propinqua(
    const universalis_t *u, int pu, int pv,
    double d, int n,
    sidus_propinquum_t *res
);

#endif /* UNIVERSALIS_H */
