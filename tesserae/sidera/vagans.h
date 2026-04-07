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

#endif /* SIDUS_VAGANS_H */
