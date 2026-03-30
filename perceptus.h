/*
 * perceptus.h — perceptus spectatoris planetarum
 *
 * Omnia parametra externa redditionis planetae: illuminatio (aspectus,
 * coaspectus) et pipeline convolutionalis (acuitas, detallum, granum,
 * aberratio). Sicut instrumentum ad sidera, perceptus ad planetas.
 */

#ifndef PERCEPTUS_H
#define PERCEPTUS_H

#include "aspectus.h"

typedef struct {
    planeta_aspectus_t aspectus;    /* lux primaria (lumen=1.0 si absens) */
    planeta_aspectus_t coaspectus;  /* lux secundaria (lumen=0.0 si absens) */
    double acuitas;                 /* acuitas [0..2] */
    double detallum;                /* synthesia texturarum [0..1] */
    double granum;                  /* granum pelliculae [0..1] */
    double aberratio;               /* aberratio chromatica [0..1] */
} planeta_perceptus_t;

planeta_perceptus_t planeta_perceptus_ex_ison(const char *ison);

void planeta_perceptum_applicare(unsigned char *fen,
                                 const planeta_perceptus_t *p);

#endif /* PERCEPTUS_H */
