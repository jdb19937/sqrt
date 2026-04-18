/*
 * torus_animare.c — animatio tori Hevea, imagines PPM scribens
 *
 * camera_t circa torum rotans; fundum stellarum toroidaliter volvitur.
 * Rotatio camerae perioditatem cosmicam spatii T² demonstrat:
 * post revolutionem plenam, eaedem stellae redeunt.
 * Usus: ./torus_animare [numerus_imaginum]
 */

#include "helvea.h"
#include "instrumentum.h"
#include "tessera.h"
#include "campus.h"
#include "caela.h"
#include <ison/ison.h>

#include <stdio.h>
#include <stdlib.h>

#define LATITUDO_IMG  640
#define ALTITUDO_IMG  480
#define GRADUS_U      600
#define GRADUS_V      300

int main(int argc, char **argv)
{
    const char *via_caela = "caelae/terra.ison";
    const char *via_instr = "instrumenta/oculus.ison";
    int numerus_imaginum = 72;
    int argi = 1;
    if (argi < argc && argv[argi][0] != '-') {
        numerus_imaginum = atoi(argv[argi++]);
    }
    if (argi < argc)
        via_caela = argv[argi++];
    if (argi < argc)
        via_instr = argv[argi++];
    if (numerus_imaginum < 1)
        numerus_imaginum = 72;

    /* campum stellarum ex caela reddere */
    fprintf(stderr, "Campum stellarum reddens: %s + %s\n", via_caela, via_instr);
    char *caela_ison = ison_lege_plicam(via_caela);
    if (!caela_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", via_caela);
        return 1;
    }
    caela_t *caela = caela_ex_ison(caela_ison);
    free(caela_ison);
    if (!caela) {
        fprintf(stderr, "ERROR: caelam legere non possum\n");
        return 1;
    }
    char *instr_ison = ison_lege_plicam(via_instr);
    if (!instr_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", via_instr);
        return 1;
    }
    instrumentum_t inst;
    instrumentum_ex_ison(&inst, instr_ison);
    free(instr_ison);
    campus_t *campus = campus_ex_caela(caela, &inst);
    caela_destruere(caela);
    if (!campus) {
        fprintf(stderr, "ERROR: campus stellarum reddere non possum!\n");
        return 1;
    }

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    tabula_t tab;
    tab.latitudo      = LATITUDO_IMG;
    tab.altitudo      = ALTITUDO_IMG;
    tab.bytes_pixel   = 3;
    tab.imaginis      = (unsigned char *)malloc(n_pix * 3);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (!tab.imaginis || !tab.profunditatis) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    /* superficiem praecomputare */
    fprintf(stderr, "Superficiem computans...\n");
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

    fprintf(stderr, "Animationem reddens: %d imagines\n", numerus_imaginum);

    for (int f = 0; f < numerus_imaginum; f++) {
        fprintf(stderr, "Imago %d/%d\n", f + 1, numerus_imaginum);

        double angulus  = DUO_PI * (double)f / (double)numerus_imaginum;
        double dist_cam = 3.2;
        double alt_cam  = 1.3;
        vec3_t pos_cam = vec3(
            dist_cam * cos(angulus),
            dist_cam * sin(angulus),
            alt_cam
        );
        vec3_t scopus = vec3(0.0, 0.0, -0.05);
        camera_t cam  = cameram_constituere(pos_cam, scopus);

        /* fundum stellarum — toroidaliter volvitur cum camera */
        int delta_x = (int)(angulus / DUO_PI * campus->latitudo);
        int delta_y = (int)(alt_cam / 4.0 * campus->altitudo);

        tabulam_purgare(&tab);
        fundum_implere(
            &tab, campus->pixels,
            campus->latitudo, campus->altitudo,
            delta_x, delta_y
        );

        scaenam_reddere(
            &tab, puncta, normae, GRADUS_U, GRADUS_V,
            &cam, helvea_illuminare, pixel_rgb
        );

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

    campus_destruere(campus);
    free(tab.imaginis);
    free(tab.profunditatis);
    free(puncta);
    free(normae);

    fprintf(stderr, "Animatio perfecta est.\n");
    return 0;
}
