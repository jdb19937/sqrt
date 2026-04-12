/*
 * orbita.h — orbita periodica in toro quadrato
 *
 * Motus periodicus planetae in campo toroidali.
 * Positio et scala computantur ex passu temporis.
 *
 * x(t) = cx + amplitudo_x · sin(2π · freq_x · t/P + phase_x) + lat · rev_x · t/P
 * y(t) = cy + amplitudo_y · sin(2π · freq_y · t/P + phase_y) + alt · rev_y · t/P
 * scala(t) = scala + scala_amplitudo · sin(2π · t/P + scala_phase)
 *
 * Omnia mod (latitudo, altitudo) pro topologia toroidali.
 * Periodicitas guarantitur quia freq et rev integri sunt.
 */

#ifndef ORBITA_H
#define ORBITA_H

#include <stdio.h>

typedef struct {
    double cx, cy;              /* centrum */
    double amplitudo_x;         /* amplitudo oscillationis x */
    double amplitudo_y;         /* amplitudo oscillationis y */
    int    frequentia_x;        /* multiplicator frequentiae x */
    int    frequentia_y;        /* multiplicator frequentiae y */
    double phase_x;             /* phase initialis x (radiani) */
    double phase_y;             /* phase initialis y (radiani) */
    int    revolutiones_x;      /* circumvolutiones toroidales x per periodum */
    int    revolutiones_y;      /* circumvolutiones toroidales y per periodum */
    int    periodus;            /* periodus in passibus temporis */
    double cz;                  /* centrum z */
    double amplitudo_z;         /* amplitudo oscillationis z */
    double phase_z;             /* phase z (radiani) */
} orbita_t;

/* legit orbitam ex ISON */
void orbita_ex_ison(orbita_t *o, const char *ison);

/* scribit orbitam in FILE */
void orbita_in_ison(FILE *f, const orbita_t *o);

/* computat positionem et z ad passum temporis t */
void orbita_computare(
    const orbita_t *o,
    int t,
    int latitudo, int altitudo,
    double *x, double *y, double *z
);

#endif /* ORBITA_H */
