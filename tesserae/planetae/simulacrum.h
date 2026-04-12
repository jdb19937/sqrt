/*
 * simulacrum.h — planeta simulacrum (imago GIF ex plica)
 *
 * Simplex genus: nomen planetae GIF selectat,
 * imago in fenestram redditur.
 */

#ifndef PLANETA_SIMULACRUM_H
#define PLANETA_SIMULACRUM_H

#include "../planeta.h"

#define SIMULACRUM_NOMEN_MAX 64

typedef struct {
    char nomen[SIMULACRUM_NOMEN_MAX];   /* e.g. "vortexa", "cherenkov" */
} simulacrulum_t;

typedef struct {
    tessella_t     avi;
    planetella_t   pro;
    simulacrulum_t res;
} simulacrum_t;

void reddere_simulacrum(unsigned char *fen, const simulacrum_t *p);
void simulacrum_in_ison(FILE *f, const simulacrum_t *s);
void simulacrum_ex_ison(simulacrum_t *s, const char *ison);

#endif /* PLANETA_SIMULACRUM_H */
