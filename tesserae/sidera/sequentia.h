/*
 * sequentia.h — sidus sequentiae principalis
 */

#ifndef SIDUS_SEQUENTIA_H
#define SIDUS_SEQUENTIA_H

#include "../sidus.h"

typedef struct {
    int res;
} sequentiola_t;

typedef struct {
    tessella_t    avi;
    sidulum_t     pro;
    sequentiola_t res;
} sequentia_t;

void reddere_sequentia(unsigned char *fen, const sequentia_t *s, const instrumentum_t *instr);
void sequentia_in_ison(FILE *f, const sequentia_t *s);
void sequentia_ex_ison(sequentia_t *s, const char *ison);

#endif /* SIDUS_SEQUENTIA_H */
