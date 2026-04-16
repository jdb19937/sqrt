/*
 * permutationes.c — permutationes siderum caelorum
 *
 * Arithmetica campi GF(2^n) cum polynomio definiente,
 * curva elliptica non-supersingularis in forma Weierstrass
 * super GF(2^n), et mappa felis Arnoldi.
 */

#include "permutationes.h"

#include <stdio.h>
#include <stdlib.h>

/* ================================================================
 * erratum — exitio fatalis
 * ================================================================ */

static void erratum(const char *nuntius)
{
    fprintf(stderr, "permutationes: %s\n", nuntius);
    exit(1);
}

/* ================================================================
 * polynomia irreducibilia praedefinita super GF(2)
 *
 * repraesentata ut integer ubi bit i = coefficiens x^i.
 * polynomium gradus n habet bit n positum.
 * ================================================================ */

typedef struct {
    int   n;
    gf_t  polynomium;
} gf_praedefinitum_t;

static const gf_praedefinitum_t praedefinita[] = {
    /* n   polynomium        polynomium irreducibile     */
    {  1,         0x3 },  /* x + 1                       */
    {  2,         0x7 },  /* x^2 + x + 1                 */
    {  3,         0xB },  /* x^3 + x + 1                 */
    {  4,        0x13 },  /* x^4 + x + 1                 */
    {  5,        0x25 },  /* x^5 + x^2 + 1               */
    {  6,        0x43 },  /* x^6 + x + 1                 */
    {  7,        0x89 },  /* x^7 + x^3 + 1               */
    {  8,       0x11D },  /* x^8 + x^4 + x^3 + x^2 + 1   */
    {  9,       0x211 },  /* x^9 + x^4 + 1               */
    { 10,       0x409 },  /* x^10 + x^3 + 1              */
    { 11,       0x805 },  /* x^11 + x^2 + 1              */
    { 12,      0x1053 },  /* x^12 + x^6 + x^4 + x + 1    */
    { 13,      0x201B },  /* x^13 + x^4 + x^3 + x + 1    */
    { 14,      0x4443 },  /* x^14 + x^10 + x^6 + x + 1   */
    { 15,      0x8003 },  /* x^15 + x + 1                */
    { 16,     0x1002D },  /* x^16 + x^5 + x^3 + x^2 + 1  */
    {  0,           0 }
};

static const gf_praedefinitum_t *gf_quaere_praedefinitum(int n)
{
    for (int i = 0; praedefinita[i].n != 0; i++)
        if (praedefinita[i].n == n)
            return &praedefinita[i];
    return NULL;
}

gf_t gf_polynomium(int n)
{
    const gf_praedefinitum_t *gf = gf_quaere_praedefinitum(n);
    if (!gf)
        erratum("polynomium pro hoc n non invenitur");
    return gf->polynomium;
}

/* multiplicatio in GF(2^n): multiplicatio polynomialis mod p */
gf_t gf_multiplica(gf_t a, gf_t b, int n, gf_t poly)
{
    gf_t r = 0;
    for (int i = 0; i < n; i++) {
        if (b & 1)
            r ^= a;
        b >>= 1;
        a <<= 1;
        if (a & ((gf_t)1 << n))
            a ^= poly;
    }
    return r;
}

/* quadratura in GF(2^n) */
gf_t gf_quadra(gf_t a, int n, gf_t poly)
{
    return gf_multiplica(a, a, n, poly);
}

/* inversio per potestiam: a^{-1} = a^{2^n - 2} */
gf_t gf_inverte(gf_t a, int n, gf_t poly)
{
    if (a == 0)
        erratum("inversio nullius in GF(2^n)");
    gf_t r         = 1;
    gf_t b         = a;
    unsigned int e = ((unsigned int)1 << n) - 2;
    while (e) {
        if (e & 1)
            r = gf_multiplica(r, b, n, poly);
        b = gf_quadra(b, n, poly);
        e >>= 1;
    }
    return r;
}

/* a^e in GF(2^n) per quadraturam repetitam */
gf_t gf_potestia(gf_t a, unsigned e, int n, gf_t poly)
{
    if (e == 0)
        return 1;
    if (a == 0)
        return 0;
    gf_t r = 1;
    gf_t b = a;
    while (e) {
        if (e & 1)
            r = gf_multiplica(r, b, n, poly);
        b = gf_quadra(b, n, poly);
        e >>= 1;
    }
    return r;
}

/* ================================================================
 * curva elliptica super GF(2^n)
 *
 * forma Weierstrass non-supersingularis:
 *   y^2 + x*y = x^3 + a*x^2 + b
 * ubi b != 0 (aliter singularis).
 *
 * punctum in infinito: (0, 0) per conventionem.
 * ================================================================ */

const punctum_t INFINITUM = { 0, 0 };

