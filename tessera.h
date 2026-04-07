/*
 * tessera.h — tessera polymorpha
 */

#ifndef TESSERA_H
#define TESSERA_H

#include "tessella.h"

#include "tesserae/sidus.h"
#include "tesserae/planeta.h"
#include "tesserae/visio.h"

typedef struct tessera {
    tessella_t t;

    /* unio phylum — quodque phylum iam genera sua continet */
    union {
        /* diminutivae (accessio communis tantum) */
        sidulum_t     sidulum;
        planetella_t  planetella;
        visiuncula_t  visiuncula;

        /* plena (cum generibus) */
        sidus_t       sidus;
        planeta_t     planeta;
        visio_t       visio;
    } g;
} tessera_t;

#endif /* TESSERA_H */
