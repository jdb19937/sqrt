/*
 * gen_universalis.c — universalem ex designatione generare
 *
 * Legit designationem ex plica ISON,
 * generat universalem, emittit ISON ad stdout.
 *
 * Usus: ./gen_universalis designationes/basic.ison > universalia/basic.ison
 */

#include "designatio.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usus: gen_universalis <designatio.ison>\n");
        return 1;
    }

    char *ison = ison_lege_plicam(argv[1]);
    if (!ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[1]);
        return 1;
    }

    designatio_t des;
    designatio_ex_ison(&des, ison);
    free(ison);

    universalis_t *u = universalis_ex_designatione(&des);
    if (!u) {
        fprintf(stderr, "ERROR: universalem generare non possum\n");
        return 1;
    }

    char *output = universalis_in_ison(u);
    universalis_destruere(u);
    if (!output) {
        fprintf(stderr, "ERROR: ISON scribere non possum\n");
        return 1;
    }

    fputs(output, stdout);
    free(output);
    return 0;
}
