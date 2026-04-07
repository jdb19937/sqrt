/*
 * galaxia.h — sidus galaxia (objectum distans)
 *
 * Morphologia Hubble (1926): elliptica, spiralis, barrata,
 * lenticularis, irregularis.
 */

#ifndef SIDUS_GALAXIA_H
#define SIDUS_GALAXIA_H

#include "../sidus.h"

typedef struct {
    sidus_t                basis;
    galaxia_morphologia_t  morphologia;    /* classificatio Hubble */
    double                 angulus_phase;   /* angulus positionis in caelo */
} sidus_galaxia_t;

#endif /* SIDUS_GALAXIA_H */
