/*
 * formula.c — formula generationis caeli: lector et scriptor
 */

#include "formula.h"
#include "orbita.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * lector
 * ================================================================ */

static void lege_planetam(const char *raw, void *ctx_v)
{
    formula_t *f = (formula_t *)ctx_v;
    int n        = f->numerus_planetarum_formulae;
    f->planetae  = realloc(f->planetae, (size_t)(n + 1) * sizeof(planeta_formulae_t));
    if (!f->planetae)
        return;

    planeta_formulae_t *pf = &f->planetae[n];
    memset(pf, 0, sizeof(*pf));
    pf->nomen = ison_da_chordam(raw, "nomen");
    pf->scala = ison_da_f(raw, "scala", 1.0);

    char *pl_ison = ison_da_crudum(raw, "planeta");
    if (pl_ison) {
        planeta_t *tmp = planeta_ex_ison(pl_ison);
        free(pl_ison);
        if (tmp) {
            pf->planeta = *tmp;
            free(tmp);
        }
    }

    char *orb_ison = ison_da_crudum(raw, "orbita");
    if (orb_ison) {
        orbita_ex_ison(&pf->orbita, orb_ison);
        free(orb_ison);
    }

    f->numerus_planetarum_formulae = n + 1;
}

void formula_ex_ison(formula_t *f, const char *ison)
{
    memset(f, 0, sizeof(*f));
    f->latitudo            = (int)ison_da_n(ison, "latitudo", 1024);
    f->altitudo            = (int)ison_da_n(ison, "altitudo", 512);
    f->numerus_stellarum   = (int)ison_da_n(ison, "numerus_stellarum", 10000);
    f->densitas_galaxiae   = ison_da_f(ison, "densitas_galaxiae", 0.5);
    f->inclinatio_galaxiae = ison_da_f(ison, "inclinatio_galaxiae", 0.0);
    f->latitudo_galaxiae   = ison_da_f(ison, "latitudo_galaxiae", 0.2);
    f->semen               = (unsigned int)ison_da_n(ison, "semen", 1);
    f->galaxia_glow        = ison_da_f(ison, "galaxia_glow", 0.0);
    f->galaxia_rift        = ison_da_f(ison, "galaxia_rift", 0.0);
    f->galaxia_nebulae     = (int)ison_da_n(ison, "galaxia_nebulae", 0);
    f->max_supergigantes   = (int)ison_da_n(ison, "max_supergigantes", 0);
    f->max_gigantes        = (int)ison_da_n(ison, "max_gigantes", 0);
    f->max_exotica         = (int)ison_da_n(ison, "max_exotica", 0);
    f->numerus_planetarum  = (int)ison_da_n(ison, "numerus_planetarum", 0);
    f->planetae_temp_min   = ison_da_f(ison, "planetae_temp_min", 4000);
    f->planetae_temp_max   = ison_da_f(ison, "planetae_temp_max", 6000);
    f->numerus_galaxiarum  = (int)ison_da_n(ison, "numerus_galaxiarum", 0);
    f->max_galaxiae        = (int)ison_da_n(ison, "max_galaxiae", 0);

    ison_pro_quoque_elemento(ison, "planetae", lege_planetam, f);
}

/* ================================================================
 * scriptor
 * ================================================================ */

void formula_in_ison(FILE *f, const formula_t *form)
{
    fprintf(f, "{\n");
    fprintf(f, "  \"latitudo\": %d,\n", form->latitudo);
    fprintf(f, "  \"altitudo\": %d,\n", form->altitudo);
    fprintf(f, "  \"numerus_stellarum\": %d,\n", form->numerus_stellarum);
    fprintf(f, "  \"densitas_galaxiae\": %.2f,\n", form->densitas_galaxiae);
    fprintf(f, "  \"inclinatio_galaxiae\": %.2f,\n", form->inclinatio_galaxiae);
    fprintf(f, "  \"latitudo_galaxiae\": %.2f,\n", form->latitudo_galaxiae);
    fprintf(f, "  \"semen\": %u,\n", form->semen);
    fprintf(f, "  \"galaxia_glow\": %.1f,\n", form->galaxia_glow);
    fprintf(f, "  \"galaxia_rift\": %.1f,\n", form->galaxia_rift);
    fprintf(f, "  \"galaxia_nebulae\": %d,\n", form->galaxia_nebulae);
    fprintf(f, "  \"max_supergigantes\": %d,\n", form->max_supergigantes);
    fprintf(f, "  \"max_gigantes\": %d,\n", form->max_gigantes);
    fprintf(f, "  \"max_exotica\": %d,\n", form->max_exotica);
    fprintf(f, "  \"numerus_planetarum\": %d,\n", form->numerus_planetarum);
    fprintf(f, "  \"planetae_temp_min\": %.0f,\n", form->planetae_temp_min);
    fprintf(f, "  \"planetae_temp_max\": %.0f,\n", form->planetae_temp_max);
    fprintf(f, "  \"numerus_galaxiarum\": %d,\n", form->numerus_galaxiarum);
    fprintf(f, "  \"max_galaxiae\": %d,\n", form->max_galaxiae);

    fprintf(f, "  \"planetae\": [\n");
    for (int i = 0; i < form->numerus_planetarum_formulae; i++) {
        const planeta_formulae_t *pf = &form->planetae[i];
        if (i > 0)
            fprintf(f, ",\n");
        fprintf(f, "    {");
        if (pf->nomen)
            fprintf(f, "\"nomen\": \"%s\", ", pf->nomen);
        fprintf(f, "\"scala\": %.3f, ", pf->scala);
        fprintf(f, "\"planeta\": ");
        planeta_in_ison(f, &pf->planeta);
        fprintf(f, ", \"orbita\": ");
        orbita_in_ison(f, &pf->orbita);
        fprintf(f, "}");
    }
    fprintf(f, "\n  ]\n");
    fprintf(f, "}\n");
}

/* ================================================================
 * destructor
 * ================================================================ */

void formula_purgare(formula_t *f)
{
    if (f->planetae) {
        for (int i = 0; i < f->numerus_planetarum_formulae; i++)
            free(f->planetae[i].nomen);
        free(f->planetae);
        f->planetae = NULL;
    }
}
