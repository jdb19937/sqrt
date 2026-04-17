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
 * arithmetica campi GF(2^n)
 * ================================================================ */

typedef unsigned int gf_t;

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

static gf_t gf_polynomium(int n)
{
    const gf_praedefinitum_t *gf = gf_quaere_praedefinitum(n);
    if (!gf)
        erratum("polynomium pro hoc n non invenitur");
    return gf->polynomium;
}

/* multiplicatio in GF(2^n): multiplicatio polynomialis mod p */
static gf_t gf_multiplica(gf_t a, gf_t b, int n, gf_t poly)
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
static gf_t gf_quadra(gf_t a, int n, gf_t poly)
{
    return gf_multiplica(a, a, n, poly);
}

/* inversio per potestiam: a^{-1} = a^{2^n - 2} */
static gf_t gf_inverte(gf_t a, int n, gf_t poly)
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
static gf_t gf_potestia(gf_t a, unsigned e, int n, gf_t poly)
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

static const punctum_t INFINITUM = { 0, 0 };

static int est_infinitum(punctum_t p)
{
    return p.x == 0 && p.y == 0;
}

static int est_punctum_in_curva(punctum_t p, const curva_t *c)
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

static punctum_t curva_adde(punctum_t p, punctum_t q, const curva_t *c)
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

static punctum_t curva_multiplica(int k, punctum_t p, const curva_t *c)
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

static unsigned curva_ordo_puncti(punctum_t p, const curva_t *c)
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
    /* n    a    b   gx      gy      ordo      |GF|=2^n  */
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
 * permuta — delegat ad permutationem specificam per p.type
 * ================================================================ */

void permuta(permutatio_t p, locus_t *loci, int numerus, int modulus, int k)
{
    switch (p) {
    case PERMUTATIO_CURVA:  permuta_curva(loci, numerus, modulus, k);  break;
    case PERMUTATIO_DUCTUS: permuta_ductus(loci, numerus, modulus, k); break;
    case PERMUTATIO_FELES:  permuta_feles(loci, numerus, modulus, k);  break;
    default:                erratum("permutatio typus ignotus");
    }
}

/* ================================================================
 * permuta_curva — permutatio per curvam ellipticam
 * ================================================================ */

void permuta_curva(locus_t *loci, int numerus, int modulus, int k)
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
        if (loci[i].u < 0 || loci[i].u >= modulus)
            erratum("locus u extra fines");
        if (loci[i].v < 0 || loci[i].v >= modulus)
            erratum("locus v extra fines");

        punctum_t p;
        p.x = (gf_t)loci[i].u;
        p.y = (gf_t)loci[i].v;

        if (!est_punctum_in_curva(p, &curva))
            erratum("locus non est punctum curvae validum");

        punctum_t r = curva_adde(p, kG, &curva);
        loci[i].u   = (int)r.x;
        loci[i].v   = (int)r.y;
    }
}

/* ================================================================
 * permuta_ductus — (x,y) → (2·x, 2·y) k vicibus
 *
 * elementum 2 = polynomium x in GF(2^n). Si polynomium definiens
 * est primitivum, ordo = 2^n − 1.
 * 2^k computatur per quadraturam repetitam in campo.
 * ================================================================ */

ductus_praedefinitum_t ductus_quaere(int n)
{
    if (n < 2 || n > 16)
        erratum("n extra fines pro ductu");
    int modulus = 1 << n;
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");
    ductus_praedefinitum_t dp;
    dp.n    = n;
    dp.gen  = 2;
    dp.ordo = ductus_ordo(modulus);
    return dp;
}

/* periodus: minimum k ubi 2^k = 1 in GF(2^n). = 2^n - 1 si polynomium primitivum */
unsigned ductus_ordo(int modulus)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");

    int n = log2_integer(modulus);
    const gf_praedefinitum_t *gf = gf_quaere_praedefinitum(n);
    if (!gf)
        erratum("polynomium praedefinitum pro hoc n non invenitur");
    gf_t poly  = gf->polynomium;
    unsigned m = ((unsigned)1 << n) - 1;
    gf_t a     = 1;
    unsigned k = 0;
    do {
        a = gf_multiplica(a, 2, n, poly);
        k++;
    } while (a != 1 && k <= m);
    return k;
}

