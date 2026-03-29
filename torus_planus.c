/*
 * torus_planus.c — imago unica tori Hevea in PPM
 *
 * Immersio isometrica C1 tori plani quadrati in R3
 * secundum methodum Nash-Kuiper corrugationis.
 */

#include "helvea.h"

#include <stdio.h>
#include <stdlib.h>

#define LATITUDO_IMG  1920
#define ALTITUDO_IMG  1080
#define GRADUS_U      800
#define GRADUS_V      400

int main(void)
{
    fprintf(stderr, "=== TORUS PLANUS CORRUGATUS ===\n");
    fprintf(stderr, "Immersio isometrica C1 in R3\n\n");

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    helvea_tabula_t tab;
    tab.latitudo = LATITUDO_IMG;
    tab.altitudo = ALTITUDO_IMG;
    tab.bytes_pixel = 3;
    tab.imaginis = (unsigned char *)calloc(n_pix * 3, 1);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (!tab.imaginis || !tab.profunditatis) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    /* superficiem praecomputare */
    fprintf(stderr, "Superficiem computans (%d x %d)...\n",
            GRADUS_U, GRADUS_V);

    size_t n_vert = (size_t)(GRADUS_U + 1) * (GRADUS_V + 1);
    Vec3 *puncta = (Vec3 *)malloc(n_vert * sizeof(Vec3));
    Vec3 *normae = (Vec3 *)malloc(n_vert * sizeof(Vec3));

    if (!puncta || !normae) {
        fprintf(stderr, "ERROR: memoria insufficiens pro superficie!\n");
        return 1;
    }

    helvea_superficiem_computare(puncta, normae, GRADUS_U, GRADUS_V,
                                 HELVEA_RADIUS_MAIOR, HELVEA_RADIUS_MINOR,
                                 HELVEA_CORRUGATA);

    /* cameram constituere */
    Vec3 positio_camerae = vec3(2.6, -2.0, 1.5);
    Vec3 scopus          = vec3(0.0, 0.0, -0.05);
    Camera cam = helvea_cameram_constituere(positio_camerae, scopus);

    /* scaenam reddere */
    fprintf(stderr, "Triangula reddens...\n");
    helvea_scaenam_reddere(&tab, puncta, normae, GRADUS_U, GRADUS_V,
                           &cam, helvea_illuminare, helvea_pixel_rgb);

    /* PPM scribere */
    const char *via_ppm = "torus_planus.ppm";
    fprintf(stderr, "Imaginem scribens: %s\n", via_ppm);

    FILE *fasciculus = fopen(via_ppm, "wb");
    if (!fasciculus) {
        fprintf(stderr, "ERROR: fasciculum aperire non possum!\n");
        return 1;
    }
    fprintf(fasciculus, "P6\n%d %d\n255\n", LATITUDO_IMG, ALTITUDO_IMG);
    fwrite(tab.imaginis, 1, n_pix * 3, fasciculus);
    fclose(fasciculus);

    free(tab.imaginis);
    free(tab.profunditatis);
    free(puncta);
    free(normae);

    fprintf(stderr, "\nOpus perfectum est.\n");
    return 0;
}
