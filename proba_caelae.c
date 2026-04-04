/*
 * proba_caelae.c — probatio pipeline: caele → isonl → reddit
 *
 * Pro quaque plica .isonl in caelae/ directorio:
 *   1. legit campum stellarum via campus_ex_isonl_reddere()
 *   2. scribit PPM in proba_caelae/
 *   3. numerat pixels non nigros
 *
 * Usus: ./proba_caelae [instrumentum.ison]
 *   (praefinitum: instrumenta/jwst.ison)
 */

#include "instrumentum.h"
#include "sidus.h"
#include "campus.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

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
    fprintf(stderr, "Instrumentum: %s\n\n", via_instr);

    /* proba_caelae/ creare si non exstat */
#if defined(_WIN32)
    _mkdir("proba_caelae");
#else
    {
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "mkdir -p probae/caelae");
        (void)system(cmd);
    }
#endif

    DIR *dir = opendir("caelae");
    if (!dir) {
        fprintf(stderr, "ERROR: caelae/ aperire non possum\n");
        return 1;
    }

    struct dirent *ent;
    int n_bona = 0, n_mala = 0;

    while ((ent = readdir(dir)) != NULL) {
        const char *nomen = ent->d_name;
        size_t len        = strlen(nomen);
        if (len < 7 || strcmp(nomen + len - 6, ".isonl") != 0)
            continue;

        char via_isonl[256];
        snprintf(via_isonl, sizeof(via_isonl), "caelae/%s", nomen);

        campus_t *campus = campus_ex_isonl_reddere(via_isonl, via_instr);
        if (!campus) {
            fprintf(stderr, "  MALUM: %s reddere non possum\n", via_isonl);
            n_mala++;
            continue;
        }

        /* nomen sine .isonl */
        char basis[128];
        size_t blen = len - 6;
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
