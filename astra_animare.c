/*
 * astra_animare.c — animatio campi stellarum
 *
 * Reddit campum stellarum staticum ex caela + instrumento,
 * deinde per tabulas effectus dynamicos applicat (scintillatio,
 * refractio atmosphaerica) et scribit MP4 et/vel GIF.
 *
 * Usus:
 *   ./astra_animare <caela.ison> <instrumentum.ison> [optiones]
 *
 * Optiones:
 *   -mp4 <via>        via plicae MP4
 *   -gif <via>        via plicae GIF
 *   -n <numerus>      numerus tabularum (defaltum 120)
 *   -fps <numerus>    tabulae per secundam (defaltum 30)
 *   -scala <numerus>  factor reductionis (defaltum 2)
 *   -wx <numerus>     revolutiones horizontales (loop perfectus, 0)
 *   -wy <numerus>     revolutiones verticales (loop perfectus, 0)
 *   -dx <valor>       translatio horiz pixeles/tabula (linearis, 0)
 *   -dy <valor>       translatio vert pixeles/tabula (linearis, 0)
 *   -gif_scala <n>    factor reductionis additus pro GIF (defaltum 2)
 *   -gif_mora <cs>    mora inter tabulas GIF centisecundis (defaltum 3)
 */

#include "instrumentum.h"
#include "tessera.h"
#include "campus.h"
#include "caela.h"
#include "ison.h"
#include "phantasma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RGB campus → ARGB8888 buffer (quod phantasma expectat) */
static void rgb_ad_argb(
    uint32_t *dest, const unsigned char *fons,
    int latitudo, int altitudo
) {
    int n = latitudo * altitudo;
    for (int i = 0; i < n; i++) {
        int si = i * 3;
        dest[i] = 0xFF000000u
            | ((uint32_t)fons[si + 0] << 16)
            | ((uint32_t)fons[si + 1] << 8)
            | ((uint32_t)fons[si + 2]);
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(
            stderr,
            "Usus: astra_animare <caela.ison> <instrumentum.ison> [optiones]\n"
            "  -mp4 <via>      plica MP4\n"
            "  -gif <via>      plica GIF\n"
            "  -n <num>        numerus tabularum (120)\n"
            "  -fps <num>      tabulae per secundam (30)\n"
            "  -scala <num>    factor reductionis (2)\n"
            "  -gif_scala <n>  reductionis GIF (2)\n"
            "  -gif_mora <cs>  mora GIF centisecundis (3)\n"
            "  -wx <num>       revolutiones horizontales (loop, 0)\n"
            "  -wy <num>       revolutiones verticales (loop, 0)\n"
            "  -dx <val>       translatio horiz pixeles/tabula (linearis)\n"
            "  -dy <val>       translatio vert pixeles/tabula (linearis)\n"
        );
        return 1;
    }

    const char *via_caela = argv[1];
    const char *via_instr = argv[2];
    const char *via_mp4 = NULL;
    const char *via_gif = NULL;
    int num_tab = 120;
    int fps = 30;
    int scala = 2;
    int gif_scala = 2;
    int gif_mora = 3;
    int wx = 0, wy = 0;          /* revolutiones toroidales (loop) */
    double dx_lin = 0, dy_lin = 0; /* translatio linearis px/tabula */
    int usa_dx = 0;

    for (int i = 3; i < argc - 1; i++) {
        if (strcmp(argv[i], "-mp4") == 0)
            via_mp4 = argv[++i];
        else if (strcmp(argv[i], "-gif") == 0)
            via_gif = argv[++i];
        else if (strcmp(argv[i], "-n") == 0)
            num_tab = atoi(argv[++i]);
        else if (strcmp(argv[i], "-fps") == 0)
            fps = atoi(argv[++i]);
        else if (strcmp(argv[i], "-scala") == 0)
            scala = atoi(argv[++i]);
        else if (strcmp(argv[i], "-gif_scala") == 0)
            gif_scala = atoi(argv[++i]);
        else if (strcmp(argv[i], "-gif_mora") == 0)
            gif_mora = atoi(argv[++i]);
        else if (strcmp(argv[i], "-wx") == 0)
            wx = atoi(argv[++i]);
        else if (strcmp(argv[i], "-wy") == 0)
            wy = atoi(argv[++i]);
        else if (strcmp(argv[i], "-dx") == 0) {
            dx_lin = atof(argv[++i]);
            usa_dx = 1;
        }else if (strcmp(argv[i], "-dy") == 0) {
            dy_lin = atof(argv[++i]);
            usa_dx = 1;
        }
    }

    if (!via_mp4 && !via_gif) {
        fprintf(stderr, "ERROR: nec -mp4 nec -gif specificatum\n");
        return 1;
    }

    /* campum stellarum staticum reddere */
    fprintf(stderr, "Campum reddens: %s + %s\n", via_caela, via_instr);
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
    /* instrumentum legere */
    char *instr_ison = ison_lege_plicam(via_instr);
    if (!instr_ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", via_instr);
        return 1;
    }
    instrumentum_t inst;
    instrumentum_ex_ison(&inst, instr_ison);
    free(instr_ison);

    campus_t *basis = campus_ex_caela(caela, &inst);
    caela_destruere(caela);
    if (!basis) {
        fprintf(stderr, "ERROR: campus reddere non possum\n");
        return 1;
    }

    int oL = basis->latitudo / scala;
    int oA = basis->altitudo / scala;
    fprintf(
        stderr, "Animans: %d tabulas, %dx%d (scala %d), %.1f fps\n",
        num_tab, oL, oA, scala, (double)fps
    );

    /* inscriptores aperire */
    pfr_mp4_t *mp4 = NULL;
    pfr_gif_t *gif = NULL;
    if (via_mp4) {
        mp4 = pfr_mp4_initia(via_mp4, oL, oA, fps);
        if (!mp4)
            fprintf(stderr, "MONITUM: MP4 aperire non possum\n");
    }
    if (via_gif) {
        gif = pfr_gif_initia(via_gif, oL, oA, gif_mora, gif_scala);
        if (!gif)
            fprintf(stderr, "MONITUM: GIF aperire non possum\n");
    }

    uint32_t *argb = (uint32_t *)malloc((size_t)oL * oA * sizeof(uint32_t));

    for (int f = 0; f < num_tab; f++) {
        if (f % 10 == 0)
            fprintf(stderr, "  tabula %d/%d\r", f + 1, num_tab);

        /* translatio: wx/wy = revolutiones exactae (loop perfectus),
         * dx/dy = pixeles lineares per tabulam (sine loop) */
        double tx, ty;
        if (usa_dx) {
            tx = dx_lin * f;
            ty = dy_lin * f;
        } else {
            tx = (double)wx * basis->latitudo * f / num_tab;
            ty = (double)wy * basis->altitudo * f / num_tab;
        }

        campus_t *tab = campus_tabulam_dynamicam(
            basis, &inst, f, scala, tx, ty
        );

        rgb_ad_argb(argb, tab->pixels, oL, oA);

        if (mp4)
            pfr_mp4_tabulam_adde(mp4, argb);
        if (gif)
            pfr_gif_tabulam_adde(gif, argb);

        campus_destruere(tab);
    }

    fprintf(stderr, "\n");

    if (mp4) {
        pfr_mp4_fini(mp4);
        fprintf(stderr, "MP4: %s\n", via_mp4);
    }
    if (gif) {
        pfr_gif_fini(gif);
        fprintf(stderr, "GIF: %s\n", via_gif);
    }

    free(argb);
    campus_destruere(basis);

    fprintf(stderr, "Animatio perfecta est.\n");
    return 0;
}
