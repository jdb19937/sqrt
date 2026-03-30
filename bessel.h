/*
 * bessel.h — functiones Bessel approximatae
 *
 * J₀, J₁ approximationes polynomiales et inversa J₀
 * per Newton-Raphson. Independens a libm j0/j1.
 *
 * Approximationes Abramowitz & Stegun (1964) §9.4,
 * errore maximo < 5e-8 pro |x| ≤ 8.
 */

#ifndef BESSEL_H
#define BESSEL_H

#include <math.h>

/* ================================================================
 * J₀(x) — functio Bessel primi generis, ordinis 0
 *
 * Abramowitz & Stegun 9.4.1 / 9.4.3
 * ================================================================ */

static inline double bessel_j0(double x)
{
    double ax = fabs(x);

    if (ax < 8.0) {
        double y = x * x;
        return (57568490574.0
              + y * (-13362590354.0
              + y * (651619640.7
              + y * (-11214424.18
              + y * (77392.33017
              + y * (-184.9052456))))))
              / (57568490411.0
              + y * (1029532985.0
              + y * (9494680.718
              + y * (59272.64853
              + y * (267.8532712
              + y * 1.0)))));
    }

    double z = 8.0 / ax;
    double y = z * z;
    double xx = ax - 0.785398164;

    double p0 = 1.0
              + y * (-0.1098628627e-2
              + y * (0.2734510407e-4
              + y * (-0.2073370639e-5
              + y * 0.2093887211e-6)));

    double q0 = -0.1562499995e-1
              + y * (0.1430488765e-3
              + y * (-0.6911147651e-5
              + y * (0.7621095161e-6
              + y * (-0.934935152e-7))));

    return sqrt(0.636619772 / ax) * (p0 * cos(xx) - z * q0 * sin(xx));
}

/* ================================================================
 * J₁(x) — functio Bessel primi generis, ordinis 1
 *
 * Abramowitz & Stegun 9.4.4 / 9.4.6
 * ================================================================ */

static inline double bessel_j1(double x)
{
    double ax = fabs(x);

    if (ax < 8.0) {
        double y = x * x;
        double r = x * (72362614232.0
              + y * (-7895059235.0
              + y * (242396853.1
              + y * (-2972611.439
              + y * (15704.48260
              + y * (-30.16036606))))))
              / (144725228442.0
              + y * (2300535178.0
              + y * (18583304.74
              + y * (99447.43394
              + y * (376.9991397
              + y * 1.0)))));
        return r;
    }

    double z = 8.0 / ax;
    double y = z * z;
    double xx = ax - 2.356194491;

    double p1 = 1.0
              + y * (0.183105e-2
              + y * (-0.3516396496e-4
              + y * (0.2457520174e-5
              + y * (-0.240337019e-6))));

    double q1 = 0.04687499995
              + y * (-0.2002690873e-3
              + y * (0.8449199096e-5
              + y * (-0.88228987e-6
              + y * 0.105787412e-6)));

    double ans = sqrt(0.636619772 / ax) * (p1 * cos(xx) - z * q1 * sin(xx));
    return (x < 0.0) ? -ans : ans;
}

/* ================================================================
 * J₀⁻¹(x) — inversa functionis J₀ in [0, j₀₁]
 *
 * Pro x ∈ [0, 1], reddit y tale ut J₀(y) = x.
 * Newton-Raphson: yₙ₊₁ = yₙ + (J₀(yₙ) - x) / J₁(yₙ)
 *
 * Ex codice Hevea (Borrelli et al. 2012):
 *   initium: 2.404826 (prima radix J₀)
 *   tolerantia: 1e-9
 * ================================================================ */

static inline double bessel_j0_inversa(double x)
{
    if (x >= 1.0) return 0.0;
    if (x <= 0.0) return 2.404825557695773;  /* prima radix J₀ */

    double a = 2.404825557695773;
    double b;
    do {
        b = a;
        double j1a = bessel_j1(a);
        if (fabs(j1a) < 1e-15) break;
        a = b + (bessel_j0(b) - x) / j1a;
    } while (fabs(a - b) > 1e-9);

    return (a > 0.0) ? a : 0.0;
}

#endif /* BESSEL_H */
