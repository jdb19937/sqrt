/*
 * tessera.h — 
 */

#ifndef TESSERA_H
#define TESSERA_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tessella.h"

#include "tesserae/sidus.h"
#include "tesserae/planeta.h"
#include "tesserae/visio.h"

typedef struct tessera {
    tessella_t t;

    /* unio phylorum — basis phyli */
    union {
        sidus_t sidus;
        planeta_t planeta;
        visio_t visio;
    } p;

    union {
        int _x; /* stubbus */
    } g;
} tessera_t;

#endif /* TESSERA_H */
