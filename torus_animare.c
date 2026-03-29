/*
 * torus_animare.c — animatio tori Hevea, imagines PPM scribens
 *
 * Camera circa torum rotans; fundum stellarum toroidaliter volvitur.
 * Rotatio camerae perioditatem cosmicam spatii T² demonstrat:
 * post revolutionem plenam, eaedem stellae redeunt.
 * Usus: ./torus_animare [numerus_imaginum]
 */

#include "helvea.h"
#include "astra.h"

#include <stdio.h>
#include <stdlib.h>

#define LATITUDO_IMG  640
#define ALTITUDO_IMG  480
#define GRADUS_U      600
#define GRADUS_V      300

int main(int argc, char **argv)
{
    const char *via_isonl = "caelae/terra.isonl";
    const char *via_instr = "instrumenta/oculus.ison";
    int numerus_imaginum = 72;
    int argi = 1;
    if (argi < argc && argv[argi][0] != '-') {
        numerus_imaginum = atoi(argv[argi++]);
    }
    if (argi < argc) via_isonl = argv[argi++];
    if (argi < argc) via_instr = argv[argi++];
    if (numerus_imaginum < 1) numerus_imaginum = 72;

    /* campum stellarum ex ISONL reddere */
    fprintf(stderr, "Campum stellarum reddens: %s + %s\n", via_isonl, via_instr);
    astra_campus_t *campus = astra_ex_isonl_reddere(via_isonl, via_instr);
    if (!campus) {
        fprintf(stderr, "ERROR: campus stellarum reddere non possum!\n");
        return 1;
    }

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    helvea_tabula_t tab;
    tab.latitudo = LATITUDO_IMG;
    tab.altitudo = ALTITUDO_IMG;
    tab.bytes_pixel = 3;
    tab.imaginis = (unsigned char *)malloc(n_pix * 3);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (!tab.imaginis || !tab.profunditatis) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    /* superficiem praecomputare */
    fprintf(stderr, "Superficiem computans...\n");
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

    fprintf(stderr, "Animationem reddens: %d imagines\n", numerus_imaginum);

    for (int f = 0; f < numerus_imaginum; f++) {
        fprintf(stderr, "Imago %d/%d\n", f + 1, numerus_imaginum);

        double angulus = DUO_PI * (double)f / (double)numerus_imaginum;
        double dist_cam = 3.2;
        double alt_cam  = 1.3;
        Vec3 pos_cam = vec3(dist_cam * cos(angulus),
                            dist_cam * sin(angulus),
                            alt_cam);
        Vec3 scopus = vec3(0.0, 0.0, -0.05);
        Camera cam = helvea_cameram_constituere(pos_cam, scopus);

        /* fundum stellarum — toroidaliter volvitur cum camera */
        int delta_x = (int)(angulus / DUO_PI * campus->latitudo);
        int delta_y = (int)(alt_cam / 4.0 * campus->altitudo);

        helvea_tabulam_purgare(&tab);
        helvea_fundum_implere(&tab, campus->pixels,
                              campus->latitudo, campus->altitudo,
                              delta_x, delta_y);

        helvea_scaenam_reddere(&tab, puncta, normae, GRADUS_U, GRADUS_V,
                               &cam, helvea_illuminare, helvea_pixel_rgb);

        char nomen[256];
        snprintf(nomen, sizeof(nomen), "tabulae/imago_%04d.ppm", f);
        FILE *fas = fopen(nomen, "wb");
        if (!fas) {
            fprintf(stderr, "ERROR: %s aperire non possum!\n", nomen);
            return 1;
        }
        fprintf(fas, "P6\n%d %d\n255\n", LATITUDO_IMG, ALTITUDO_IMG);
        fwrite(tab.imaginis, 1, n_pix * 3, fas);
        fclose(fas);
    }

    astra_campum_destruere(campus);
    free(tab.imaginis);
    free(tab.profunditatis);
    free(puncta);
    free(normae);

    fprintf(stderr, "Animatio perfecta est.\n");
    return 0;
}
