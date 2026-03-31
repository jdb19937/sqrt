/*
 * astra.h — corpus caeleste: phyla et structura communis
 *
 * Planeta et sidus uterque astram unam sunt.
 * Phylum distinguit: planeta = corpus prope reddendum;
 *                    sidus   = stella vel galaxia ex distantia.
 *
 * PLANETA_SOL et SIDUS_VAGANS phylis diversis pertinent:
 *   planeta/sol — sol ut corpus proximum (fusio plena, granulatio)
 *   sidus/vagans — corpus vagans ut punctum luminosum in campo
 */

#ifndef ASTRA_H
#define ASTRA_H

#include "planeta.h"
#include "sidus.h"

/* ================================================================
 * phyla
 * ================================================================ */

typedef enum {
    PHYLUM_PLANETA,   /* corpus prope: planeta, luna, sol */
    PHYLUM_SIDUS      /* corpus ex distantia: stella, galaxia, vagans */
} phylum_t;

/* ================================================================
 * astra_t — unio phylorum
 * ================================================================ */

typedef struct {
    phylum_t phylum;
    union {
        planeta_t planeta;
        sidus_t   sidus;
    } corpus;
} astra_t;

/* ================================================================
 * functiones
 * ================================================================ */

/*
 * astra_ex_ison — legit astram ex chorda ISON.
 * campus "phylum" determinat quid legatur:
 *   "planeta" vel absente → planeta_ex_ison
 *   "sidus"               → sidus ex ison
 */
astra_t astra_ex_ison(const char *ison);

#endif /* ASTRA_H */
