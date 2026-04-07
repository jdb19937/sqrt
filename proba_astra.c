/*
 * proba_astra.c — probatio bibliothecae astrorum
 *
 * Probat generationem campi et redditionem siderum in memoria.
 * Nullas plicas scribit.
 */

#include "instrumentum.h"
#include "tessera.h"
#include "campus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEN SIDUS_FENESTRA

/* numerum pixelorum non nigrorum in fenestra computare */
static int pixels_activos(const unsigned char *rgba, int n_pix)
{
    int c = 0;
    for (int i = 0; i < n_pix; i++)
        if (rgba[i * 4 + 3] > 0)
            c++;
    return c;
}

int main(void)
{
    int errores = 0;

    fprintf(stderr, "=== ASTRA PROBA ===\n\n");

    /* --- campum stellarum --- */
    fprintf(stderr, "Campum generans (256x128)...\n");
    campus_t *campus = campus_creare(256, 128);
    campus_parametri_t par = {
        .numerus_stellarum = 2000,
        .densitas_galaxiae = 0.7,
        .inclinatio_galaxiae = 0.3,
        .latitudo_galaxiae = 0.12,
        .semen = 42
    };
    instrumentum_t instr_campus = {
        .halo_radius = 3.0, .halo_vis = 0.08,
        .saturatio = 1.4
    };
    campus_generare(campus, &par, &instr_campus);

    /* probatio: aliqui pixels non nigri */
    int activi = 0;
    for (int i = 0; i < 256 * 128 * 3; i++)
        if (campus->pixels[i] > 0)
            activi++;

    if (activi > 0) {
        fprintf(stderr, "  campus: %d pixels activi — BENE\n", activi);
    } else {
        fprintf(stderr, "  campus: nullus pixel activus — MALUM\n");
        errores++;
    }
    campus_destruere(campus);

    /* --- singula genera siderum --- */
    fprintf(stderr, "\nSpecimina generum:\n");

    instrumentum_t instr_lucida = {
        .spiculae = 6, .spiculae_long = 12.0, .spiculae_ang = 0.15,
        .halo_radius = 5.0, .halo_vis = 0.2, .saturatio = 1.0
    };
    instrumentum_t instr_nulla = {.saturatio = 1.0};

    struct {
        const char *nomen;
        sidus_t sidus;
        instrumentum_t *instr;
    } specimina[] = {
        {"nanum_album",      {.p = {SIDUS_NANUM_ALBUM,  1.0, 25000}},   &instr_lucida},
        {"sequentia",        {.p = {SIDUS_SEQUENTIA,     0.5, 5800}},    &instr_lucida},
        {"gigas_rubrum",     {.p = {SIDUS_GIGAS_RUBRUM,  0.8, 3200}},    &instr_lucida},
        {"supergigas",       {.p = {SIDUS_SUPERGIGAS,    0.2, 3500}},    &instr_lucida},
        {"neutronium",       {.p = {SIDUS_NEUTRONIUM,    1.0, 500000}},  &instr_nulla},
        {"crystallinum",     {.p = {SIDUS_CRYSTALLINUM,  0.5, 8000}},    &instr_nulla},
        {"magnetar",         {.g.magnetar = {{SIDUS_MAGNETAR, 0.5, 5000000}, 0}},   &instr_nulla},
        {"magnetar_2",       {.g.magnetar = {{SIDUS_MAGNETAR, 1.2, 5000000}, 0}},   &instr_nulla},
        {"gal_elliptica",    {.g.galaxia = {{SIDUS_GALAXIA, 4.0, 8000},    GALAXIA_ELLIPTICA, 0.5}},          &instr_nulla},
        {"gal_spiralis",     {.g.galaxia = {{SIDUS_GALAXIA, 3.5, 7000},    GALAXIA_SPIRALIS, 1.2}},           &instr_nulla},
        {"gal_barrata",      {.g.galaxia = {{SIDUS_GALAXIA, 4.2, 5000},    GALAXIA_SPIRALIS_BARRATA, 2.8}},   &instr_nulla},
        {"gal_lenticularis", {.g.galaxia = {{SIDUS_GALAXIA, 4.5, 6000},    GALAXIA_LENTICULARIS, 0.8}},       &instr_nulla},
        {"gal_irregularis",  {.g.galaxia = {{SIDUS_GALAXIA, 4.0, 9000},    GALAXIA_IRREGULARIS, 3.5}},        &instr_nulla},
        {"gal_edge_on",      {.g.galaxia = {{SIDUS_GALAXIA, 3.8, 500},     GALAXIA_SPIRALIS, 0.0}},           &instr_nulla},
        {"planeta_plenus",   {.g.vagans = {{SIDUS_VAGANS,   1.0, 5500},    0.0, 0.5}},  &instr_nulla},
        {"planeta_falcatus", {.g.vagans = {{SIDUS_VAGANS,   1.5, 4500},    0.35, 0.8}}, &instr_nulla},
    };

    int n_spec = (int)(sizeof(specimina) / sizeof(specimina[0]));
    unsigned char fen[FEN * FEN * 4];

    for (int i = 0; i < n_spec; i++) {
        sidus_reddere(fen, &specimina[i].sidus, specimina[i].instr);
        int px = pixels_activos(fen, FEN * FEN);

        if (px > 0) {
            fprintf(stderr, "  %-20s %4d pixels — BENE\n", specimina[i].nomen, px);
        } else {
            fprintf(stderr, "  %-20s nullus pixel — MALUM\n", specimina[i].nomen);
            errores++;
        }
    }

    /* --- temperatura ad colorem --- */
    fprintf(stderr, "\nTemperatura ad colorem:\n");
    double tempp[] = {2500, 5800, 10000, 25000, 40000};
    for (int i = 0; i < 5; i++) {
        color_t col = sidus_temperatura_ad_colorem(tempp[i]);
        fprintf(
            stderr, "  %7.0fK → R=%.2f G=%.2f B=%.2f\n",
            tempp[i], col.r, col.g, col.b
        );
    }

    fprintf(
        stderr, "\n%s\n", errores == 0 ? "Omnes probationes praeterierunt."
        : "ERRORES inventi!"
    );
    return errores;
}
