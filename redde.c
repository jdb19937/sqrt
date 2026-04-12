/*
 * redde.c — campum stellarum ex caela in PPM reddit
 *
 * Legit caelam stellarum (.ison) et instrumentum ISON,
 * applicat effectus opticos, reddit PPM.
 *
 * Usus: ./redde <caela.ison> <instrumentum.ison> [imago.ppm]
 *
 * Si imago.ppm omittitur, scribit ad stdout.
 */

#include "instrumentum.h"
#include "tessera.h"
#include "campus.h"
#include "caela.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usus: redde <caela.ison> <instrumentum.ison> [imago.ppm]\n");
        return 1;
    }

    fprintf(stderr, "Stellas reddens: %s + %s\n", argv[1], argv[2]);
    char *caela_ison = ison_lege_plicam(argv[1]);
    if (!caela_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[1]);
        return 1;
    }
    caela_t *caela = caela_ex_ison(caela_ison);
    free(caela_ison);
    if (!caela) {
        fprintf(stderr, "ERROR: caelam legere non possum\n");
        return 1;
    }
    char *instr_ison = ison_lege_plicam(argv[2]);
    if (!instr_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[2]);
        return 1;
    }
    instrumentum_t inst;
    instrumentum_ex_ison(&inst, instr_ison);
    free(instr_ison);
    campus_t *campus = campus_ex_caela(caela, &inst);
    caela_destruere(caela);
    if (!campus) {
        fprintf(stderr, "ERROR: campus reddere non possum\n");
        return 1;
    }

    FILE *f;
    const char *via;
    if (argc >= 4) {
        via = argv[3];
        f   = fopen(via, "wb");
        if (!f) {
            fprintf(stderr, "ERROR: %s aperire non possum\n", via);
            return 1;
        }
    } else {
        via = "stdout";
        f   = stdout;
    }

    fprintf(stderr, "Imaginem scribens: %s\n", via);
    fprintf(f, "P6\n%d %d\n255\n", campus->latitudo, campus->altitudo);
    fwrite(
        campus->pixels, 1,
        (size_t)campus->latitudo * campus->altitudo * 3, f
    );

    if (f != stdout)
        fclose(f);

    campus_destruere(campus);

    fprintf(stderr, "Opus perfectum est.\n");
    return 0;
}