void permuta_ductus(locus_t *loci, int numerus, int modulus, int k)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");

    int n = log2_integer(modulus);

    const gf_praedefinitum_t *gf = gf_quaere_praedefinitum(n);
    if (!gf)
        erratum("polynomium praedefinitum pro hoc n non invenitur");

    gf_t poly = gf->polynomium;

    /* computa 2^k in GF(2^n) */
    gf_t factor = gf_potestia(2, (unsigned)k, n, poly);

    for (int i = 0; i < numerus; i++) {
        if (loci[i].u < 0 || loci[i].u >= modulus)
            erratum("locus u extra fines");
        if (loci[i].v < 0 || loci[i].v >= modulus)
            erratum("locus v extra fines");

        gf_t u    = (gf_t)loci[i].u;
        gf_t v    = (gf_t)loci[i].v;
        loci[i].u = (int)gf_multiplica(u, factor, n, poly);
        loci[i].v = (int)gf_multiplica(v, factor, n, poly);
    }
}

/* ================================================================
 * permuta_feles — mappa felis Arnoldi
 *
 * [[2,1],[1,1]] applicata k vicibus ad (u,v) mod m.
 * matrix potestiatur per quadraturam repetitam.
 * ================================================================ */

void permuta_feles(locus_t *loci, int numerus, int modulus, int k)
{
    if (!est_potestas_duo(modulus))
        erratum("modulus non est potestas duo");

    for (int i = 0; i < numerus; i++) {
        if (loci[i].u < 0 || loci[i].u >= modulus)
            erratum("locus u extra fines");
        if (loci[i].v < 0 || loci[i].v >= modulus)
            erratum("locus v extra fines");
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
        long u    = loci[i].u;
        long v    = loci[i].v;
        loci[i].u = (int)((ra * u + rb * v) % modulus);
        loci[i].v = (int)((rc * u + rd * v) % modulus);
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

/* ================================================================
 * probationes internae
 * ================================================================ */

static int proba_errores = 0;

static void expecta(int condicio, const char *nuntius)
{
    if (!condicio) {
        fprintf(stderr, "FALSUM: %s\n", nuntius);
        proba_errores++;
    }
}

/* --- probationes campi GF(2^n) --- */

static void proba_gf_basica(void)
{
    fprintf(stderr, "--- GF(2^n) basica ---\n");

    int n     = 4;
    gf_t poly = gf_polynomium(n);

    /* 0 * a = 0 */
    expecta(gf_multiplica(0, 0x5, n, poly) == 0, "0 * a = 0");

    /* 1 * a = a */
    expecta(gf_multiplica(1, 0x7, n, poly) == 0x7, "1 * a = a");

    /* a * 1 = a */
    expecta(gf_multiplica(0x7, 1, n, poly) == 0x7, "a * 1 = a");

    /* commutativitas */
    gf_t ab = gf_multiplica(0x5, 0x3, n, poly);
    gf_t ba = gf_multiplica(0x3, 0x5, n, poly);
    expecta(ab == ba, "a*b = b*a");

    /* associativitas */
    gf_t a    = 0x7, b = 0x5, c = 0x3;
    gf_t ab_c = gf_multiplica(gf_multiplica(a, b, n, poly), c, n, poly);
    gf_t a_bc = gf_multiplica(a, gf_multiplica(b, c, n, poly), n, poly);
    expecta(ab_c == a_bc, "(a*b)*c = a*(b*c)");
}

static void proba_gf_inversio(void)
{
    fprintf(stderr, "--- GF(2^n) inversio ---\n");

    int n     = 4;
    gf_t poly = gf_polynomium(n);
    gf_t sz   = (gf_t)1 << n;

    int bona = 1;
    for (gf_t a = 1; a < sz; a++) {
        gf_t inv  = gf_inverte(a, n, poly);
        gf_t prod = gf_multiplica(a, inv, n, poly);
        if (prod != 1) {
            fprintf(
                stderr, "  inversio fallax: a=0x%X inv=0x%X prod=0x%X\n",
                a, inv, prod
            );
            bona = 0;
        }
    }
    expecta(bona, "a * a^{-1} = 1 pro omni a in GF(2^4)");
}

static void proba_gf_generator(void)
{
    fprintf(stderr, "--- GF(2^n) generator ---\n");

    for (int n = 2; n <= 8; n++) {
        gf_t poly     = gf_polynomium(n);
        gf_t gen      = 2;
        unsigned ordo = ((unsigned)1 << n) - 1;

        gf_t pot = 1;
        unsigned k;
        for (k = 1; k < ordo; k++) {
            pot = gf_multiplica(pot, gen, n, poly);
            if (pot == 1)
                break;
        }
        char buf[80];
        snprintf(buf, sizeof(buf), "GF(2^%d) generator ordo = %u", n, ordo);
        expecta(k == ordo, buf);
    }
}

/* --- probationes curvae ellipticae --- */

static void proba_ec_infinitum(void)
{
    fprintf(stderr, "--- EC infinitum ---\n");

    expecta(est_infinitum(INFINITUM), "INFINITUM est infinitum");
    punctum_t p = { 1, 1 };
    expecta(!est_infinitum(p), "(1,1) non est infinitum");
}

static void proba_ec_curva(void)
{
    fprintf(stderr, "--- EC in_curva ---\n");

    int n = 4;
    curva_t c;
    c.a    = 1;
    c.b    = 1;
    c.n    = n;
    c.poly = gf_polynomium(n);

    expecta(est_punctum_in_curva(INFINITUM, &c), "infinitum in curva");

    gf_t sz = (gf_t)1 << n;
    int numerus_punctorum = 1;
    for (gf_t x = 0; x < sz; x++)
        for (gf_t y = 0; y < sz; y++) {
        if (x == 0 && y == 0)
            continue;
        punctum_t p = { x, y };
        if (est_punctum_in_curva(p, &c))
            numerus_punctorum++;
    }
    fprintf(stderr, "  puncta curvae in GF(2^4): %d\n", numerus_punctorum);
    expecta(numerus_punctorum >= 1, "curva habet puncta");
}

static void proba_ec_additio(void)
{
    fprintf(stderr, "--- EC additio ---\n");

    int n = 4;
    curva_t c;
    c.a    = 1;
    c.b    = 1;
    c.n    = n;
    c.poly = gf_polynomium(n);

    gf_t sz = (gf_t)1 << n;
    punctum_t p = { 0, 0 };
    for (gf_t x = 1; x < sz && est_infinitum(p); x++)
        for (gf_t y = 0; y < sz && est_infinitum(p); y++) {
        punctum_t t = { x, y };
        if (est_punctum_in_curva(t, &c))
            p = t;
    }
    expecta(!est_infinitum(p), "punctum curvae inventum");
    fprintf(stderr, "  P = (0x%X, 0x%X)\n", p.x, p.y);

    punctum_t r = curva_adde(p, INFINITUM, &c);
    expecta(r.x == p.x && r.y == p.y, "P + O = P");

    r = curva_adde(INFINITUM, p, &c);
    expecta(r.x == p.x && r.y == p.y, "O + P = P");

    punctum_t neg_p = { p.x, p.x ^ p.y };
    expecta(est_punctum_in_curva(neg_p, &c), "-P in curva");
    r = curva_adde(p, neg_p, &c);
    expecta(est_infinitum(r), "P + (-P) = O");

    punctum_t q = { 0, 0 };
    for (gf_t x = 1; x < sz; x++)
        for (gf_t y = 0; y < sz; y++) {
        punctum_t t = { x, y };
        if (est_punctum_in_curva(t, &c) && (t.x != p.x || t.y != p.y)) {
            q = t;
            goto inventum;
        }
    }
inventum:
    if (!est_infinitum(q)) {
        r = curva_adde(p, q, &c);
        expecta(est_punctum_in_curva(r, &c), "P + Q in curva");
        fprintf(stderr, "  P+Q = (0x%X, 0x%X)\n", r.x, r.y);
    }
}

static void proba_ec_ordo(void)
{
    fprintf(stderr, "--- EC ordo ---\n");

    for (int n = 2; n <= 8; n++) {
        gf_t poly = gf_polynomium(n);
        curva_t c;
        c.a    = 1;
        c.b    = 1;
        c.n    = n;
        c.poly = poly;

        gf_t sz = (gf_t)1 << n;
        punctum_t p = { 0, 0 };
        for (gf_t x = 1; x < sz && est_infinitum(p); x++)
            for (gf_t y = 0; y < sz && est_infinitum(p); y++) {
            punctum_t t = { x, y };
            if (est_punctum_in_curva(t, &c))
                p = t;
        }
        if (est_infinitum(p))
            continue;

        unsigned ordo = curva_ordo_puncti(p, &c);

        char buf[80];
        snprintf(
            buf, sizeof(buf),
            "GF(2^%d): P=(0x%X,0x%X) ordo=%u", n, p.x, p.y, ordo
        );
        fprintf(stderr, "  %s\n", buf);

        punctum_t r = curva_multiplica((int)ordo, p, &c);
        snprintf(
            buf, sizeof(buf),
            "GF(2^%d): %u*P = O per multiplicam", n, ordo
        );
        expecta(est_infinitum(r), buf);
    }
}

static void proba_curva_multiplica(void)
{
    fprintf(stderr, "--- EC multiplicatio scalaris ---\n");

    int n = 4;
    curva_t c;
    c.a    = 1;
    c.b    = 1;
    c.n    = n;
    c.poly = gf_polynomium(n);

    gf_t sz = (gf_t)1 << n;
    punctum_t p = { 0, 0 };
    for (gf_t x = 1; x < sz && est_infinitum(p); x++)
        for (gf_t y = 0; y < sz && est_infinitum(p); y++) {
        punctum_t t = { x, y };
        if (est_punctum_in_curva(t, &c))
            p = t;
    }

    expecta(est_infinitum(curva_multiplica(0, p, &c)), "0*P = O");

    punctum_t r = curva_multiplica(1, p, &c);
    expecta(r.x == p.x && r.y == p.y, "1*P = P");

    punctum_t dbl_add = curva_adde(p, p, &c);
    punctum_t dbl_mul = curva_multiplica(2, p, &c);
    expecta(dbl_add.x == dbl_mul.x && dbl_add.y == dbl_mul.y, "2*P = P+P");

    punctum_t tpl_add = curva_adde(dbl_add, p, &c);
    punctum_t tpl_mul = curva_multiplica(3, p, &c);
    expecta(tpl_add.x == tpl_mul.x && tpl_add.y == tpl_mul.y, "3*P = P+P+P");

    int bona = 1;
    for (int k = 0; k <= 20; k++) {
        r = curva_multiplica(k, p, &c);
        if (!est_punctum_in_curva(r, &c)) {
            fprintf(stderr, "  %d*P non in curva!\n", k);
            bona = 0;
        }
    }
    expecta(bona, "k*P in curva pro k=0..20");
}

/* --- probationes omnium camporum praedefinitorum --- */

static void proba_omnes_campos(void)
{
    fprintf(stderr, "--- omnes campi praedefiniti ---\n");

    for (int n = 1; n <= 16; n++) {
        gf_t poly = gf_polynomium(n);
        gf_t gen  = n == 1 ? 1 : 2;
        unsigned ordo_expectatus = ((unsigned)1 << n) - 1;
        char buf[120];

        gf_t pot = 1;
        unsigned ordo_inventus = 0;
        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            pot = gf_multiplica(pot, gen, n, poly);
            if (pot == 1) {
                ordo_inventus = k;
                break;
            }
        }

        snprintf(
            buf, sizeof(buf),
            "GF(2^%d): gen=0x%X ordo=%u (expectatus %u)",
            n, gen, ordo_inventus, ordo_expectatus
        );
        expecta(ordo_inventus == ordo_expectatus, buf);

        int minimalis = 1;
        pot = 1;
        for (unsigned k = 1; k < ordo_inventus; k++) {
            pot = gf_multiplica(pot, gen, n, poly);
            if (pot == 1) {
                snprintf(
                    buf, sizeof(buf),
                    "GF(2^%d): gen^%u = 1 ante ordinem", n, k
                );
                expecta(0, buf);
                minimalis = 0;
                break;
            }
        }
        if (minimalis) {
            snprintf(
                buf, sizeof(buf),
                "GF(2^%d): ordo minimalis", n
            );
            expecta(1, buf);
        }

        fprintf(stderr, "  GF(2^%2d): ordo=%u OK\n", n, ordo_inventus);
    }
}

/* ================================================================
 * proba_interna — interfacies publica probationum internarum
 * ================================================================ */

int proba_interna(void)
{
    proba_errores = 0;

    proba_gf_basica();
    proba_gf_inversio();
    proba_gf_generator();
    proba_omnes_campos();
    proba_ec_infinitum();
    proba_ec_curva();
    proba_ec_additio();
    proba_ec_ordo();
    proba_curva_multiplica();

    return proba_errores;
}
