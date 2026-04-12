/*
 * genera.c — formulam proceduraliter generare
 *
 * Ex semine (et optionaliter sidere) systema planetarium
 * integrum generat. Reddit ISON formulae ad stdout.
 *
 * Usus:
 *   ./genera <semen>
 */

#include "formula.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usus: genera <semen>\n");
        return 1;
    }

    unsigned int semen = (unsigned int)strtoul(argv[1], NULL, 10);

    formula_t f;
    memset(&f, 0, sizeof(f));
    formula_generare(&f, semen);

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
