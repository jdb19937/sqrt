/*
 * vectores.h — operationes vectoriales communes
 *
 * Typi mathematici, operationes vectoriales, rotationes,
 * constantes. Omnia programmata graphica hanc
 * bibliothecam adhibent.
 */

#ifndef VECTORES_H
#define VECTORES_H

#include <math.h>

/* ================================================================
 * constantes
 * ================================================================ */

#define PI_GRAECUM  3.14159265358979323846
#define DUO_PI      (2.0 * PI_GRAECUM)

/* ================================================================
 * typi mathematici
 * ================================================================ */

typedef struct { double x, y, z; } vec3_t;

/* ================================================================
 * operationes vectoriales (inline in capite)
 * ================================================================ */

static inline vec3_t vec3(double x, double y, double z)
{
    return (vec3_t){x, y, z};
}

static inline vec3_t summa(vec3_t a, vec3_t b)
{
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3_t differentia(vec3_t a, vec3_t b)
{
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline double productum_scalare(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3_t productum_vectoriale(vec3_t a, vec3_t b)
{
    return (vec3_t){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline vec3_t multiplicare(vec3_t v, double s)
{
    return (vec3_t){v.x * s, v.y * s, v.z * s};
}

static inline double magnitudo(vec3_t v)
{
    return sqrt(productum_scalare(v, v));
}

static inline vec3_t normalizare(vec3_t v)
{
    double m = magnitudo(v);
    if (m < 1e-15) return (vec3_t){0.0, 0.0, 1.0};
    return multiplicare(v, 1.0 / m);
}

/* ================================================================
 * rotatio punctorum circa axem
 * ================================================================ */

static inline vec3_t rotare_x(vec3_t p, double a)
{
    double ca = cos(a), sa = sin(a);
    return (vec3_t){p.x, p.y * ca - p.z * sa, p.y * sa + p.z * ca};
}

static inline vec3_t rotare_y(vec3_t p, double a)
{
    double ca = cos(a), sa = sin(a);
    return (vec3_t){p.x * ca + p.z * sa, p.y, -p.x * sa + p.z * ca};
}

static inline vec3_t rotare_z(vec3_t p, double a)
{
    double ca = cos(a), sa = sin(a);
    return (vec3_t){p.x * ca - p.y * sa, p.x * sa + p.y * ca, p.z};
}

typedef vec3_t (*functio_rotandi)(vec3_t, double);

#endif /* VECTORES_H */
