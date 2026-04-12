/*
 * sidus.c — sidera singula, renderer
 *
 * Reddit singula sidera in fenestra 64x64.
 */

#include "sidus_communia.h"

#include <stdio.h>

unsigned int sidus_semen_g = 1;

const char *sidus_nomina_generum[SIDUS_NUMERUS] = {
    "Nanum Album",
    "Sequentia",
    "Gigas Rubrum",
    "Supergigas",
    "Neutronium",
    "Crystallinum",
    "Magnetar",
    "Galaxia",
    "Vagans"
};

/* ================================================================
 * temperatura ad colorem (approximatio Planck)
 * ================================================================ */

color_t sidus_temperatura_ad_colorem(double kelvin)
{
    /* Approximatio Tanner Helland (2012) functionis Planckianae
     * per CIE 1931 2° standard observer.
     * Exacta intra 1% pro 1000K-40000K contra integrationem
     * spectri Planckiani B(λ,T) = 2hc²/λ⁵ · 1/(e^(hc/λkT)-1)
     * convolutam cum x̄(λ), ȳ(λ), z̄(λ) et transformatam per
     * sRGB matricem (Rec. 709 primariis). */
    double t = kelvin / 100.0;
    double r, g, b;

    if (t <= 66.0) {
        r = 255.0;
        g = 99.4708 * log(t) - 161.1196;
        if (g < 0)
            g = 0;
        if (g > 255)
            g = 255;
    } else {
        r = 329.699 * pow(t - 60.0, -0.1332);
        if (r < 0)
            r = 0;
        if (r > 255)
            r = 255;
        g = 288.122 * pow(t - 60.0, -0.0755);
        if (g < 0)
            g = 0;
        if (g > 255)
            g = 255;
    }

    if (t >= 66.0) {
        b = 255.0;
    } else if (t <= 19.0) {
        b = 0.0;
    } else {
        b = 138.518 * log(t - 10.0) - 305.045;
        if (b < 0)
            b = 0;
        if (b > 255)
            b = 255;
    }

    return (color_t){r / 255.0, g / 255.0, b / 255.0, 1.0};
}

/* ================================================================
 * ISON
 * ================================================================ */

void sidus_in_ison(FILE *f, const sidus_t *s)
{
    switch (s->qui) {
    case SIDUS_NANUM_ALBUM:  nanum_album_in_ison(f, &s->ubi.nanum_album);   break;
    case SIDUS_SEQUENTIA:    sequentia_in_ison(f, &s->ubi.sequentia);        break;
    case SIDUS_GIGAS_RUBRUM: gigas_rubrum_in_ison(f, &s->ubi.gigas_rubrum); break;
    case SIDUS_SUPERGIGAS:   supergigas_in_ison(f, &s->ubi.supergigas);     break;
    case SIDUS_NEUTRONIUM:   neutronium_in_ison(f, &s->ubi.neutronium);     break;
    case SIDUS_CRYSTALLINUM: crystallinum_in_ison(f, &s->ubi.crystallinum); break;
    case SIDUS_MAGNETAR:     magnetar_in_ison(f, &s->ubi.magnetar);         break;
    case SIDUS_GALAXIA:      galaxia_in_ison(f, &s->ubi.galaxia);           break;
    case SIDUS_VAGANS:       vagans_in_ison(f, &s->ubi.vagans);             break;
    default: break;
    }
}

void sidus_ex_ison(sidus_t *s, const char *ison)
{
    memset(s, 0, sizeof(*s));
    char *tmp;
    if ((tmp = ison_da_crudum(ison, "vaganulus"))) {
        free(tmp); s->qui = SIDUS_VAGANS;
        vagans_ex_ison(&s->ubi.vagans, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "galaxiola"))) {
        free(tmp); s->qui = SIDUS_GALAXIA;
        galaxia_ex_ison(&s->ubi.galaxia, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "magnetarulum"))) {
        free(tmp); s->qui = SIDUS_MAGNETAR;
        magnetar_ex_ison(&s->ubi.magnetar, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "nanulum_album"))) {
        free(tmp); s->qui = SIDUS_NANUM_ALBUM;
        nanum_album_ex_ison(&s->ubi.nanum_album, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "gigulum_rubrum"))) {
        free(tmp); s->qui = SIDUS_GIGAS_RUBRUM;
        gigas_rubrum_ex_ison(&s->ubi.gigas_rubrum, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "supergigulum"))) {
        free(tmp); s->qui = SIDUS_SUPERGIGAS;
        supergigas_ex_ison(&s->ubi.supergigas, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "neutroniulum"))) {
        free(tmp); s->qui = SIDUS_NEUTRONIUM;
        neutronium_ex_ison(&s->ubi.neutronium, ison); return;
    }
    if ((tmp = ison_da_crudum(ison, "crystallulum"))) {
        free(tmp); s->qui = SIDUS_CRYSTALLINUM;
        crystallinum_ex_ison(&s->ubi.crystallinum, ison); return;
    }
    /* praefinitum: sequentia (solum sidulum) */
    s->qui = SIDUS_SEQUENTIA;
    sequentia_ex_ison(&s->ubi.sequentia, ison);
}

/* ================================================================
 * dispatcher
 * ================================================================ */

void sidus_reddere(
    unsigned char *fenestra,
    const sidus_t *sidus,
    const instrumentum_t *instrumentum
) {
    memset(fenestra, 0, FEN * FEN * 4);

    switch (sidus->qui) {
    case SIDUS_NANUM_ALBUM:
        reddere_nanum_album(fenestra, &sidus->ubi.nanum_album, instrumentum);
        break;
    case SIDUS_SEQUENTIA:
        reddere_sequentia(fenestra, &sidus->ubi.sequentia, instrumentum);
        break;
    case SIDUS_GIGAS_RUBRUM:
        reddere_gigas_rubrum(fenestra, &sidus->ubi.gigas_rubrum, instrumentum);
        break;
    case SIDUS_SUPERGIGAS:
        reddere_supergigas(fenestra, &sidus->ubi.supergigas, instrumentum);
        break;
    case SIDUS_NEUTRONIUM:
        reddere_neutronium(fenestra, &sidus->ubi.neutronium, instrumentum);
        break;
    case SIDUS_CRYSTALLINUM:
        reddere_crystallinum(fenestra, &sidus->ubi.crystallinum, instrumentum);
        break;
    case SIDUS_MAGNETAR:
        reddere_magnetar(fenestra, &sidus->ubi.magnetar, instrumentum);
        break;
    case SIDUS_GALAXIA:
        reddere_galaxia(fenestra, &sidus->ubi.galaxia, instrumentum);
        break;
    case SIDUS_VAGANS:
        reddere_vagans(fenestra, &sidus->ubi.vagans, instrumentum);
        break;
    default:
        break;
    }
}
