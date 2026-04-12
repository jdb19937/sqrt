/*
 * crystallinum.h — sidus crystallinum (materia quark CFL)
 */

#ifndef SIDUS_CRYSTALLINUM_H
#define SIDUS_CRYSTALLINUM_H

#include "../sidus.h"

typedef struct {
    int res;
} crystallulum_t;

typedef struct {
    tessella_t     avi;
    sidulum_t      pro;
    crystallulum_t res;
} crystallinum_t;

void reddere_crystallinum(unsigned char *fen, const crystallinum_t *s, const instrumentum_t *instr);
void crystallinum_in_ison(FILE *f, const crystallinum_t *s);
void crystallinum_ex_ison(crystallinum_t *s, const char *ison);

#endif /* SIDUS_CRYSTALLINUM_H */
