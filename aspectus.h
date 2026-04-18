/*
 * aspectus.h — aspectus illuminationis planetarum
 *
 * Aspectus parametra redditionis continet quae a campo definiuntur,
 * non proprietates intrinsecas corporis. Sicut instrumentum ad sidus:
 * eadem planeta aliter apparet prope solem, longe a sole, in eclipsi.
 */

#ifndef ASPECTUS_H
#define ASPECTUS_H

#include <stdio.h>

/* ================================================================
 * typus
 * ================================================================ */

/*
 * Parametra illuminationis exterioris planetae.
 * Haec in campo (campus.ison) per planetam positionemque solis definiuntur.
 *
 * situs:   fractio obscurata [0=plenus, 0.5=dimidius, 1=novus].
 * angulus: directio lucis incidentis in radianes (0=dextra, π/2=infra).
 * lumen:   quantitas illuminationis [0=nulla, 1=plena, >1=aucta].
 */
typedef struct {
    double situs;    /* illuminatio: 0=plenus, 0.5=dimidius, 1=novus */
    double angulus;  /* directio lucis (radiani) */
    double lumen;    /* quantitas illuminationis (praefinitum 1.0) */
} planeta_aspectus_t;

/* ================================================================
 * functiones
 * ================================================================ */

/*
 * planeta_aspectus_ex_ison — legit aspectum ex chorda ISON.
 * Si campus non invenitur, praefinitum reddit (situs=0, angulus=0).
 */
planeta_aspectus_t planeta_aspectus_ex_ison(const char *ison);

void planeta_aspectus_in_ison(FILE *f, const planeta_aspectus_t *a);

#endif /* ASPECTUS_H */
