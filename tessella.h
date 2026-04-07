/*
 * tessella.h — 
 */

#ifndef TESSELLA_H
#define TESSELLA_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* phyla tessera */
typedef enum {
    SIDUS,
    PLANETA,
    VISIO
} phylum_t;

typedef struct tessella {
    phylum_t p;
} tessella_t;

#endif /* TESSELLA_H */
