#include "../planeta_communia.h"
#include <phantasma/phantasma.h>
/* simulacrum.c — renderer planetae simulacri (imago GIF) */

void reddere_simulacrum(unsigned char *fen, const simulacrum_t *p)
{
    /* viam GIF construe */
    char via[256];
    snprintf(via, sizeof(via), "gifs/%s.gif", p->res.nomen);

    pfr_gif_lector_t *lector = pfr_gif_lege_initia(via);
    if (!lector)
        return;

    int lat = 0, alt = 0;
    if (pfr_gif_lege_dimensiones(lector, &lat, &alt) != 0) {
        pfr_gif_lege_fini(lector);
        return;
    }

    /* primam tabulam lege in ARGB8888 */
    uint32_t *argb = (uint32_t *)calloc((size_t)lat * alt, sizeof(uint32_t));
    if (!argb) {
        pfr_gif_lege_fini(lector);
        return;
    }

    if (pfr_gif_lege_tabulam(lector, argb) != 0) {
        free(argb);
        pfr_gif_lege_fini(lector);
        return;
    }
    pfr_gif_lege_fini(lector);

    /* scala ad fenestram 512×512 — nearest neighbor */
    for (int py = 0; py < FEN; py++) {
        int sy = py * alt / FEN;
        if (sy >= alt) sy = alt - 1;
        for (int px = 0; px < FEN; px++) {
            int sx = px * lat / FEN;
            if (sx >= lat) sx = lat - 1;

            uint32_t pixel = argb[sy * lat + sx];
            /* ARGB8888: A[31:24] R[23:16] G[15:8] B[7:0] */
            unsigned char a = (pixel >> 24) & 0xFF;
            unsigned char r = (pixel >> 16) & 0xFF;
            unsigned char g = (pixel >>  8) & 0xFF;
            unsigned char b = (pixel      ) & 0xFF;

            int idx = (py * FEN + px) * 4;
            fen[idx + 0] = r;
            fen[idx + 1] = g;
            fen[idx + 2] = b;
            fen[idx + 3] = a;
        }
    }

    free(argb);
}

void simulacrum_ex_ison(simulacrum_t *v, const char *ison)
{
    v->pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);

    char *nomen = ison_da_chordam(ison, "simulacrulum.nomen");
    if (nomen) {
        strncpy(v->res.nomen, nomen, SIMULACRUM_NOMEN_MAX - 1);
        v->res.nomen[SIMULACRUM_NOMEN_MAX - 1] = '\0';
        free(nomen);
    } else {
        strcpy(v->res.nomen, "vortexa");
    }
}

void simulacrum_in_ison(FILE *f, const simulacrum_t *s)
{
    fprintf(f, "{\"planetella\": {\"radius\": %.2f, \"inclinatio\": %.3f, \"rotatio\": %.1f, \"semen\": %u}",
        s->pro.radius, s->pro.inclinatio, s->pro.rotatio, s->pro.semen);
    fprintf(f, ", \"simulacrulum\": {\"nomen\": \"%s\"}}", s->res.nomen);
}