int est_infinitum(punctum_t p)
{
    return p.x == 0 && p.y == 0;
}

int est_punctum_in_curva(punctum_t p, const curva_t *c)
{
    if (est_infinitum(p))
        return 1;
    gf_t x2  = gf_quadra(p.x, c->n, c->poly);
    gf_t x3  = gf_multiplica(x2, p.x, c->n, c->poly);
    gf_t y2  = gf_quadra(p.y, c->n, c->poly);
    gf_t xy  = gf_multiplica(p.x, p.y, c->n, c->poly);
    gf_t sin = y2 ^ xy;
    gf_t dex = x3 ^ gf_multiplica(c->a, x2, c->n, c->poly) ^ c->b;
    return sin == dex;
}

punctum_t curva_adde(punctum_t p, punctum_t q, const curva_t *c)
{
    if (est_infinitum(p))
        return q;
    if (est_infinitum(q))
        return p;

    punctum_t r;

    if (p.x == q.x) {
        if (p.y == q.y) {
            if (p.x == 0)
                return INFINITUM;
            gf_t inv_x  = gf_inverte(p.x, c->n, c->poly);
            gf_t lambda = p.x ^ gf_multiplica(p.y, inv_x, c->n, c->poly);
            r.x         = gf_quadra(lambda, c->n, c->poly) ^ lambda ^ c->a;
            r.y = gf_quadra(p.x, c->n, c->poly)
                ^ gf_multiplica(lambda ^ 1, r.x, c->n, c->poly);
            return r;
        }
        return INFINITUM;
    }

    gf_t dx     = p.x ^ q.x;
    gf_t dy     = p.y ^ q.y;
    gf_t inv_dx = gf_inverte(dx, c->n, c->poly);
    gf_t lambda = gf_multiplica(dy, inv_dx, c->n, c->poly);
    r.x         = gf_quadra(lambda, c->n, c->poly) ^ lambda ^ p.x ^ q.x ^ c->a;
    r.y         = gf_multiplica(lambda, p.x ^ r.x, c->n, c->poly) ^ r.x ^ p.y;
    return r;
}

punctum_t curva_multiplica(int k, punctum_t p, const curva_t *c)
{
    if (k < 0)
        erratum("multiplicatio negativa non sustinetur");
    if (k == 0 || est_infinitum(p))
        return INFINITUM;
    punctum_t r    = INFINITUM;
    punctum_t b    = p;
    unsigned int e = (unsigned int)k;
    while (e) {
        if (e & 1)
            r = curva_adde(r, b, c);
        b = curva_adde(b, b, c);
        e >>= 1;
    }
    return r;
}

unsigned curva_ordo_puncti(punctum_t p, const curva_t *c)
{
    if (est_infinitum(p))
        return 1;
    punctum_t acc  = p;
    unsigned ordo  = 1;
    unsigned limes = (unsigned)1 << (c->n + 1);
    while (!est_infinitum(acc) && ordo < limes) {
        acc = curva_adde(acc, p, c);
        ordo++;
    }
    return ordo;
}

/* ================================================================
 * curvae praedefinitae — y^2 + xy = x^3 + a*x^2 + b
 * ================================================================ */

static const curva_praedefinita_t curvae_praedefinitae[] = {
    /*  n     a    b      gx      gy    ordo    |GF|=2^n  */
    {  2, 0x1, 0x1, 0x2,    0x1,        8 }, /*       4  */
    {  3, 0x2, 0x3, 0x1,    0x0,       12 }, /*       8  */
    {  4, 0x2, 0x6, 0xD,    0x7,       24 }, /*      16  */
    {  5, 0x2, 0x3, 0x3,    0x15,      28 }, /*      32  */
    {  6, 0x2, 0x3, 0x2D,   0x1C,      56 }, /*      64  */
    {  7, 0x2, 0x2, 0x59,   0x7,      144 }, /*     128  */
    {  8, 0x2, 0x2, 0x38,   0x89,     240 }, /*     256  */
    {  9, 0x2, 0x3, 0x36,   0x8D,     540 }, /*     512  */
    { 10, 0x2, 0x3, 0x385,  0x58,    1000 }, /*    1024  */
    { 11, 0x2, 0x3, 0x3F2,  0x4E4,   1988 }, /*    2048  */
    { 12, 0x2, 0x2, 0x80C,  0x4AB,   4128 }, /*    4096  */
    { 13, 0x2, 0x2, 0x676,  0x214,   8320 }, /*    8192  */
    { 14, 0x2, 0x2, 0x3DDE, 0x156C, 16240 }, /*   16384  */
    { 15, 0x2, 0x3, 0x573,  0x30B5, 32532 }, /*   32768  */
    { 16, 0x2, 0x2, 0xE1DD, 0x4EC5, 65088 }, /*   65536  */
    { 0, 0, 0, 0, 0, 0 }
};

