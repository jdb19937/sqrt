/*
 * proba_caelae.c — probatio pipeline: formula → caela → campus → ppm
 *
 * Pro quaque plica .ison in formulae/ directorio:
 *   1. generat caelam via caela_ex_formula() ad passum temporis magnum
 *   2. applicat orbitas et illuminationem via caela_orbitas_applicare()
 *   3. reddit campum via campus_ex_caela()
 *   4. scribit PPM in probae/caelae/
 *   5. numerat pixels non nigros
 *
 * Usus: ./proba_caelae [instrumentum.ison]
 *   (praefinitum: instrumenta/jwst.ison)
 */

#include "instrumentum.h"
#include "tessera.h"
#include "campus.h"
#include "formula.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define PASSUS_TEMPORIS 150

static void scribe_ppm(
    const char *via, const unsigned char *rgb,
    int latitudo, int altitudo
) {
    FILE *f = fopen(via, "wb");
    if (!f) {
        fprintf(stderr, "ERROR: %s aperire non possum\n", via);
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", latitudo, altitudo);
    fwrite(rgb, 1, (size_t)latitudo * altitudo * 3, f);
    fclose(f);
}

static int pixels_activi(const unsigned char *rgb, int latitudo, int altitudo)
{
    int n   = 0;
    int tot = latitudo * altitudo * 3;
    for (int i = 0; i < tot; i++)
        if (rgb[i] > 0) {
        n++;
        i += 2;
    }/* per pixelem, non per canalem */
    return n / 1;
}

int main(int argc, char **argv)
{
    const char *via_instr = (argc >= 2) ? argv[1] : "instrumenta/jwst.ison";

    fprintf(stderr, "=== CAELAE PROBA ===\n");
    fprintf(stderr, "Instrumentum: %s, t=%d\n\n", via_instr, PASSUS_TEMPORIS);

    /* probae/caelae/ creare si non exstat */
#if defined(_WIN32)
    _mkdir("probae/caelae");
#else
    {
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "mkdir -p probae/caelae");
        (void)system(cmd);
    }
#endif

    DIR *dir = opendir("formulae");
    if (!dir) {
        fprintf(stderr, "ERROR: formulae/ aperire non possum\n");
        return 1;
    }

    char *instr_ison = ison_lege_plicam(via_instr);
    if (!instr_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", via_instr);
        closedir(dir);
        return 1;
    }
    instrumentum_t inst;
    instrumentum_ex_ison(&inst, instr_ison);
    free(instr_ison);

    struct dirent *ent;
    int n_bona = 0, n_mala = 0;

    while ((ent = readdir(dir)) != NULL) {
        const char *nomen = ent->d_name;
        size_t len        = strlen(nomen);
        if (len < 6 || strcmp(nomen + len - 5, ".ison") != 0)
            continue;

        char via_formula[256];
        snprintf(via_formula, sizeof(via_formula), "formulae/%s", nomen);

        char *form_ison = ison_lege_plicam(via_formula);
        if (!form_ison) {
            fprintf(stderr, "  MALUM: %s legere non possum\n", via_formula);
            n_mala++;
            continue;
        }
        formula_t form;
        formula_ex_ison(&form, form_ison);
        free(form_ison);

        caela_t *caela = caela_ex_formula(&form, PASSUS_TEMPORIS);
        if (!caela) {
            fprintf(stderr, "  MALUM: %s caelam generare non possum\n", via_formula);
            formula_purgare(&form);
            n_mala++;
            continue;
        }
        caela_orbitas_applicare(caela, &form, PASSUS_TEMPORIS);
        formula_purgare(&form);

        campus_t *campus = campus_ex_caela(caela, &inst);
        caela_destruere(caela);
        if (!campus) {
            fprintf(stderr, "  MALUM: %s reddere non possum\n", via_formula);
            n_mala++;
            continue;
        }

        /* nomen sine .ison */
        char basis[128];
        size_t blen = len - 5;
        if (blen >= sizeof(basis))
            blen = sizeof(basis) - 1;
        strncpy(basis, nomen, blen);
        basis[blen] = '\0';

        char via_ppm[256];
        snprintf(via_ppm, sizeof(via_ppm), "probae/caelae/%s.ppm", basis);

        scribe_ppm(via_ppm, campus->pixels, campus->latitudo, campus->altitudo);

        int px = pixels_activi(campus->pixels, campus->latitudo, campus->altitudo);
        fprintf(
            stderr, "  %-20s %4dx%-4d  %7d px activi → %s\n",
            nomen, campus->latitudo, campus->altitudo, px, via_ppm
        );

        if (px > 0)
            n_bona++;
        else {
            fprintf(stderr, "    MALUM: nullus pixel\n");
            n_mala++;
        }

        campus_destruere(campus);
    }
    closedir(dir);

    fprintf(stderr, "\n%d caelae redditae", n_bona);
    if (n_mala)
        fprintf(stderr, ", %d errores", n_mala);
    fprintf(stderr, ".\n");
    return n_mala;
}
