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

    /* unio generum — accessio phyli per primum membrum (basis) */
    union {
        /* phyla (accessio communis) */
        sidus_t                sidus;
        planeta_t              planeta;
        visio_t                visio;

        /* genera siderum */
        sidus_nanum_album_t    sidus_nanum_album;
        sidus_sequentia_t      sidus_sequentia;
        sidus_gigas_rubrum_t   sidus_gigas_rubrum;
        sidus_supergigas_t     sidus_supergigas;
        sidus_neutronium_t     sidus_neutronium;
        sidus_crystallinum_t   sidus_crystallinum;
        sidus_magnetar_t       sidus_magnetar;
        sidus_galaxia_t        sidus_galaxia;
        sidus_vagans_t         sidus_vagans;

        /* genera planetarum */
        planeta_saxosum_t      planeta_saxosum;
        planeta_gaseosum_t     planeta_gaseosum;
        planeta_glaciale_t     planeta_glaciale;
        planeta_parvum_t       planeta_parvum;
        planeta_sol_t          planeta_sol;
        planeta_nebula_t       planeta_nebula;

        /* genera visionum */
        visio_torus_t          visio_torus;
        visio_navis_t          visio_navis;
        visio_zeppelinus_t     visio_zeppelinus;
    } g;
} tessera_t;

#endif /* TESSERA_H */