const curva_praedefinita_t *curva_quaere(int n)
{
    for (int i = 0; curvae_praedefinitae[i].n != 0; i++)
        if (curvae_praedefinitae[i].n == n)
            return &curvae_praedefinitae[i];
    return NULL;
}

/* ================================================================
 * confirmationes auxiliares
 * ================================================================ */

static int est_potestas_duo(int m)
{
    return m > 0 && (m & (m - 1)) == 0;
}

static int log2_integer(int m)
{
    int n = 0;
    while ((1 << n) < m)
        n++;
    return n;
}

/* ================================================================
 * permuta_curva — permutatio per curvam ellipticam
 * ================================================================ */

void permuta_curva(sidus_caeli_t *sidera, int numerus, int modulus, int k)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");
    if (modulus < 4)
        erratum("modulus nimis parvus pro curva elliptica (minimum 4)");

    int n = log2_integer(modulus);

    const gf_praedefinitum_t *gf = gf_quaere_praedefinitum(n);
    if (!gf)
        erratum("polynomium praedefinitum pro hoc n non invenitur");

    const curva_praedefinita_t *cp = curva_quaere(n);
    if (!cp)
        erratum("curva praedefinita pro hoc n non invenitur");

    curva_t curva;
    curva.a    = cp->a;
    curva.b    = cp->b;
    curva.n    = n;
    curva.poly = gf->polynomium;

    punctum_t gen;
    gen.x = cp->gx;
    gen.y = cp->gy;
    if (!est_punctum_in_curva(gen, &curva))
        erratum("generator praedefinitus non in curva iacet");

    unsigned ordo = cp->ordo;
    int k_red     = ((k % (int)ordo) + (int)ordo) % (int)ordo;

    punctum_t kG = curva_multiplica(k_red, gen, &curva);

    for (int i = 0; i < numerus; i++) {
        if (sidera[i].x < 0 || sidera[i].x >= modulus)
            erratum("sidus x extra fines");
        if (sidera[i].y < 0 || sidera[i].y >= modulus)
            erratum("sidus y extra fines");

        punctum_t p;
        p.x = (gf_t)sidera[i].x;
        p.y = (gf_t)sidera[i].y;

        if (!est_punctum_in_curva(p, &curva))
            erratum("sidus non est punctum curvae validum");

        punctum_t r = curva_adde(p, kG, &curva);
        sidera[i].x = (int)r.x;
        sidera[i].y = (int)r.y;
    }
}

/* ================================================================
 * potestiatio campi — (x,y) → (x^e, y^e)
 * ================================================================ */

static const potestia_praedefinita_t potestia_praedefinitae[] = {
    /* n   e   ordo        2^n-1  */
    {  2,  5,     2 },  /*     3  */
    {  3,  3,     6 },  /*     7  */
    {  4,  7,     4 },  /*    15  */
    {  5,  3,    30 },  /*    31  */
    {  6,  5,     6 },  /*    63  */
    {  7,  3,   126 },  /*   127  */
    {  8,  7,    16 },  /*   255  */
    {  9,  3,    12 },  /*   511  */
    { 10,  5,    30 },  /*  1023  */
    { 11,  3,    88 },  /*  2047  */
    { 12, 11,    12 },  /*  4095  */
    { 13,  3,   910 },  /*  8191  */
    { 14,  5,    42 },  /* 16383  */
    { 15,  3,   150 },  /* 32767  */
    { 16,  7,   256 },  /* 65535  */
    { 0, 0, 0 }
};

const potestia_praedefinita_t *potestia_quaere(int n)
{
    for (int i = 0; potestia_praedefinitae[i].n != 0; i++)
        if (potestia_praedefinitae[i].n == n)
            return &potestia_praedefinitae[i];
    return NULL;
}

/* ordo e mod (2^n - 1): minimum k ubi e^k ≡ 1 */
unsigned potestia_ordo(int n, unsigned e)
{
    if (n < 1)
        return 0;
    unsigned m = ((unsigned)1 << n) - 1;
    if (m <= 1)
        return 1;
    unsigned pot = e % m;
    unsigned k   = 1;
    while (pot != 1 && k <= m) {
        pot = (unsigned)((unsigned long)pot * e % m);
        k++;
    }
    return k;
}

/* ================================================================
 * permuta_potestia — (x,y) → (x^e, y^e) k vicibus
 *
 * k applicata: (x,y) → (x^{e^k}, y^{e^k}).
 * e^k computatur mod (2^n - 1).
 * ================================================================ */

