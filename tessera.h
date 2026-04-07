/*
 * tessera.h — tessera polymorpha
 */

#ifndef TESSERA_H
#define TESSERA_H

#include "tessella.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tessella.h"
#include "tesserae/sidus.h"
#include "tesserae/planeta.h"
#include "tesserae/visio.h"

/* phyla tessera */
typedef enum {
    SIDUS,
    PLANETA,
    VISIO
} tesserarius_t;

typedef struct tessera {
    tesserarius_t qui;

    union {
        sidus_t   sidus;
        planeta_t planeta;
        visio_t   visio;
    } ubi;
} tessera_t;

#endif /* TESSERA_H */
