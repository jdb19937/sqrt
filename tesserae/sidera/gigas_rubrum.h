/*
 * gigas_rubrum.h — sidus gigas rubrum
 */

#ifndef SIDUS_GIGAS_RUBRUM_H
#define SIDUS_GIGAS_RUBRUM_H

#include "../sidus.h"

typedef struct {
    int res;
} gigulum_rubrum_t;

typedef struct {
    tessella_t       avi;
    sidulum_t        pro;
    gigulum_rubrum_t res;
} gigas_rubrum_t;

void reddere_gigas_rubrum(unsigned char *fen, const gigas_rubrum_t *s, const instrumentum_t *instr);
void gigas_rubrum_in_ison(FILE *f, const gigas_rubrum_t *s);
void gigas_rubrum_ex_ison(gigas_rubrum_t *s, const char *ison);

#endif /* SIDUS_GIGAS_RUBRUM_H */
