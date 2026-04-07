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
    sidus_t basis;
    double  phase;           /* illuminatio: 0=plenus, 1=novus */
    double  angulus_phase;   /* angulus illuminationis */
} sidus_vagans_t;

#endif /* SIDUS_VAGANS_H */
