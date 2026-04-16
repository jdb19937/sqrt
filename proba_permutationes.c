/*
 * proba_permutationes.c — probationes arithmeticae campi et curvae
 */

#include "permutationes.h"

#include <stdio.h>
#include <stdlib.h>

static int errores = 0;

static void expecta(int condicio, const char *nuntius)
{
    if (!condicio) {
        fprintf(stderr, "FALSUM: %s\n", nuntius);
        errores++;
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

/* --- probationes omnium curvarum praedefinitarum --- */

static void proba_omnes_curvas(void)
{
    fprintf(stderr, "--- omnes curvae praedefinitae ---\n");

    for (int n = 2; n <= 16; n++) {
        gf_t poly = gf_polynomium(n);
        const curva_praedefinita_t *cp = curva_quaere(n);
        expecta(cp != NULL, "curva praedefinita inventa");

        curva_t c;
        c.a    = cp->a;
        c.b    = cp->b;
        c.n    = n;
        c.poly = poly;

        punctum_t gen;
        gen.x = cp->gx;
        gen.y = cp->gy;
        unsigned ordo_expectatus = cp->ordo;
        char buf[120];

        snprintf(
            buf, sizeof(buf),
            "EC(2^%d): G=(0x%X,0x%X) in curva", n, gen.x, gen.y
        );
        expecta(est_punctum_in_curva(gen, &c), buf);

        punctum_t acc = gen;
        unsigned ordo_inventus = 0;
        int omnia_in_curva = 1;

        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            if (!est_punctum_in_curva(acc, &c)) {
                snprintf(
                    buf, sizeof(buf),
                    "EC(2^%d): %u*G=(0x%X,0x%X) non in curva",
                    n, k, acc.x, acc.y
                );
                expecta(0, buf);
                omnia_in_curva = 0;
            }
            if (k < ordo_expectatus) {
                if (est_infinitum(acc)) {
                    snprintf(
                        buf, sizeof(buf),
                        "EC(2^%d): %u*G = O ante ordinem %u",
                        n, k, ordo_expectatus
                    );
                    expecta(0, buf);
                    ordo_inventus = k;
                    break;
                }
            }
            if (k == ordo_expectatus) {
                if (est_infinitum(acc)) {
                    ordo_inventus = k;
                } else {
                    snprintf(
                        buf, sizeof(buf),
                        "EC(2^%d): %u*G != O (est (0x%X,0x%X))",
                        n, ordo_expectatus, acc.x, acc.y
                    );
                    expecta(0, buf);
                }
                break;
            }
            acc = curva_adde(acc, gen, &c);
        }

        snprintf(
            buf, sizeof(buf),
            "EC(2^%d): ordo=%u exactus", n, ordo_expectatus
        );
        expecta(ordo_inventus == ordo_expectatus, buf);

        snprintf(
            buf, sizeof(buf),
            "EC(2^%d): omnia k*G in curva", n
        );
        expecta(omnia_in_curva, buf);

        fprintf(
            stderr, "  EC(2^%2d): a=0x%X b=0x%X G=(0x%X,0x%X) ordo=%u OK\n",
            n, cp->a, cp->b, gen.x, gen.y, ordo_inventus
        );
    }
}

/* --- probationes omnium potestiarum praedefinitarum --- */

static void proba_omnes_potestias(void)
{
    fprintf(stderr, "--- omnes potestiae praedefinitae ---\n");

    for (int n = 2; n <= 16; n++) {
        gf_t poly = gf_polynomium(n);
        const potestia_praedefinita_t *pp = potestia_quaere(n);
        expecta(pp != NULL, "potestia praedefinita inventa");

        unsigned e = pp->e;
        unsigned ordo_expectatus = pp->ordo;
        char buf[120];

        /* proba (2,3) coniunctim */
        gf_t ax = 2, ay = 3;
        unsigned ordo_inventus = 0;
        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            ax = gf_potestia(ax, e, n, poly);
            ay = gf_potestia(ay, e, n, poly);
            if (ax == 2 && ay == 3) {
                ordo_inventus = k;
                break;
            }
        }

        snprintf(
            buf, sizeof(buf),
            "pot(2^%d): (2,3) e=%u ordo=%u (expectatus %u)",
            n, e, ordo_inventus, ordo_expectatus
        );
        expecta(ordo_inventus == ordo_expectatus, buf);

        ax = 2;
        ay = 3;
        int minimalis = 1;
        for (unsigned k = 1; k < ordo_inventus; k++) {
            ax = gf_potestia(ax, e, n, poly);
            ay = gf_potestia(ay, e, n, poly);
            if (ax == 2 && ay == 3) {
                snprintf(
                    buf, sizeof(buf),
                    "pot(2^%d): (2,3) redit post %u ante ordinem %u",
                    n, k, ordo_expectatus
                );
                expecta(0, buf);
                minimalis = 0;
                break;
            }
        }
        if (minimalis) {
            snprintf(
                buf, sizeof(buf),
                "pot(2^%d): ordo minimalis", n
            );
            expecta(1, buf);
        }

        fprintf(stderr, "  pot(2^%2d): e=%2u ordo=%5u OK\n", n, e, ordo_expectatus);
    }
}

