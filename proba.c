/*
 * proba.c — probationes operationum communium
 *
 * Probat vectores.h (operationes vectoriales, rotationes),
 * color.h (color_t, gamma_corrigere), et bessel.h (J₀, J₁, J₀⁻¹).
 */

#include "vectores.h"
#include "color.h"
#include "bessel.h"

#include <stdio.h>
#include <math.h>

static int errores = 0;

static void expecta_prope(const char *nomen, double habitus, double expectatus, double epsilon)
{
    double diff = fabs(habitus - expectatus);
    if (diff > epsilon) {
        fprintf(
            stderr, "  MALUM: %s = %.6f, expectatum %.6f (diff %.2e)\n",
            nomen, habitus, expectatus, diff
        );
        errores++;
    }
}

static void expecta_vec3(const char *nomen, vec3_t h, vec3_t e, double epsilon)
{
    double diff = magnitudo(differentia(h, e));
    if (diff > epsilon) {
        fprintf(
            stderr, "  MALUM: %s = (%.4f, %.4f, %.4f), "
            "expectatum (%.4f, %.4f, %.4f)\n",
            nomen, h.x, h.y, h.z, e.x, e.y, e.z
        );
        errores++;
    }
}

/* ================================================================
 * operationes vectoriales
 * ================================================================ */

static void proba_vec3_fundamenta(void)
{
    fprintf(stderr, "vec3 fundamenta:\n");

    vec3_t a = vec3(1.0, 2.0, 3.0);
    vec3_t b = vec3(4.0, 5.0, 6.0);

    expecta_vec3("summa", summa(a, b), vec3(5.0, 7.0, 9.0), 1e-12);
    expecta_vec3("differentia", differentia(b, a), vec3(3.0, 3.0, 3.0), 1e-12);
    expecta_vec3("multiplicare", multiplicare(a, 2.0), vec3(2.0, 4.0, 6.0), 1e-12);

    expecta_prope("productum_scalare", productum_scalare(a, b), 32.0, 1e-12);

    vec3_t c = productum_vectoriale(vec3(1, 0, 0), vec3(0, 1, 0));
    expecta_vec3("productum_vectoriale(x,y)", c, vec3(0, 0, 1), 1e-12);

    fprintf(stderr, "  BENE\n");
}

static void proba_magnitudo(void)
{
    fprintf(stderr, "magnitudo et normalizare:\n");

    expecta_prope("magnitudo(3,4,0)", magnitudo(vec3(3.0, 4.0, 0.0)), 5.0, 1e-12);
    expecta_prope("magnitudo(0,0,0)", magnitudo(vec3(0, 0, 0)), 0.0, 1e-15);
    expecta_prope("magnitudo(1,1,1)", magnitudo(vec3(1, 1, 1)), sqrt(3.0), 1e-12);

    vec3_t n = normalizare(vec3(0.0, 3.0, 4.0));
    expecta_prope("|normalizare|", magnitudo(n), 1.0, 1e-12);
    expecta_vec3("normalizare(0,3,4)", n, vec3(0.0, 0.6, 0.8), 1e-12);

    /* vector nullus — debet (0,0,1) reddere */
    vec3_t z = normalizare(vec3(0, 0, 0));
    expecta_vec3("normalizare(0,0,0)", z, vec3(0, 0, 1), 1e-12);

    fprintf(stderr, "  BENE\n");
}

/* ================================================================
 * rotationes
 * ================================================================ */

static void proba_rotationes(void)
{
    fprintf(stderr, "rotationes:\n");

    double pi2 = PI_GRAECUM / 2.0;

    /* rotare_x: (0,1,0) per π/2 → (0,0,1) */
    expecta_vec3("rotare_x(y,π/2)", rotare_x(vec3(0, 1, 0), pi2), vec3(0, 0, 1), 1e-12);

    /* rotare_y: (1,0,0) per π/2 → (0,0,-1) */
    expecta_vec3("rotare_y(x,π/2)", rotare_y(vec3(1, 0, 0), pi2), vec3(0, 0, -1), 1e-12);

    /* rotare_z: (1,0,0) per π/2 → (0,1,0) */
    expecta_vec3("rotare_z(x,π/2)", rotare_z(vec3(1, 0, 0), pi2), vec3(0, 1, 0), 1e-12);

    /* rotatio per 2π debet identitatem reddere */
    vec3_t p = vec3(1.7, -2.3, 0.5);
    expecta_vec3("rotare_x(2π)", rotare_x(p, DUO_PI), p, 1e-10);
    expecta_vec3("rotare_y(2π)", rotare_y(p, DUO_PI), p, 1e-10);
    expecta_vec3("rotare_z(2π)", rotare_z(p, DUO_PI), p, 1e-10);

    /* rotatio conservat magnitudinem */
    double m = magnitudo(p);
    expecta_prope("|rotare_x|", magnitudo(rotare_x(p, 1.23)), m, 1e-12);
    expecta_prope("|rotare_y|", magnitudo(rotare_y(p, 2.45)), m, 1e-12);
    expecta_prope("|rotare_z|", magnitudo(rotare_z(p, 3.67)), m, 1e-12);

    /* functio_rotandi typus */
    functio_rotandi rot[3] = { rotare_x, rotare_y, rotare_z };
    for (int i = 0; i < 3; i++)
        expecta_prope(
            "functio_rotandi conservat",
            magnitudo(rot[i](p, 0.77)), m, 1e-12
        );

    fprintf(stderr, "  BENE\n");
}

/* ================================================================
 * colores
 * ================================================================ */

