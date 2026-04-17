/*
 * proba_permutationes.c — probationes permutationum
 *
 * Probationes operationum internarum (GF(2^n), curva elliptica) vocantur
 * per proba_interna(). Probationes ipsarum permutationum hic definiuntur
 * et usum faciunt APIum publici (permuta_curva, permuta_ductus, permuta_feles).
 */

#include "permutationes.h"

#include <stdio.h>

static int errores = 0;

static void expecta(int condicio, const char *nuntius)
{
    if (!condicio) {
        fprintf(stderr, "FALSUM: %s\n", nuntius);
        errores++;
    }
}

/* --- probationes omnium curvarum praedefinitarum --- */

static void proba_omnes_curvas(void)
{
    fprintf(stderr, "--- omnes curvae praedefinitae ---\n");

    for (int n = 2; n <= 16; n++) {
        int modulus = 1 << n;
        const curva_praedefinita_t *cp = curva_quaere(n);
        expecta(cp != NULL, "curva praedefinita inventa");
        if (!cp)
            continue;

        unsigned ordo_expectatus = cp->ordo;
        char buf[120];

        locus_t loc = { (int)cp->gx, (int)cp->gy };
        int gu = loc.u, gv = loc.v;

        unsigned ordo_inventus = 0;
        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            permuta_curva(&loc, 1, modulus, 1);
            if (loc.u == gu && loc.v == gv) {
                ordo_inventus = k;
                break;
            }
        }

        snprintf(
            buf, sizeof(buf),
            "EC(2^%d): G ordo=%u (expectatus %u)",
            n, ordo_inventus, ordo_expectatus
        );
        expecta(ordo_inventus == ordo_expectatus, buf);

        /* singula vocatio k=ordo debet esse identitas */
        loc.u = gu;
        loc.v = gv;
        permuta_curva(&loc, 1, modulus, (int)ordo_expectatus);
        snprintf(
            buf, sizeof(buf),
            "EC(2^%d): permuta_curva(k=%u) identitas", n, ordo_expectatus
        );
        expecta(loc.u == gu && loc.v == gv, buf);

        /* ordo minimalis */
        loc.u         = gu;
        loc.v         = gv;
        int minimalis = 1;
        for (unsigned k = 1; k < ordo_inventus; k++) {
            permuta_curva(&loc, 1, modulus, 1);
            if (loc.u == gu && loc.v == gv) {
                snprintf(
                    buf, sizeof(buf),
                    "EC(2^%d): G redit post %u ante ordinem %u",
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
                "EC(2^%d): ordo minimalis", n
            );
            expecta(1, buf);
        }

        fprintf(
            stderr, "  EC(2^%2d): a=0x%X b=0x%X G=(0x%X,0x%X) ordo=%u OK\n",
            n, cp->a, cp->b, cp->gx, cp->gy, ordo_inventus
        );
    }
}

/* --- probationes omnium ductuum praedefinitorum --- */

static void proba_omnes_ductus(void)
{
    fprintf(stderr, "--- omnes ductus praedefiniti ---\n");

    for (int n = 2; n <= 16; n++) {
        int modulus = 1 << n;
        unsigned ordo_expectatus = ductus_ordo(modulus);
        char buf[120];

        /* confirma ductus_ordo = 2^n - 1 (polynomium primitivum) */
        snprintf(
            buf, sizeof(buf),
            "duc(2^%d): ductus_ordo=%u (2^n-1=%u)",
            n, ordo_expectatus, ((unsigned)1 << n) - 1
        );
        expecta(ordo_expectatus == ((unsigned)1 << n) - 1, buf);

        /* proba (2,3) coniunctim per permuta_ductus iterativum */
        locus_t loc = { 2, 3 };
        unsigned ordo_inventus = 0;
        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            permuta_ductus(&loc, 1, modulus, 1);
            if (loc.u == 2 && loc.v == 3) {
                ordo_inventus = k;
                break;
            }
        }

        snprintf(
            buf, sizeof(buf),
            "duc(2^%d): (2,3) ordo=%u (expectatus %u)",
            n, ordo_inventus, ordo_expectatus
        );
        expecta(ordo_inventus == ordo_expectatus, buf);

        /* singula vocatio k=ordo debet esse identitas */
        loc.u = 2;
        loc.v = 3;
        permuta_ductus(&loc, 1, modulus, (int)ordo_expectatus);
        snprintf(
            buf, sizeof(buf),
            "duc(2^%d): permuta_ductus(k=%u) identitas", n, ordo_expectatus
        );
        expecta(loc.u == 2 && loc.v == 3, buf);

        loc.u         = 2;
        loc.v         = 3;
        int minimalis = 1;
        for (unsigned k = 1; k < ordo_inventus; k++) {
            permuta_ductus(&loc, 1, modulus, 1);
            if (loc.u == 2 && loc.v == 3) {
                snprintf(
                    buf, sizeof(buf),
                    "duc(2^%d): (2,3) redit post %u ante ordinem %u",
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
                "duc(2^%d): ordo minimalis", n
            );
            expecta(1, buf);
        }

        fprintf(stderr, "  duc(2^%2d): ordo=%5u OK\n", n, ordo_expectatus);
    }
}

/* --- probationes omnium felium praedefinitarum --- */

static void proba_omnes_feles(void)
{
    fprintf(stderr, "--- omnes feles praedefinitae ---\n");

    for (int n = 1; n <= 16; n++) {
        const feles_praedefinita_t *fp = feles_quaere(n);
        expecta(fp != NULL, "feles praedefinita inventa");
        if (!fp)
            continue;

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

        /* proba (2,3) coniunctim per permuta_feles iterativam */
        locus_t loc = { 2 % modulus, 3 % modulus };
        int u0 = loc.u, v0 = loc.v;
        unsigned ordo_inventus = 0;
        for (unsigned k = 1; k <= ordo_expectatus; k++) {
            permuta_feles(&loc, 1, modulus, 1);
            if (loc.u == u0 && loc.v == v0) {
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

        /* singula vocatio k=ordo debet esse identitas */
        loc.u = u0;
        loc.v = v0;
        permuta_feles(&loc, 1, modulus, (int)ordo_expectatus);
        snprintf(
            buf, sizeof(buf),
            "feles(2^%d): permuta_feles(k=%u) identitas", n, ordo_expectatus
        );
        expecta(loc.u == u0 && loc.v == v0, buf);

        /* ordo minimalis */
        loc.u         = u0;
        loc.v         = v0;
        int minimalis = 1;
        for (unsigned k = 1; k < ordo_inventus; k++) {
            permuta_feles(&loc, 1, modulus, 1);
            if (loc.u == u0 && loc.v == v0) {
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

    errores += proba_interna();
    proba_omnes_curvas();
    proba_omnes_ductus();
    proba_omnes_feles();

    fprintf(stderr, "\n");
    if (errores == 0)
        fprintf(stderr, "Omnia recta.\n");
    else
        fprintf(stderr, "%d errores inventi.\n", errores);
    return errores ? 1 : 0;
}