/* --- probationes omnium felium praedefinitarum --- */

static void proba_omnes_feles(void)
{
    fprintf(stderr, "--- omnes feles praedefinitae ---\n");

    for (int n = 1; n <= 16; n++) {
        const feles_praedefinita_t *fp = feles_quaere(n);
        expecta(fp != NULL, "feles praedefinita inventa");

        unsigned ordo_expectatus = fp->ordo;
        int modulus = 1 << n;
        char buf[120];

        /* confirma feles_ordo() congruit cum praedefinito */
        unsigned ordo_computatus = feles_ordo(modulus);
        snprintf(
            buf, sizeof(buf),
            "feles(2^%d): ordo computatus=%u expectatus=%u",
            n, ordo_computatus, ordo_expectatus
        );
        expecta(ordo_computatus == ordo_expectatus, buf);

        /* proba (2,3) coniunctim per mappam iterativam */
        long x = 2 % modulus, y = 3 % modulus;
        unsigned ordo_inventus = 0;
        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            long nx = (2 * x + y) % modulus;
            long ny = (x + y) % modulus;
            x       = nx;
            y       = ny;
            if (x == 2 % modulus && y == 3 % modulus) {
                ordo_inventus = k;
                break;
            }
        }

        snprintf(
            buf, sizeof(buf),
            "feles(2^%d): (2,3) ordo=%u (expectatus %u)",
            n, ordo_inventus, ordo_expectatus
        );
        expecta(ordo_inventus == ordo_expectatus, buf);

        /* ordo minimalis */
        x = 2 % modulus;
        y = 3 % modulus;
        int minimalis = 1;
        for (unsigned k = 1; k < ordo_inventus; k++) {
            long nx = (2 * x + y) % modulus;
            long ny = (x + y) % modulus;
            x       = nx;
            y       = ny;
            if (x == 2 % modulus && y == 3 % modulus) {
                snprintf(
                    buf, sizeof(buf),
                    "feles(2^%d): (2,3) redit post %u ante ordinem %u",
                    n, k, ordo_expectatus
                );
                expecta(0, buf);
                minimalis = 0;
                break;
            }
        }
        if (minimalis) {
            snprintf(
                buf, sizeof(buf),
                "feles(2^%d): ordo minimalis", n
            );
            expecta(1, buf);
        }

        fprintf(stderr, "  feles(2^%2d): ordo=%5u OK\n", n, ordo_expectatus);
    }
}

/* --- principale --- */

int main(void)
{
    fprintf(stderr, "=== PROBA PERMUTATIONES ===\n\n");

    proba_gf_basica();
    proba_gf_inversio();
    proba_gf_generator();
    proba_ec_infinitum();
    proba_ec_curva();
    proba_ec_additio();
    proba_ec_ordo();
    proba_curva_multiplica();
    proba_omnes_campos();
    proba_omnes_curvas();
    proba_omnes_potestias();
    proba_omnes_feles();

    fprintf(stderr, "\n");
    if (errores == 0)
        fprintf(stderr, "Omnia recta.\n");
    else
        fprintf(stderr, "%d errores inventi.\n", errores);
    return errores ? 1 : 0;
}
