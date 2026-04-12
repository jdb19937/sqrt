/*
 * neutronium.h — sidus neutronium
 */

#ifndef SIDUS_NEUTRONIUM_H
#define SIDUS_NEUTRONIUM_H

#include "../sidus.h"

typedef struct {
    int res;
} neutroniulum_t;

typedef struct {
    tessella_t     avi;
    sidulum_t      pro;
    neutroniulum_t res;
} neutronium_t;

void reddere_neutronium(unsigned char *fen, const neutronium_t *s, const instrumentum_t *instr);
void neutronium_in_ison(FILE *f, const neutronium_t *s);
void neutronium_ex_ison(neutronium_t *s, const char *ison);

#endif /* SIDUS_NEUTRONIUM_H */
