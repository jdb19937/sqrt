/*
 * supergigas.h — sidus supergigas
 */

#ifndef SIDUS_SUPERGIGAS_H
#define SIDUS_SUPERGIGAS_H

#include "../sidus.h"

typedef struct {
    int res;
} supergigulum_t;

typedef struct {
    tessella_t     avi;
    sidulum_t      pro;
    supergigulum_t res;
} supergigas_t;

void reddere_supergigas(unsigned char *fen, const supergigas_t *s, const instrumentum_t *instr);
void supergigas_in_ison(FILE *f, const supergigas_t *s);
void supergigas_ex_ison(supergigas_t *s, const char *ison);

#endif /* SIDUS_SUPERGIGAS_H */
