/*
 * redde.c — campum stellarum ex ISONL in PPM reddit
 *
 * Legit ISONL stellarum (ex caele) et instrumentum ISON,
 * applicat effectus opticos, reddit PPM.
 *
 * Usus: ./redde <stellae.isonl> <instrumentum.ison> [imago.ppm]
 *
 * Si imago.ppm omittitur, scribit ad stdout.
 */

#include "astra.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usus: redde <stellae.isonl> <instrumentum.ison> [imago.ppm]\n");
        return 1;
    }

    fprintf(stderr, "Stellas reddens: %s + %s\n", argv[1], argv[2]);
    astra_campus_t *campus = astra_ex_isonl_reddere(argv[1], argv[2]);
    if (!campus) {
        fprintf(stderr, "ERROR: campus reddere non possum\n");
        return 1;
    }

    FILE *f;
    const char *via;
    if (argc >= 4) {
        via = argv[3];
        f = fopen(via, "wb");
        if (!f) {
            fprintf(stderr, "ERROR: %s aperire non possum\n", via);
            return 1;
        }
    } else {
        via = "stdout";
        f = stdout;
    }

    fprintf(stderr, "Imaginem scribens: %s\n", via);
    fprintf(f, "P6\n%d %d\n255\n", campus->latitudo, campus->altitudo);
    fwrite(campus->pixels, 1,
           (size_t)campus->latitudo * campus->altitudo * 3, f);

    if (f != stdout) fclose(f);

    astra_campum_destruere(campus);

    fprintf(stderr, "Opus perfectum est.\n");
    return 0;
}
