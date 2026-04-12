/*
 * designatio.h — designatio generationis universi
 *
 * Parametri ex quibus universalis stellarum et galaxiarum
 * generatur. Similis formulae sed sine planetis.
 */

#ifndef DESIGNATIO_H
#define DESIGNATIO_H

#include "universalis.h"

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

    /* galaxiae distantes */
    int          numerus_galaxiarum;
} designatio_t;

/* legit designationem ex chorda ISON */
void designatio_ex_ison(designatio_t *d, const char *ison);

/* scribit designationem in FILE */
void designatio_in_ison(FILE *f, const designatio_t *d);

/* generat universalem ex designatione */
universalis_t *universalis_ex_designatione(const designatio_t *d);

#endif /* DESIGNATIO_H */
