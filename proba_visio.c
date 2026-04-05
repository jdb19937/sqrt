/*
 * proba_visio.c — reddit omnes visiones in PPM
 *
 * Legit omnes .ison plicas ex visiones/ directorio,
 * reddit unumquamque in 2048x2048 imaginem.
 */

#include "visio.h"
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
    unsigned char *fen = (unsigned char *)calloc(
        (size_t)VISIO_FENESTRA * VISIO_FENESTRA * 4, 1
    );
    if (!fen) {
        fprintf(stderr, "ERROR: memoria\n");
        return 1;
    }

    DIR *dir = opendir("visiones");
    if (!dir) {
        fprintf(stderr, "ERROR: visiones/ aperire non possum\n");
        free(fen);
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
        snprintf(via_ison, sizeof(via_ison), "visiones/%s", nomen);

        char *ison = ison_lege_plicam(via_ison);
        if (!ison) {
            fprintf(stderr, "  MALUM: %s legere non possum\n", via_ison);
            continue;
        }

        visio_t *v = visio_ex_ison(ison);
        free(ison);
        if (!v) {
            fprintf(stderr, "  MALUM: %s genus ignotum\n", via_ison);
            continue;
        }

        memset(fen, 0, (size_t)VISIO_FENESTRA * VISIO_FENESTRA * 4);
        fprintf(stderr, "  Reddens: %s...\n", nomen);
        visio_reddere(fen, v);
        free(v);

        /* nomen sine extensione */
        char basis[128];
        strncpy(basis, nomen, len - 5);
        basis[len - 5] = '\0';

        char via_ppm[256];
        snprintf(via_ppm, sizeof(via_ppm), "probae/visiones/%s.ppm", basis);

        scribe_ppm(via_ppm, fen, VISIO_FENESTRA, VISIO_FENESTRA);
        fprintf(stderr, "  %-20s -> %s\n", nomen, via_ppm);
        n++;
    }
    closedir(dir);

    fprintf(stderr, "\n%d visiones redditae.\n", n);
    free(fen);
    return 0;
}
