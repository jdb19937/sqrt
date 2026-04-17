/*
 * permutationes.h — permutationes locorum in toro quadrato
 *
 * Tres permutationes in loco super Z/mZ x Z/mZ cum m=2^n:
 *   - curva elliptica super GF(2^n)
 *   - ductus per elementum 2 in GF(2^n)
 *   - mappa felis Arnoldi
 */

#ifndef PERMUTATIONES_H
#define PERMUTATIONES_H

#include "locus.h"

/* ================================================================
 * permutatio — descriptor compositus cum typo et parametris
 * ================================================================ */

#define PERMUTATIO_CURVA   1
#define PERMUTATIO_DUCTUS  2
#define PERMUTATIO_FELES   3

typedef int permutatio_t;

/*
 * permuta — delegat ad permutationem specificam per typum.
 * modulus potestas 2 esse debet. omnia (u,v) in [0,modulus) esse debent.
 */
void permuta(permutatio_t p, locus_t *loci, int numerus, int modulus, int k);

/*
 * permuta_curva — addit k*G ad omnia loca in loco.
 * modulus potestas 2 esse debet. omnia (u,v) in [0,modulus) esse debent
 * et puncta curvae valida esse debent.
 */
void permuta_curva(locus_t *loci, int numerus, int modulus, int k);

/* parametri curvae ellipticae praedefinitae pro n = log2(modulus) */
typedef struct {
    int          n;
    unsigned int a;
    unsigned int b;
    unsigned int gx;
    unsigned int gy;
    unsigned     ordo;  /* ordo generatoris G */
} curva_praedefinita_t;

const curva_praedefinita_t *curva_quaere(int n);

/*
 * permuta_ductus — applicat (u,v) → (2·u, 2·v) k vicibus in GF(2^n).
 * modulus potestas 2 esse debet. omnia (u,v) in [0,modulus) esse debent.
 */
void permuta_ductus(locus_t *loci, int numerus, int modulus, int k);

/* periodus ductus in GF(2^n): minimum k ubi 2^k = 1. = 2^n - 1 si polynomium primitivum */
unsigned ductus_ordo(int modulus);

/* parametri ductus (nulla tabula — quaere computat) */
typedef struct {
    int      n;
    unsigned gen;   /* semper 2 = polynomium x */
    unsigned ordo;
} ductus_praedefinitum_t;

ductus_praedefinitum_t ductus_quaere(int n);

/*
 * permuta_feles — applicat mappam felis Arnoldi k vicibus ad omnia loca.
 * modulus potestas 2 esse debet. omnia (u,v) in [0,modulus) esse debent.
 */
void permuta_feles(locus_t *loci, int numerus, int modulus, int k);

/* periodus mappae felis praedefinita pro n = log2(modulus) */
typedef struct {
    int      n;
    unsigned ordo;  /* periodus: minimum k ubi M^k = I mod 2^n */
} feles_praedefinita_t;

const feles_praedefinita_t *feles_quaere(int n);

/* periodus mappae felis Arnoldi mod m: minimum k ubi M^k = I */
unsigned feles_ordo(int modulus);

/*
 * proba_interna — probationes internae operationum GF(2^n) et curvae ellipticae.
 * reddit numerum errorum inventorum (0 si omnia recta).
 */
int proba_interna(void);

#endif /* PERMUTATIONES_H */
