/*
 * universalis.c — universalis: lector, scriptor
 */

#include "universalis.h"
#include <ison/ison.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * destructor
 * ================================================================ */

static void libera_tabulam(sidus_universalis_t *tab, int n)
{
    if (!tab)
        return;
    for (int i = 0; i < n; i++)
        free(tab[i].nomen);
    free(tab);
}

void universalis_destruere(universalis_t *u)
{
    if (!u)
        return;
    libera_tabulam(u->stellae, u->numerus_stellarum);
    libera_tabulam(u->galaxiae, u->numerus_galaxiarum);
    free(u);
}

/* ================================================================
 * universalis_ex_ison — legit universalem ex chorda ISON
 * ================================================================ */

typedef struct {
    universalis_t *u;
    int galaxia;  /* 1 = galaxiae, 0 = stellae */
} lector_ctx_t;

static void lege_sidus(const char *raw, void *ctx_v)
{
    lector_ctx_t *ctx = (lector_ctx_t *)ctx_v;
    universalis_t *u  = ctx->u;

    sidus_universalis_t su;
    su.u     = (int)ison_da_n(raw, "u", 0);
    su.v     = (int)ison_da_n(raw, "v", 0);
    su.nomen = ison_da_chordam(raw, "nomen");

    char *sidus_raw = ison_da_crudum(raw, "sidus");
    if (sidus_raw) {
        sidus_ex_ison(&su.sidus, sidus_raw);
        free(sidus_raw);
    } else {
        memset(&su.sidus, 0, sizeof(sidus_t));
        su.sidus.qui = ctx->galaxia ? SIDUS_GALAXIA : SIDUS_SEQUENTIA;
        su.sidus.ubi.sequentia.pro.magnitudo   = 6.0;
        su.sidus.ubi.sequentia.pro.temperatura = 5000;
    }

    if (ctx->galaxia) {
        int n       = u->numerus_galaxiarum;
        u->galaxiae = realloc(u->galaxiae, (size_t)(n + 1) * sizeof(sidus_universalis_t));
        if (!u->galaxiae)
            return;
        u->galaxiae[n]        = su;
        u->numerus_galaxiarum = n + 1;
    } else {
        int n      = u->numerus_stellarum;
        u->stellae = realloc(u->stellae, (size_t)(n + 1) * sizeof(sidus_universalis_t));
        if (!u->stellae)
            return;
        u->stellae[n]        = su;
        u->numerus_stellarum = n + 1;
    }
}

universalis_t *universalis_ex_ison(const char *ison)
{
    universalis_t *u = calloc(1, sizeof(universalis_t));
    if (!u)
        return NULL;

    u->latitudo = (int)ison_da_n(ison, "latitudo", 4096);
    u->altitudo = (int)ison_da_n(ison, "altitudo", 4096);

    lector_ctx_t ctx_s = { u, 0 };
    ison_pro_quoque_elemento(ison, "stellae", lege_sidus, &ctx_s);

    lector_ctx_t ctx_g = { u, 1 };
    ison_pro_quoque_elemento(ison, "galaxiae", lege_sidus, &ctx_g);

    return u;
}

/* ================================================================
 * universalis_in_ison — scribit universalem in chordam ISON
 * ================================================================ */

static void scribe_tabulam(
    FILE *f, const char *nomen,
    const sidus_universalis_t *tab, int n,
    int ultima
) {
    fprintf(f, "  \"%s\": [\n", nomen);
    for (int i = 0; i < n; i++) {
        const sidus_universalis_t *su = &tab[i];
        if (i > 0)
            fprintf(f, ",\n");
        fprintf(f, "    {\"u\": %d, \"v\": %d", su->u, su->v);
        if (su->nomen)
            fprintf(f, ", \"nomen\": \"%s\"", su->nomen);
        fprintf(f, ", \"sidus\": ");
        sidus_in_ison(f, &su->sidus);
        fprintf(f, "}");
    }
    fprintf(f, "\n  ]%s\n", ultima ? "" : ",");
}

char *universalis_in_ison(const universalis_t *u)
{
    char *result = NULL;
    size_t sz    = 0;
    FILE *f      = open_memstream(&result, &sz);
    if (!f)
        return NULL;

    fprintf(f, "{\n");
    fprintf(f, "  \"latitudo\": %d,\n", u->latitudo);
    fprintf(f, "  \"altitudo\": %d,\n", u->altitudo);

    scribe_tabulam(f, "stellae", u->stellae, u->numerus_stellarum, 0);
    scribe_tabulam(f, "galaxiae", u->galaxiae, u->numerus_galaxiarum, 1);

    fprintf(f, "}\n");

    fclose(f);
    return result;
}

/* ================================================================
 * universalis_propinqua — stellae propinquissimae
 * ================================================================ */

int universalis_propinqua(
    const universalis_t *u, int pu, int pv,
    double d, int n,
    sidus_propinquum_t *res
) {
    int num = 0;

    for (int i = 0; i < u->numerus_stellarum; i++) {
        double dx = abs(u->stellae[i].u - pu);
        double dy = abs(u->stellae[i].v - pv);
        if (dx > u->latitudo * 0.5)
            dx = u->latitudo - dx;
        if (dy > u->altitudo * 0.5)
            dy = u->altitudo - dy;
        double dist = sqrt(dx * dx + dy * dy);

        if (dist > d)
            continue;

        /* insertio ordinata — locus ubi dist < existens */
        int pos = num;
        while (pos > 0 && res[pos - 1].distantia > dist)
            pos--;

        if (num < n) {
            for (int j = num; j > pos; j--)
                res[j] = res[j - 1];
            res[pos].index     = i;
            res[pos].distantia = dist;
            num++;
        } else if (pos < n) {
            for (int j = n - 1; j > pos; j--)
                res[j] = res[j - 1];
            res[pos].index     = i;
            res[pos].distantia = dist;
        }
    }

    return num;
}
