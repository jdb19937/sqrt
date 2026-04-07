/*
 * nebula.h — planeta nebula (nubes gasei procedurale)
 *
 * Non sphaericum: forma ex fbm angulari.
 * Color ex temperatura Planckiana + lineis emissionis (Ha, OIII).
 */

#ifndef PLANETA_NEBULA_H
#define PLANETA_NEBULA_H

#include "../planeta.h"

typedef struct {
    /* emissio */
    double temperatura;      /* color corporis nigri (K); 0 = 5000K */
    double luminositas;      /* mensura emissionis */

    /* linea emissionis */
    double h2;               /* intensitas Ha [0,1] */
    double o2;               /* intensitas OIII [0,1] */

    /* structura */
    double carbo;            /* fractio nubeculae absorbentis [0,1] */
    double tectonica;        /* turbulentia [0,1] */
    double nubes;            /* densitas filamentorum [0,1] */
} nebulula_t;

typedef struct {
    tessella_t   avi;
    planetella_t pro;
    nebulula_t   res;
} nebula_t;

#endif /* PLANETA_NEBULA_H */
