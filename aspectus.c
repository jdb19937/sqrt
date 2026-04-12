/*
 * aspectus.c — aspectus illuminationis planetarum
 */

#include "aspectus.h"
#include "ison.h"

planeta_aspectus_t planeta_aspectus_ex_ison(const char *ison)
{
    planeta_aspectus_t a;
    if (!ison) {
        a.situs   = 0.0;
        a.angulus = 0.0;
        a.lumen   = 1.0;
        return a;
    }
    a.situs   = ison_da_f(ison, "situs",   0.0);
    a.angulus = ison_da_f(ison, "angulus", 0.0);
    a.lumen   = ison_da_f(ison, "lumen",   1.0);
    return a;
}

void planeta_aspectus_in_ison(FILE *f, const planeta_aspectus_t *a)
{
    fprintf(
        f, "{\"situs\": %.2f, \"angulus\": %.2f, \"lumen\": %.1f}",
        a->situs, a->angulus, a->lumen
    );
}