void permuta_potestia(sidus_caeli_t *sidera, int numerus, int modulus, int k)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");

    int n = log2_integer(modulus);

    const gf_praedefinitum_t *gf = gf_quaere_praedefinitum(n);
    if (!gf)
        erratum("polynomium praedefinitum pro hoc n non invenitur");

    const potestia_praedefinita_t *pp = potestia_quaere(n);
    if (!pp)
        erratum("potestia praedefinita pro hoc n non invenitur");

    gf_t poly  = gf->polynomium;
    unsigned m = ((unsigned)1 << n) - 1;  /* ordo gruppi multiplicativi */

    /* computa e^k mod m */
    unsigned e    = pp->e;
    unsigned ek   = 1;
    unsigned base = e % m;
    int kk        = k;
    while (kk > 0) {
        if (kk & 1)
            ek = (unsigned)((unsigned long)ek * base % m);
        base = (unsigned)((unsigned long)base * base % m);
        kk >>= 1;
    }

    for (int i = 0; i < numerus; i++) {
        if (sidera[i].x < 0 || sidera[i].x >= modulus)
            erratum("sidus x extra fines");
        if (sidera[i].y < 0 || sidera[i].y >= modulus)
            erratum("sidus y extra fines");

        gf_t x      = (gf_t)sidera[i].x;
        gf_t y      = (gf_t)sidera[i].y;
        sidera[i].x = (int)gf_potestia(x, ek, n, poly);
        sidera[i].y = (int)gf_potestia(y, ek, n, poly);
    }
}

/* ================================================================
 * permuta_feles — mappa felis Arnoldi
 *
 * [[2,1],[1,1]] applicata k vicibus ad (x,y) mod m.
 * matrix potestiatur per quadraturam repetitam.
 * ================================================================ */

void permuta_feles(sidus_caeli_t *sidera, int numerus, int modulus, int k)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");

    for (int i = 0; i < numerus; i++) {
        if (sidera[i].x < 0 || sidera[i].x >= modulus)
            erratum("sidus x extra fines");
        if (sidera[i].y < 0 || sidera[i].y >= modulus)
            erratum("sidus y extra fines");
    }

    if (k <= 0)
        return;

    long a  = 2, b = 1, c = 1, d = 1;
    long ra = 1, rb = 0, rc = 0, rd = 1;
    int e   = k;

    while (e > 0) {
        if (e & 1) {
            long na = (ra * a + rb * c) % modulus;
            long nb = (ra * b + rb * d) % modulus;
            long nc = (rc * a + rd * c) % modulus;
            long nd = (rc * b + rd * d) % modulus;
            ra      = na;
            rb      = nb;
            rc      = nc;
            rd      = nd;
        }
        long na = (a * a + b * c) % modulus;
        long nb = (a * b + b * d) % modulus;
        long nc = (c * a + d * c) % modulus;
        long nd = (c * b + d * d) % modulus;
        a       = na;
        b       = nb;
        c       = nc;
        d       = nd;
        e >>= 1;
    }

    for (int i = 0; i < numerus; i++) {
        long x      = sidera[i].x;
        long y      = sidera[i].y;
        sidera[i].x = (int)((ra * x + rb * y) % modulus);
        sidera[i].y = (int)((rc * x + rd * y) % modulus);
    }
}

/* ================================================================
 * mappa felis Arnoldi — periodi praedefinitae
 * ================================================================ */

static const feles_praedefinita_t feles_praedefinitae[] = {
    /*  n   ordo    modulus=2^n */
    {  1,      3 }, /*       2  */
    {  2,      3 }, /*       4  */
    {  3,      6 }, /*       8  */
    {  4,     12 }, /*      16  */
    {  5,     24 }, /*      32  */
    {  6,     48 }, /*      64  */
    {  7,     96 }, /*     128  */
    {  8,    192 }, /*     256  */
    {  9,    384 }, /*     512  */
    { 10,    768 }, /*    1024  */
    { 11,   1536 }, /*    2048  */
    { 12,   3072 }, /*    4096  */
    { 13,   6144 }, /*    8192  */
    { 14,  12288 }, /*   16384  */
    { 15,  24576 }, /*   32768  */
    { 16,  49152 }, /*   65536  */
    { 0, 0 }
};

const feles_praedefinita_t *feles_quaere(int n)
{
    for (int i = 0; feles_praedefinitae[i].n != 0; i++)
        if (feles_praedefinitae[i].n == n)
            return &feles_praedefinitae[i];
    return NULL;
}

/* periodus mappae felis: minimum k ubi [[2,1],[1,1]]^k = I mod m */
unsigned feles_ordo(int modulus)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");

    long m     = modulus;
    long a     = 2 % m, b = 1 % m, c = 1 % m, d = 1 % m;
    unsigned k = 1;

    while (a != 1 || b != 0 || c != 0 || d != 1) {
        long na = (a * 2 + b) % m;
        long nb = (a + b)     % m;
        long nc = (c * 2 + d) % m;
        long nd = (c + d)     % m;
        a       = na;
        b       = nb;
        c       = nc;
        d       = nd;
        k++;
    }
    return k;
}
