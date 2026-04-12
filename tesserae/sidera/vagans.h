/*
 * vagans.h — sidus vagans (luna, planeta proximus)
 *
 * Discus maior cum illuminatione phasica.
 * Phase: 0=plenus, 1=novus. Angulus: directio lucis.
 */

#ifndef SIDUS_VAGANS_H
#define SIDUS_VAGANS_H

#include "../sidus.h"

typedef struct {
    double phase;       /* illuminatio: 0=plenus, 1=novus */
    double angulus;     /* angulus illuminationis */
} vaganulus_t;

typedef struct {
    tessella_t  avi;
    sidulum_t   pro;
    vaganulus_t res;
} vagans_t;

void reddere_vagans(unsigned char *fen, const vagans_t *s, const instrumentum_t *instr);
void vagans_in_ison(FILE *f, const vagans_t *s);
void vagans_ex_ison(vagans_t *s, const char *ison);

#endif /* SIDUS_VAGANS_H */
