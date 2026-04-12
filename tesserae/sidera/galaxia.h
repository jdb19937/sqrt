/*
 * galaxia.h — sidus galaxia (objectum distans)
 *
 * Morphologia Hubble (1926): elliptica, spiralis, barrata,
 * lenticularis, irregularis.
 */

#ifndef SIDUS_GALAXIA_H
#define SIDUS_GALAXIA_H

#include "../sidus.h"

/*
 * Morphologia galaxiae — classificatio Hubble (1926).
 */
typedef enum {
    GALAXIA_ELLIPTICA,          /* E0-E7: spheroidalis */
    GALAXIA_SPIRALIS,           /* Sa-Sd: brachia spiralia */
    GALAXIA_SPIRALIS_BARRATA,   /* SBa-SBd: cum barra centrali */
    GALAXIA_LENTICULARIS,       /* S0: discus sine brachiis */
    GALAXIA_IRREGULARIS,        /* Irr: asymmetrica */
    GALAXIA_NUMERUS
} galaxia_morphologia_t;

typedef struct {
    galaxia_morphologia_t morphologia;    /* classificatio Hubble */
    double                angulus;        /* angulus positionis in caelo */
} galaxiola_t;

typedef struct {
    tessella_t  avi;
    sidulum_t   pro;
    galaxiola_t res;
} galaxia_t;

void reddere_galaxia(unsigned char *fen, const galaxia_t *s, const instrumentum_t *instr);
void galaxia_in_ison(FILE *f, const galaxia_t *s);
void galaxia_ex_ison(galaxia_t *s, const char *ison);

#endif /* SIDUS_GALAXIA_H */