static void proba_gamma(void)
{
    fprintf(stderr, "gamma_corrigere:\n");

    /* 0.0 → 0, 1.0 → 255 */
    if (gamma_corrigere(0.0) != 0) {
        fprintf(stderr, "  MALUM: gamma(0.0) = %d\n", gamma_corrigere(0.0));
        errores++;
    }
    if (gamma_corrigere(1.0) != 255) {
        fprintf(stderr, "  MALUM: gamma(1.0) = %d\n", gamma_corrigere(1.0));
        errores++;
    }

    /* limites: valores extra [0,1] saturantur */
    if (gamma_corrigere(-0.5) != 0) {
        fprintf(stderr, "  MALUM: gamma(-0.5) = %d\n", gamma_corrigere(-0.5));
        errores++;
    }
    if (gamma_corrigere(1.5) != 255) {
        fprintf(stderr, "  MALUM: gamma(1.5) = %d\n", gamma_corrigere(1.5));
        errores++;
    }

    /* monotonia: gamma(0.25) < gamma(0.5) < gamma(0.75) */
    unsigned char g25 = gamma_corrigere(0.25);
    unsigned char g50 = gamma_corrigere(0.50);
    unsigned char g75 = gamma_corrigere(0.75);
    if (!(g25 < g50 && g50 < g75)) {
        fprintf(stderr, "  MALUM: monotonia %d, %d, %d\n", g25, g50, g75);
        errores++;
    }

    /* gamma 2.2: medium (0.5) debet circa 186 esse */
    if (g50 < 180 || g50 > 192) {
        fprintf(stderr, "  MALUM: gamma(0.5) = %d, expectatum ~186\n", g50);
        errores++;
    }

    fprintf(stderr, "  BENE\n");
}

static void proba_color_structura(void)
{
    fprintf(stderr, "color_t structura:\n");

    color_t c = {0.5, 0.6, 0.7, 1.0};
    expecta_prope("r", c.r, 0.5, 1e-15);
    expecta_prope("g", c.g, 0.6, 1e-15);
    expecta_prope("b", c.b, 0.7, 1e-15);
    expecta_prope("a", c.a, 1.0, 1e-15);

    /* membra omissa ad zero implentur */
    color_t z = {0};
    expecta_prope("zero.a", z.a, 0.0, 1e-15);

    fprintf(stderr, "  BENE\n");
}

/* ================================================================
 * functiones Bessel
 * ================================================================ */

static void proba_bessel(void)
{
    fprintf(stderr, "bessel_j0:\n");

    /* J₀(0) = 1 */
    expecta_prope("J0(0)", bessel_j0(0.0), 1.0, 1e-7);

    /* J₀(2.4048) ≈ 0 (prima radix) */
    expecta_prope("J0(2.4048)", bessel_j0(2.404825557695773), 0.0, 1e-6);

    /* J₀(1.0) ≈ 0.7651976866 */
    expecta_prope("J0(1.0)", bessel_j0(1.0), 0.7651976866, 1e-6);

    /* J₀(5.0) ≈ -0.1775967713 */
    expecta_prope("J0(5.0)", bessel_j0(5.0), -0.1775967713, 1e-6);

    fprintf(stderr, "  BENE\n");

    fprintf(stderr, "bessel_j1:\n");

    /* J₁(0) = 0 */
    expecta_prope("J1(0)", bessel_j1(0.0), 0.0, 1e-7);

    /* J₁(1.0) ≈ 0.4400505857 */
    expecta_prope("J1(1.0)", bessel_j1(1.0), 0.4400505857, 1e-6);

    /* J₁ antisymmetrica: J₁(-x) = -J₁(x) */
    expecta_prope("J1(-3.0)", bessel_j1(-3.0), -bessel_j1(3.0), 1e-12);

    fprintf(stderr, "  BENE\n");

    fprintf(stderr, "bessel_j0_inversa:\n");

    /* J₀⁻¹(1) = 0 */
    expecta_prope("J0inv(1)", bessel_j0_inversa(1.0), 0.0, 1e-7);

    /* J₀⁻¹(0) ≈ 2.4048 */
    expecta_prope("J0inv(0)", bessel_j0_inversa(0.0), 2.404825557695773, 1e-6);

    /* inversio: J₀(J₀⁻¹(x)) = x */
    double vals[] = {0.1, 0.3, 0.5, 0.7, 0.9};
    for (int i = 0; i < 5; i++) {
        double x = vals[i];
        double y = bessel_j0_inversa(x);
        char nomen[32];
        snprintf(nomen, sizeof(nomen), "J0(J0inv(%.1f))", x);
        expecta_prope(nomen, bessel_j0(y), x, 1e-6);
    }

    /* monotonia: J₀⁻¹ decrescens */
    double inv25 = bessel_j0_inversa(0.25);
    double inv50 = bessel_j0_inversa(0.50);
    double inv75 = bessel_j0_inversa(0.75);
    if (!(inv25 > inv50 && inv50 > inv75)) {
        fprintf(
            stderr, "  MALUM: monotonia J0inv %.4f, %.4f, %.4f\n",
            inv25, inv50, inv75
        );
        errores++;
    }

    fprintf(stderr, "  BENE\n");
}

/* ================================================================
 * principale
 * ================================================================ */

int main(void)
{
    fprintf(stderr, "=== PROBA: vectores.h + color.h + bessel.h ===\n\n");

    proba_vec3_fundamenta();
    proba_magnitudo();
    proba_rotationes();
    proba_gamma();
    proba_color_structura();
    proba_bessel();

    fprintf(
        stderr, "\n%s\n", errores == 0 ? "Omnes probationes praeterierunt."
        : "ERRORES inventi!"
    );
    return errores;
}
