/*
 * genera.c — formulam proceduraliter generare
 *
 * Ex semine systema planetarium integrum generat.
 * Reddit ISON formulae ad stdout.
 *
 * Usus:
 *   ./genera <temperatura> <semen>
 */

#include "formula.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usus: genera <temperatura> <semen>\n");
        return 1;
    }

    double temp        = atof(argv[1]);
    unsigned int semen = (unsigned int)strtoul(argv[2], NULL, 10);

    sidus_t sidus;
    memset(&sidus, 0, sizeof(sidus));
    sidus.qui = SIDUS_SEQUENTIA;
    sidus.ubi.sequentia.pro.temperatura = temp;
    sidus.ubi.sequentia.pro.magnitudo = 3.0;

    formula_t f;
    memset(&f, 0, sizeof(f));
    formula_generare(&f, semen, &sidus, NULL);

    char *result = NULL;
    size_t sz    = 0;
    FILE *out    = open_memstream(&result, &sz);
    if (!out) {
        fprintf(stderr, "ERROR: memoria\n");
        return 1;
    }
    formula_in_ison(out, &f);
    fclose(out);

    fputs(result, stdout);
    free(result);
    formula_purgare(&f);
    return 0;
}
