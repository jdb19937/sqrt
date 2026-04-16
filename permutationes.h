/*
 * permutationes.h — permutationes siderum caelorum
 *
 * Duae permutationes in loco: curva elliptica super GF(2^n)
 * et mappa felis Arnoldi super Z/mZ x Z/mZ.
 */

#ifndef PERMUTATIONES_H
#define PERMUTATIONES_H

#include "caela.h"

/* ================================================================
 * arithmetica campi GF(2^n)
 * ================================================================ */

typedef unsigned int gf_t;

gf_t gf_polynomium(int n);
gf_t gf_multiplica(gf_t a, gf_t b, int n, gf_t poly);
gf_t gf_quadra(gf_t a, int n, gf_t poly);
gf_t gf_inverte(gf_t a, int n, gf_t poly);
gf_t gf_potestia(gf_t a, unsigned e, int n, gf_t poly);


/* ================================================================
 * curva elliptica — y^2 + xy = x^3 + a*x^2 + b super GF(2^n)
 * ================================================================ */

typedef struct {
    gf_t a;
    gf_t b;
    int  n;
    gf_t poly;
} curva_t;

typedef struct {
    gf_t x;
    gf_t y;
} punctum_t;

typedef struct {
    int      n;
    gf_t     a;
    gf_t     b;
    gf_t     gx;
    gf_t     gy;
    unsigned ordo;
} curva_praedefinita_t;

extern const punctum_t INFINITUM;

int       est_infinitum(punctum_t p);
int       est_punctum_in_curva(punctum_t p, const curva_t *c);
punctum_t curva_adde(punctum_t p, punctum_t q, const curva_t *c);
punctum_t curva_multiplica(int k, punctum_t p, const curva_t *c);

/* ordo puncti per additionem iterativam */
unsigned curva_ordo_puncti(punctum_t p, const curva_t *c);

const curva_praedefinita_t *curva_quaere(int n);

/*
 * permuta_curva — addit k*G ad omnia sidera in loco.
 * modulus potestas 2 esse debet. omnia (x,y) in [0,modulus) esse debent
 * et puncta curvae valida esse debent.
 */
void permuta_curva(sidus_caeli_t *sidera, int numerus, int modulus, int k);


/* ================================================================
 * potestiatio campi — (x,y) → (x^e, y^e) in GF(2^n)
 * ================================================================ */

typedef struct {
    int      n;
    unsigned e;     /* exponens, gcd(e, 2^n-1) = 1 */
    unsigned ordo;  /* ordo e in (Z/(2^n-1)Z)* */
} potestia_praedefinita_t;

const potestia_praedefinita_t *potestia_quaere(int n);

/* ordo e mod (2^n - 1): minimum k ubi e^k ≡ 1 */
unsigned potestia_ordo(int n, unsigned e);

/*
 * permuta_potestia — applicat (x,y) → (x^e, y^e) k vicibus.
 * modulus potestas 2 esse debet. omnia (x,y) in [0,modulus) esse debent.
 */
void permuta_potestia(sidus_caeli_t *sidera, int numerus, int modulus, int k);


/* ================================================================
 * mappa felis Arnoldi — [[2,1],[1,1]]^k mod 2^n
 * ================================================================ */

typedef struct {
    int      n;
    unsigned ordo;  /* periodus: minimum k ubi M^k = I mod 2^n */
} feles_praedefinita_t;

const feles_praedefinita_t *feles_quaere(int n);

/* periodus mappae felis Arnoldi mod m: minimum k ubi M^k = I */
unsigned feles_ordo(int modulus);

/*
 * permuta_feles — applicat mappam felis Arnoldi k vicibus ad omnia sidera.
 * modulus potestas 2 esse debet. omnia (x,y) in [0,modulus) esse debent.
 */
void permuta_feles(sidus_caeli_t *sidera, int numerus, int modulus, int k);


#endif /* PERMUTATIONES_H */
