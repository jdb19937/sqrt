/*
 * universalis_gif.c — reddit universalem ut GIF 512x512
 *
 * Legit universalem ex ISON, reddit stellas ut puncta et
 * lineas ad 4 sidera propinquissima cuiusque sideris.
 *
 * Usus: ./universalis_gif <universalis.ison> <imago.gif>
 */

#include "universalis.h"
#include <ison/ison.h>
#include <phantasma/phantasma.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAT_IMG 512
#define ALT_IMG 512
#define PROPINQUI 4
#define DIST_MAX 400.0

static uint32_t imago[LAT_IMG * ALT_IMG];

static void pixel_pone(int x, int y, uint32_t col)
{
    if (x >= 0 && x < LAT_IMG && y >= 0 && y < ALT_IMG)
        imago[y * LAT_IMG + x] = col;
}

static void linea(int x0, int y0, int x1, int y1, uint32_t col)
{
    int dx  = abs(x1 - x0);
    int dy  = abs(y1 - y0);
    int sx  = x0 < x1 ? 1 : -1;
    int sy  = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    for (;;) {
        pixel_pone(x0, y0, col);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 <  dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/* coordinata toroidalis propinquissima in spatio imaginis */
static int toro_prox(int a, int b, int modulus)
{
    int d = b - a;
    if (d > modulus / 2)
        d -= modulus;
    if (d < -modulus / 2)
        d += modulus;
    return a + (int)((double)d * LAT_IMG / modulus);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usus: universalis_gif <universalis.ison> <imago.gif>\n");
        return 1;
    }

    char *ison = ison_lege_plicam(argv[1]);
    if (!ison) {
        fprintf(stderr, "ERROR: %s legere non possum\n", argv[1]);
        return 1;
    }

    universalis_t *u = universalis_ex_ison(ison);
    free(ison);
    if (!u) {
        fprintf(stderr, "ERROR: universalem legere non possum\n");
        return 1;
    }

    /* fons niger (ARGB: opacus niger) */
    for (int i = 0; i < LAT_IMG * ALT_IMG; i++)
        imago[i] = 0xFF000000u;

    uint32_t col_linea  = 0xFF1A1A3A;  /* purpureum obscurum (ARGB) */
    uint32_t col_stella = 0xFFCCCCFF;  /* album caeruleum (ARGB) */
    uint32_t col_clarum = 0xFFFFFFFF;  /* album purum */

    sidus_propinquum_t res[PROPINQUI];

    /* lineae primum (sub stellis) */
    for (int i = 0; i < u->numerus_stellarum; i++) {
        int su = u->stellae[i].u;
        int sv = u->stellae[i].v;
        int ix = (int)((double)su / u->latitudo * LAT_IMG);
        int iy = (int)((double)sv / u->altitudo * ALT_IMG);

        int num = universalis_propinqua(u, su, sv, DIST_MAX, PROPINQUI, res);
        for (int j = 0; j < num; j++) {
            if (res[j].distantia < 1.0)
                continue;
            int nu = u->stellae[res[j].index].u;
            int nv = u->stellae[res[j].index].v;
            int nx = toro_prox(ix, (int)((double)nu / u->latitudo * LAT_IMG), LAT_IMG);
            int ny = toro_prox(iy, (int)((double)nv / u->altitudo * ALT_IMG), ALT_IMG);
            linea(ix, iy, nx, ny, col_linea);
        }
    }

    /* stellae super lineis */
    for (int i = 0; i < u->numerus_stellarum; i++) {
        int ix     = (int)((double)u->stellae[i].u / u->latitudo * LAT_IMG);
        int iy     = (int)((double)u->stellae[i].v / u->altitudo * ALT_IMG);
        double mag = u->stellae[i].sidus.ubi.sequentia.pro.magnitudo;

        if (mag < 2.0) {
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    pixel_pone(ix + dx, iy + dy, col_clarum);
        } else if (mag < 4.0) {
            pixel_pone(ix, iy, col_clarum);
            pixel_pone(ix - 1, iy, col_stella);
            pixel_pone(ix + 1, iy, col_stella);
            pixel_pone(ix, iy - 1, col_stella);
            pixel_pone(ix, iy + 1, col_stella);
        } else {
            pixel_pone(ix, iy, col_stella);
        }
    }

    /* galaxiae — puncta solitaria */
    for (int i = 0; i < u->numerus_galaxiarum; i++) {
        int ix = (int)((double)u->galaxiae[i].u / u->latitudo * LAT_IMG);
        int iy = (int)((double)u->galaxiae[i].v / u->altitudo * ALT_IMG);
        pixel_pone(ix, iy, col_stella);
    }

    /* scribe GIF */
    pfr_gif_t *gif = pfr_gif_initia(argv[2], LAT_IMG, ALT_IMG, 0, 1);
    if (!gif) {
        fprintf(stderr, "ERROR: GIF creare non possum\n");
        universalis_destruere(u);
        return 1;
    }
    pfr_gif_tabulam_adde(gif, imago);
    pfr_gif_fini(gif);

    fprintf(
        stderr, "%d stellae, %d galaxiae, %s scriptum.\n",
        u->numerus_stellarum, u->numerus_galaxiarum, argv[2]
    );
    universalis_destruere(u);
    return 0;
}
