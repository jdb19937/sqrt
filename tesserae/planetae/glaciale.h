/*
 * glaciale.h — planeta glaciale (Uranus, Neptunus)
 *
 * Gigantes glaciales cum fasciis subtilibus, maculis,
 * et atmosphaera dominante methanum.
 */

#ifndef PLANETA_GLACIALE_H
#define PLANETA_GLACIALE_H

#include "../planeta.h"

typedef struct {
    /* atmosphaera */
    double n2;
    double o2;
    double co2;
    double ch4;
    double h2;
    double he;
    double nh3;
    double pulvis;

    /* fasciae */
    int    fasciae;
    double fasciae_contrast;

    /* maculae */
    int    maculae;
    double macula_lat;
    double macula_lon;
    double macula_radius;
} glaciellum_t;

typedef struct {
    tessella_t   avi;
    planetella_t pro;
    glaciellum_t res;
} glaciale_t;

#endif /* PLANETA_GLACIALE_H */
