/*
 * orbita.c — orbita periodica in toro quadrato
 */

#include "orbita.h"
#include "ison.h"

#include <math.h>
#include <string.h>

#define DUO_PI (2.0 * 3.14159265358979323846)

void orbita_ex_ison(orbita_t *o, const char *ison)
{
    memset(o, 0, sizeof(*o));
    o->cx             = ison_da_f(ison, "cx", 0.0);
    o->cy             = ison_da_f(ison, "cy", 0.0);
    o->amplitudo_x    = ison_da_f(ison, "amplitudo_x", 0.0);
    o->amplitudo_y    = ison_da_f(ison, "amplitudo_y", 0.0);
    o->frequentia_x   = (int)ison_da_n(ison, "frequentia_x", 1);
    o->frequentia_y   = (int)ison_da_n(ison, "frequentia_y", 1);
    o->phase_x        = ison_da_f(ison, "phase_x", 0.0);
    o->phase_y        = ison_da_f(ison, "phase_y", 0.0);
    o->revolutiones_x = (int)ison_da_n(ison, "revolutiones_x", 0);
    o->revolutiones_y = (int)ison_da_n(ison, "revolutiones_y", 0);
    o->periodus       = (int)ison_da_n(ison, "periodus", 1);
    o->cz             = ison_da_f(ison, "cz", 0.0);
    o->amplitudo_z    = ison_da_f(ison, "amplitudo_z", 0.0);
    o->phase_z        = ison_da_f(ison, "phase_z", 0.0);
}

void orbita_in_ison(FILE *f, const orbita_t *o)
{
    fprintf(
        f, "{\"cx\": %.1f, \"cy\": %.1f"
        ", \"amplitudo_x\": %.1f, \"amplitudo_y\": %.1f"
        ", \"frequentia_x\": %d, \"frequentia_y\": %d"
        ", \"phase_x\": %.3f, \"phase_y\": %.3f"
        ", \"revolutiones_x\": %d, \"revolutiones_y\": %d"
        ", \"periodus\": %d"
        ", \"cz\": %.3f, \"amplitudo_z\": %.3f, \"phase_z\": %.3f}",
        o->cx, o->cy,
        o->amplitudo_x, o->amplitudo_y,
        o->frequentia_x, o->frequentia_y,
        o->phase_x, o->phase_y,
        o->revolutiones_x, o->revolutiones_y,
        o->periodus,
        o->cz, o->amplitudo_z, o->phase_z
    );
}

void orbita_computare(
    const orbita_t *o,
    int t,
    int latitudo, int altitudo,
    double *x, double *y, double *z
) {
    int P       = o->periodus > 0 ? o->periodus : 1;
    double frac = (double)t / (double)P;

    double px = o->cx
        + o->amplitudo_x * sin(DUO_PI * o->frequentia_x * frac + o->phase_x)
        + (double)latitudo * o->revolutiones_x * frac;

    double py = o->cy
        + o->amplitudo_y * sin(DUO_PI * o->frequentia_y * frac + o->phase_y)
        + (double)altitudo * o->revolutiones_y * frac;

    /* mod toroidale */
    px = fmod(px, (double)latitudo);
    if (px < 0)
        px += latitudo;
    py = fmod(py, (double)altitudo);
    if (py < 0)
        py += altitudo;

    *x = px;
    *y = py;
    *z = o->cz + o->amplitudo_z * sin(DUO_PI * frac + o->phase_z);
}
