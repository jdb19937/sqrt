/*
 * torus_planus.c — imago unica tori Hevea in PPM
 *
 * Immersio isometrica C1 tori plani quadrati in R3
 * secundum methodum Nash-Kuiper corrugationis.
 *
 * Fundum: campus stellarum toroidalis (parametri terrae).
 * Observator intus universum toroidale T² sedet,
 * immersionem concretam eiusdem topologiae spectans.
 * Vide commentarium in helvea.h:fundum_implere.
 */

#include "helvea.h"
#include "instrumentum.h"
#include "sidus.h"
#include "campus.h"

#include <stdio.h>
#include <stdlib.h>

#define LATITUDO_IMG  1920
#define ALTITUDO_IMG  1080
#define GRADUS_U      800
#define GRADUS_V      400

int main(int argc, char **argv)
{
    const char *via_isonl = "caelae/terra.isonl";
    const char *via_instr = "instrumenta/oculus.ison";
    if (argc > 1)
        via_isonl = argv[1];
    if (argc > 2)
        via_instr = argv[2];

    fprintf(stderr, "=== TORUS PLANUS CORRUGATUS ===\n");
    fprintf(stderr, "Immersio isometrica C1 in R3\n\n");

    /* campum stellarum ex ISONL reddere */
    fprintf(stderr, "Campum stellarum reddens: %s + %s\n", via_isonl, via_instr);
    campus_t *campus = campus_ex_isonl_reddere(via_isonl, via_instr);
    if (!campus) {
        fprintf(stderr, "ERROR: campus stellarum reddere non possum!\n");
        return 1;
    }

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    tabula_t tab;
    tab.latitudo      = LATITUDO_IMG;
    tab.altitudo      = ALTITUDO_IMG;
    tab.bytes_pixel   = 3;
    tab.imaginis      = (unsigned char *)calloc(n_pix * 3, 1);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (!tab.imaginis || !tab.profunditatis) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    /* superficiem praecomputare */
    fprintf(
        stderr, "Superficiem computans (%d x %d)...\n",
        GRADUS_U, GRADUS_V
    );

    size_t n_vert  = (size_t)(GRADUS_U + 1) * (GRADUS_V + 1);
    vec3_t *puncta = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *normae = (vec3_t *)malloc(n_vert * sizeof(vec3_t));

    if (!puncta || !normae) {
        fprintf(stderr, "ERROR: memoria insufficiens pro superficie!\n");
        return 1;
    }

    helvea_superficiem_computare(
        puncta, normae, GRADUS_U, GRADUS_V,
        HELVEA_RADIUS_MAIOR, HELVEA_RADIUS_MINOR,
        HELVEA_BORRELLI
    );

    /* cameram constituere */
    vec3_t positio_camerae = vec3(2.6, -2.0, 1.5);
    vec3_t scopus          = vec3(0.0, 0.0, -0.05);
    camera_t cam = cameram_constituere(positio_camerae, scopus);

    /* fundum stellarum — translatio ex angulo camerae */
    double angulus_cam = atan2(positio_camerae.y, positio_camerae.x);
    int delta_x        = (int)(angulus_cam / DUO_PI * campus->latitudo);
    int delta_y        = (int)(positio_camerae.z / 4.0 * campus->altitudo);

    tabulam_purgare(&tab);
    fundum_implere(
        &tab, campus->pixels,
        campus->latitudo, campus->altitudo,
        delta_x, delta_y
    );

    /* scaenam reddere */
    fprintf(stderr, "Triangula reddens...\n");
    scaenam_reddere(
        &tab, puncta, normae, GRADUS_U, GRADUS_V,
        &cam, helvea_illuminare, pixel_rgb
    );

    /* PPM scribere */
    const char *via_ppm = "torus_planus.ppm";
    fprintf(stderr, "Imaginem scribens: %s\n", via_ppm);

    FILE *plica = fopen(via_ppm, "wb");
    if (!plica) {
        fprintf(stderr, "ERROR: fasciculum aperire non possum!\n");
        return 1;
    }
    fprintf(plica, "P6\n%d %d\n255\n", LATITUDO_IMG, ALTITUDO_IMG);
    fwrite(tab.imaginis, 1, n_pix * 3, plica);
    fclose(plica);

    campus_destruere(campus);
    free(tab.imaginis);
    free(tab.profunditatis);
    free(puncta);
    free(normae);

    fprintf(stderr, "\nOpus perfectum est.\n");
    return 0;
}
