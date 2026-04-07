/*
 * proba_planeta.c — reddit omnes planetas in PPM
 *
 * Legit omnes .ison plicas ex planetae/ directorio,
 * reddit unumquemque in 512×512 imaginem.
 */

#include "tessera.h"
#include "perceptus.h"
#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static void scribe_ppm(const char *via, const unsigned char *rgba, int lat, int alt)
{
    FILE *f = fopen(via, "wb");
    if (!f) {
        fprintf(stderr, "ERROR: %s aperire non possum\n", via);
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", lat, alt);
    for (int i = 0; i < lat * alt; i++) {
        unsigned char r = rgba[i * 4 + 0];
        unsigned char g = rgba[i * 4 + 1];
        unsigned char b = rgba[i * 4 + 2];
        unsigned char a = rgba[i * 4 + 3];
        /* alpha super fundum nigrum */
        unsigned char br = (unsigned char)(r * a / 255);
        unsigned char bg = (unsigned char)(g * a / 255);
        unsigned char bb = (unsigned char)(b * a / 255);
        fputc(br, f);
        fputc(bg, f);
        fputc(bb, f);
    }
    fclose(f);
}

int main(void)
{
    unsigned char *fen = (unsigned char *)calloc(PLANETA_FENESTRA * PLANETA_FENESTRA * 4, 1);
    if (!fen) {
        fprintf(stderr, "ERROR: memoria\n");
        return 1;
    }

    DIR *dir = opendir("planetae");
    if (!dir) {
        fprintf(stderr, "ERROR: planetae/ aperire non possum\n");
        return 1;
    }

    struct dirent *ent;
    int n = 0;

    while ((ent = readdir(dir)) != NULL) {
        const char *nomen = ent->d_name;
        size_t len        = strlen(nomen);
        if (len < 6 || strcmp(nomen + len - 5, ".ison") != 0)
            continue;

        char via_ison[256];
        snprintf(via_ison, sizeof(via_ison), "planetae/%s", nomen);

        char *ison = ison_lege_plicam(via_ison);
        if (!ison) {
            fprintf(stderr, "  MALUM: %s legere non possum\n", via_ison);
            continue;
        }

        char *per_raw = ison_da_crudum(ison, "perceptus");
        planeta_t p   = planeta_ex_ison(ison);
        free(ison);

        planeta_perceptus_t perc = planeta_perceptus_ex_ison(per_raw);
        free(per_raw);
        memset(fen, 0, PLANETA_FENESTRA * PLANETA_FENESTRA * 4);
        planeta_reddere(fen, &p, &perc);
        planeta_perceptum_applicare(fen, &perc);

        /* nomen sine extensione */
        char basis[128];
        strncpy(basis, nomen, len - 5);
        basis[len - 5] = '\0';

        char via_ppm[256];
        snprintf(via_ppm, sizeof(via_ppm), "probae/planetae/%s.ppm", basis);

        scribe_ppm(via_ppm, fen, PLANETA_FENESTRA, PLANETA_FENESTRA);
        fprintf(stderr, "  %-20s → %s\n", nomen, via_ppm);
        n++;
    }
    closedir(dir);

    fprintf(stderr, "\n%d planetae redditae.\n", n);
    free(fen);
    return 0;
}
