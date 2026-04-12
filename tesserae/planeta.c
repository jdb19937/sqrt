/*
 * planeta.c — renderer planetarum et lunarum
 *
 * Colores derivantur ex compositione chimica et physica
 * atmosphaerica. Nullae overrides colorum.
 *
 * Unitates:
 *   pressio_kPa      kilopascal (Terra = 101.325 kPa)
 *   inclinatio        radiani
 *   rotatio           radiani
 *   compositiones     fractiones [0, 1]
 *   craterae          densitas [0, 1] (1 = Callisto)
 *   vulcanismus       activitas [0, 1] (1 = Io)
 *   tectonica         complexitas [0, 1]
 *   aqua              fractio superficiei [0, 1]
 *   aqua_profunditas  fractio [0, 1] (0 = vadosa, 1 = abyssalis)
 *   polaris           fractio superficiei [0, 1]
 */

#include "planeta_communia.h"

#include <stdio.h>

/* ================================================================
 * renderers et parsores per genus
 * ================================================================ */

/* ================================================================
 * dispatcher
 * ================================================================ */

void planeta_reddere(
    unsigned char *fenestra, const planeta_t *planeta
) {
    memset(fenestra, 0, FEN * FEN * 4);
    switch (planeta->qui) {
    case PLANETA_SAXOSUM:  reddere_saxosum(fenestra, &planeta->ubi.saxosum);   break;
    case PLANETA_GASEOSUM: reddere_gaseosum(fenestra, &planeta->ubi.gaseosum); break;
    case PLANETA_GLACIALE: reddere_glaciale(fenestra, &planeta->ubi.glaciale); break;
    case PLANETA_PARVUM:   reddere_parvum(fenestra, &planeta->ubi.parvum);     break;
    case PLANETA_SOL:      reddere_sol(fenestra, &planeta->ubi.sol);           break;
    case PLANETA_NEBULA:   reddere_nebula(fenestra, &planeta->ubi.nebula);     break;
    }
}

void planeta_illuminationem_applicare(
    unsigned char *fen, double radius,
    const planeta_perceptus_t *perc
) {
    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            int idx = (py * FEN + px) * 4;
            if (fen[idx + 3] == 0)
                continue;

            double illum = illuminatio(px, py, radius,
                perc->aspectus.situs, perc->aspectus.angulus)
                * perc->aspectus.lumen;
            if (perc->coaspectus.lumen > 0.001)
                illum += illuminatio(px, py, radius,
                    perc->coaspectus.situs, perc->coaspectus.angulus)
                    * perc->coaspectus.lumen;

            if (illum < 0.003) {
                /* latus obscurum — opacum nigrum */
                fen[idx + 0] = 0;
                fen[idx + 1] = 0;
                fen[idx + 2] = 0;
            } else {
                if (illum > 1.0) illum = 1.0;
                fen[idx + 0] = (unsigned char)(fen[idx + 0] * illum);
                fen[idx + 1] = (unsigned char)(fen[idx + 1] * illum);
                fen[idx + 2] = (unsigned char)(fen[idx + 2] * illum);
            }
        }
    }
}

/* ================================================================
 * ISON parser
 * ================================================================ */

planeta_t *planeta_ex_ison(const char *ison)
{
    /* detege genus ex clavibus praesentibus */
    planetarius_t g = PLANETA_SAXOSUM;
    static const struct { const char *clavis; planetarius_t genus; } detectio[] = {
        {"soliculum",   PLANETA_SOL},
        {"nebulula",    PLANETA_NEBULA},
        {"gaseosculum", PLANETA_GASEOSUM},
        {"glaciellum",  PLANETA_GLACIALE},
        {"parvulum",    PLANETA_PARVUM},
    };
    for (int i = 0; i < 5; i++) {
        char *tmp = ison_da_crudum(ison, detectio[i].clavis);
        if (tmp) { free(tmp); g = detectio[i].genus; break; }
    }

    planeta_t *p = calloc(1, sizeof(planeta_t));
    if (!p) return NULL;
    p->qui = g;

    switch (g) {
    case PLANETA_SAXOSUM:  saxosum_ex_ison(&p->ubi.saxosum, ison);   break;
    case PLANETA_GASEOSUM: gaseosum_ex_ison(&p->ubi.gaseosum, ison); break;
    case PLANETA_GLACIALE: glaciale_ex_ison(&p->ubi.glaciale, ison); break;
    case PLANETA_PARVUM:   parvum_ex_ison(&p->ubi.parvum, ison);    break;
    case PLANETA_SOL:      sol_ex_ison(&p->ubi.sol, ison);          break;
    case PLANETA_NEBULA:   nebula_ex_ison(&p->ubi.nebula, ison);    break;
    }
    return p;
}

/* ================================================================
 * ISON serializer
 * ================================================================ */

void planeta_in_ison(FILE *f, const planeta_t *p)
{
    switch (p->qui) {
    case PLANETA_SAXOSUM:  saxosum_in_ison(f, &p->ubi.saxosum);   break;
    case PLANETA_GASEOSUM: gaseosum_in_ison(f, &p->ubi.gaseosum); break;
    case PLANETA_GLACIALE: glaciale_in_ison(f, &p->ubi.glaciale); break;
    case PLANETA_PARVUM:   parvum_in_ison(f, &p->ubi.parvum);    break;
    case PLANETA_SOL:      sol_in_ison(f, &p->ubi.sol);          break;
    case PLANETA_NEBULA:   nebula_in_ison(f, &p->ubi.nebula);    break;
    }
}

/* ================================================================
 * atmosphaera → instrumentum
 *
 * Mutat instrumentum basale superponendo effectus atmosphaericos.
 * Seeing additur in quadratura (fontes independentes).
 *
 * Unitates derivatae:
 *   pressio_kPa / 101 → normalized [0,1]
 *   seeing       = norm² × 3.0    [pixels]
 *   scintillatio = norm × 0.4     [amplitudo]
 *   caeli_lumen  = norm² × 0.15   [intensitas]
 *   refractio    = norm × 1.5     [pixels]
 * ================================================================ */

void planeta_instrumentum_applicare(
    double pressio_kPa,
    instrumentum_t *instr
) {
    double a = fmin(1.0, pressio_kPa / 101.0);
    if (a < 0.005)
        return;

    double seeing = a * a * 3.0;
    double v2     = instr->visio * instr->visio + seeing * seeing;
    instr->visio  = sqrt(v2);

    instr->scintillatio += a * 0.4;
    if (instr->scintillatio > 1.0)
        instr->scintillatio = 1.0;

    instr->caeli_lumen += a * a * 0.15;
    instr->refractio += a * 1.5;

    double desat = 1.0 - a * 0.15;
    if (desat < 0.3)
        desat = 0.3;
    instr->saturatio *= desat;
}
