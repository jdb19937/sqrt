/*
 * caele.c — caelam ex formula generare
 *
 * Legit formulam ex plica ISON (formulae/nomen.ison),
 * generat caelam, emittit ISON ad stdout.
 *
 * Usus: ./caele formulae/terra.ison > caelae/terra.ison
 */

#include "formula.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usus: caele <formula.ison>\n");
        return 1;
    }

    char *ison = ison_lege_plicam(argv[1]);
    if (!ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[1]);
        return 1;
    }

    formula_t form;
    formula_ex_ison(&form, ison);
    free(ison);

    caela_t *caela = caela_ex_formula(&form, 0);
    formula_purgare(&form);
    if (!caela) {
        fprintf(stderr, "ERROR: caelam generare non possum\n");
        return 1;
    }

    char *output = caela_in_ison(caela);
    caela_destruere(caela);
    if (!output) {
        fprintf(stderr, "ERROR: ISON scribere non possum\n");
        return 1;
    }

    fputs(output, stdout);
    free(output);
    return 0;
}
