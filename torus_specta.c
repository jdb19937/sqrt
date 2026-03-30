/*
 * torus_specta.c — spectator interactivus tori Hevea per phantasma (PFR)
 *
 * Torum planum corrugatum in fenestra PFR reddit.
 * Usitor camerae rotationem et conspectum clavibus regit.
 *
 * Claves:
 *   Sagittae sin/dex    — azimuthum camerae mutare
 *   Sagittae sur/deor   — elevationem camerae mutare
 *   W / S               — propinquare / recedere
 *   X / Y / Z           — axem rotationis automaticae eligere
 *   Spatium              — rotationem automaticam sistere / resumere
 *   +/- (vel =/-)       — celeritatem rotationis mutare
 *   Tab                  — thema proximum
 *   1/2                  — radium maiorem minuere / augere
 *   3/4                  — radium minorem minuere / augere
 *   Rota muris           — propinquare / recedere
 *   C                    — inscriptionem incipere / finire (→ MP4)
 *   L                    — unum cyclum inscribere (→ MP4 + GIF)
 *   R                    — restituere
 *   Q / Escape           — exire
 */

#include "helvea.h"
#include "astra.h"

#include "phantasma.h"

#include <stdio.h>
#include <time.h>

#define LATITUDO_IMG  768
#define ALTITUDO_IMG  768
#define GRADUS_U_INIT 300
#define GRADUS_V_INIT 150
#define GRADUS_MIN    40
#define GRADUS_MAX    2000


/* themata et illuminatio thematica nunc in helvea.h/c */

/* ================================================================
 * post-effecta
 * ================================================================ */

static unsigned int semen_pfx = 12345;
static inline unsigned int aleatorium(void)
{
    semen_pfx ^= semen_pfx << 13;
    semen_pfx ^= semen_pfx >> 17;
    semen_pfx ^= semen_pfx << 5;
    return semen_pfx;
}

static void pfx_applicare(tabula_t *t, int pfx, int posteriza_niv)
{
    if (pfx == HELVEA_PFX_NULLUS) return;

    size_t n_pix = (size_t)t->latitudo * t->altitudo;

    unsigned char *limbus = NULL;
    if (pfx & HELVEA_PFX_LINEAE) {
        limbus = (unsigned char *)calloc(n_pix, 1);
        for (int y = 1; y < t->altitudo - 1; y++) {
            for (int x = 1; x < t->latitudo - 1; x++) {
                int c = y * t->latitudo + x;
                double d = t->profunditatis[c];
                if (d > 1e20) continue;

                double dl = t->profunditatis[c - 1];
                double dr = t->profunditatis[c + 1];
                double du = t->profunditatis[c - t->latitudo];
                double dd = t->profunditatis[c + t->latitudo];

                if (dl > 1e20 || dr > 1e20 || du > 1e20 || dd > 1e20) {
                    limbus[c] = 255;
                    continue;
                }

                double gx = fabs(dr - dl);
                double gy = fabs(dd - du);
                double g = (gx + gy) * 25.0;
                if (g > 1.0) g = 1.0;
                limbus[c] = (unsigned char)(g * 255.0);
            }
        }
    }

    for (size_t i = 0; i < n_pix; i++) {
        size_t base = i * 4;
        double b = t->imaginis[base + 0] / 255.0;
        double g = t->imaginis[base + 1] / 255.0;
        double r = t->imaginis[base + 2] / 255.0;

        if ((pfx & HELVEA_PFX_POSTERIZA) && posteriza_niv > 0) {
            double nf = (double)posteriza_niv;
            r = floor(r * nf + 0.5) / nf;
            g = floor(g * nf + 0.5) / nf;
            b = floor(b * nf + 0.5) / nf;
        }

        if (pfx & HELVEA_PFX_GRANUM) {
            double rumore = ((double)(aleatorium() & 0xFF) / 255.0 - 0.5) * 0.08;
            r += rumore;
            g += rumore;
            b += rumore;
        }

        if (pfx & HELVEA_PFX_NIGRESCO) {
            int px = (int)(i % (size_t)t->latitudo);
            int py = (int)(i / (size_t)t->latitudo);
            double dx = (px - t->latitudo * 0.5) / (t->latitudo * 0.5);
            double dy = (py - t->altitudo * 0.5) / (t->altitudo * 0.5);
            double dist = dx * dx + dy * dy;
            double obscura = 1.0 - dist * 0.45;
            if (obscura < 0.2) obscura = 0.2;
            r *= obscura;
            g *= obscura;
            b *= obscura;
        }

        if (limbus && limbus[i] > 0) {
            double lf = limbus[i] / 255.0;
            r *= (1.0 - lf);
            g *= (1.0 - lf);
            b *= (1.0 - lf);
        }

        if (r < 0) r = 0; if (r > 1) r = 1;
        if (g < 0) g = 0; if (g > 1) g = 1;
        if (b < 0) b = 0; if (b > 1) b = 1;
        t->imaginis[base + 0] = (unsigned char)(b * 255.0);
        t->imaginis[base + 1] = (unsigned char)(g * 255.0);
        t->imaginis[base + 2] = (unsigned char)(r * 255.0);
    }

    free(limbus);
}

/* ================================================================
 * principium
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    fprintf(stderr, "Superficiem computans...\n");

    double radius_maior = HELVEA_RADIUS_MAIOR;
    double radius_minor = HELVEA_RADIUS_MINOR;
    helvea_methodus_t methodus = HELVEA_BORRELLI;
    helvea_strata = 5;
    int gradus = 6;
    #define GRADUS_U(g) (10 * (1 << (g)))
    #define GRADUS_V(g) (5 * (1 << (g)))

    size_t n_vert = (size_t)(GRADUS_U(gradus) + 1) * (GRADUS_V(gradus) + 1);
    vec3_t *puncta_orig = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *normae_orig = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *puncta_rot  = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    vec3_t *normae_rot  = (vec3_t *)malloc(n_vert * sizeof(vec3_t));
    int gradus_mutatus = 0;

    if (!puncta_orig || !normae_orig || !puncta_rot || !normae_rot) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    helvea_superficiem_computare(puncta_orig, normae_orig,
                                 GRADUS_U(gradus), GRADUS_V(gradus),
                                 radius_maior, radius_minor, methodus);
    int superficies_obsoleta = 0;
    fprintf(stderr, "Superficies parata: %zu vertices.\n", n_vert);

    /* campum stellarum ex ISONL reddere */
    const char *via_isonl = "caelae/terra.isonl";
    const char *via_instr = "instrumenta/oculus.ison";
    fprintf(stderr, "Campum stellarum reddens: %s + %s\n", via_isonl, via_instr);
    astra_campus_t *campus = astra_ex_isonl_reddere(via_isonl, via_instr);
    if (!campus) {
        fprintf(stderr, "ERROR: campus stellarum reddere non possum!\n");
        return 1;
    }
    fprintf(stderr, "Campus stellarum paratus.\n");

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    tabula_t tab;
    tab.latitudo = LATITUDO_IMG;
    tab.altitudo = ALTITUDO_IMG;
    tab.bytes_pixel = 4;
    tab.imaginis = (unsigned char *)malloc(n_pix * 4);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (pfr_initia(PFR_INITIA_VIDEO) != 0) {
        fprintf(stderr, "ERROR pfr_initia: %s\n", pfr_erratum());
        return 1;
    }

    pfr_fenestra_t *fenestra = pfr_fenestram_crea(
        "Torus Hevea — Spectator",
        PFR_POS_MEDIUM, PFR_POS_MEDIUM,
        LATITUDO_IMG, ALTITUDO_IMG, 0);
    if (!fenestra) {
        fprintf(stderr, "ERROR fenestra: %s\n", pfr_erratum());
        pfr_fini();
        return 1;
    }

    pfr_pictor_t *pictor = pfr_pictorem_crea(
        fenestra, -1, PFR_PICTOR_CELER | PFR_PICTOR_SYNC);
    if (!pictor) {
        fprintf(stderr, "ERROR pictor: %s\n", pfr_erratum());
        pfr_fenestram_destrue(fenestra);
        pfr_fini();
        return 1;
    }

    pfr_textura_t *textura = pfr_texturam_crea(
        pictor, PFR_PIXEL_ARGB8888, PFR_TEXTURA_FLUENS,
        LATITUDO_IMG, ALTITUDO_IMG);
    if (!textura) {
        fprintf(stderr, "ERROR textura: %s\n", pfr_erratum());
        pfr_pictorem_destrue(pictor);
        pfr_fenestram_destrue(fenestra);
        pfr_fini();
        return 1;
    }

    /* positio in toro T² — duo anguli libere volventes */
    double theta       = 0.6;    /* toroidalis (horizontalis) */
    double phi         = 0.4;    /* poloidalis (verticalis) */
    double distantia   = 3.2;
    double angulus_rot  = 0.0;
    double celeritas    = 0.36;
    int    axis_index   = 1;
    int    rotatio_activa = 1;

    functio_rotandi rotationes[3] = { rotare_x, rotare_y, rotare_z };
    const char *nomina_axium[3] = { "X", "Y", "Z" };

    fprintf(stderr, "Spectator paratus. Claves:\n");
    fprintf(stderr, "  Sagittae   — azimuthum et elevationem\n");
    fprintf(stderr, "  W/S        — propinquare / recedere\n");
    fprintf(stderr, "  X/Y/Z      — axem rotationis eligere\n");
    fprintf(stderr, "  Spatium    — rotationem sistere / resumere\n");
    fprintf(stderr, "  +/-        — celeritatem mutare\n");
    fprintf(stderr, "  Tab        — thema proximum\n");
    fprintf(stderr, "  M          — methodum superficiei mutare\n");
    fprintf(stderr, "  1/2        — radium maiorem (R=%.2f)\n", radius_maior);
    fprintf(stderr, "  3/4        — radium minorem (r=%.2f)\n", radius_minor);
    fprintf(stderr, "  C          — inscriptionem incipere / finire (MP4)\n");
    fprintf(stderr, "  L          — unum cyclum rotationis inscribere (MP4 + GIF)\n");
    fprintf(stderr, "  R          — restituere\n");
    fprintf(stderr, "  Q/Escape   — exire\n");
    fprintf(stderr, "Thema [0]: %s\n", helvea_themata[helvea_index_thematis].nomen);
    fprintf(stderr, "Methodus [0]: %s\n", helvea_nomina_methodorum[methodus]);

    /* inscriptio (recording) — directe per phantasma inscriptores */
    pfr_mp4_t *inscr_mp4 = NULL;
    int inscr_tabula = 0;

    /* orbita inscriptio — unus cyclus exactus */
    pfr_mp4_t *orbita_mp4 = NULL;
    pfr_gif_t *orbita_gif = NULL;
    int orbita_tabula = 0;
    double orbita_angulus_init = 0;
    char orbita_nomen[256] = {0};

    /* indicium status in terminali */
    char status_nuntius[128] = "";
    int tabulae_fps = 0;
    pfr_u32 tempus_fps = pfr_tempus();

    int currit = 1;
    pfr_u32 tempus_prius = pfr_tempus();

    while (currit) {
        pfr_u32 tempus_nunc = pfr_tempus();
        double dt = (double)(tempus_nunc - tempus_prius) / 1000.0;
        tempus_prius = tempus_nunc;

        pfr_eventus_t eventus;
        while (pfr_eventum_lege(&eventus)) {
            if (eventus.typus == PFR_EXITUS) {
                currit = 0;
            } else if (eventus.typus == PFR_ROTA_MURIS) {
                distantia -= eventus.rota.y * 0.25;
                if (distantia < 1.8) distantia = 1.8;
                if (distantia > 8.0) distantia = 8.0;
                if (orbita_mp4) {
                    pfr_mp4_fini(orbita_mp4);
                    pfr_gif_fini(orbita_gif);
                    orbita_mp4 = NULL;
                    orbita_gif = NULL;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Orbita cancellata");
                }
            } else if (eventus.typus == PFR_CLAVIS_INF) {
                switch (eventus.clavis.signum.symbolum) {
                case PFR_CL_EFFUGIUM:
                case 'q':
                    currit = 0;
                    break;
                case PFR_CL_SINISTRUM:
                case PFR_CL_DEXTRUM:
                case PFR_CL_SURSUM:
                case PFR_CL_DEORSUM:
                    if (eventus.clavis.signum.symbolum == PFR_CL_SINISTRUM) theta -= 0.08;
                    if (eventus.clavis.signum.symbolum == PFR_CL_DEXTRUM)   theta += 0.08;
                    if (eventus.clavis.signum.symbolum == PFR_CL_SURSUM)    phi   += 0.06;
                    if (eventus.clavis.signum.symbolum == PFR_CL_DEORSUM)   phi   -= 0.06;
                    if (orbita_mp4) {
                        pfr_mp4_fini(orbita_mp4);
                        pfr_gif_fini(orbita_gif);
                        orbita_mp4 = NULL;
                        orbita_gif = NULL;
                        snprintf(status_nuntius, sizeof(status_nuntius),
                             "Orbita cancellata");
                    }
                    break;
                case 'w':
                    distantia -= 0.15;
                    if (distantia < 1.8) distantia = 1.8;
                    break;
                case 's':
                    distantia += 0.15;
                    if (distantia > 8.0) distantia = 8.0;
                    break;
                case 'x':
                    axis_index = 0;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Axis: %s", nomina_axium[axis_index]);
                    break;
                case 'y':
                    axis_index = 1;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Axis: %s", nomina_axium[axis_index]);
                    break;
                case 'z':
                    axis_index = 2;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Axis: %s", nomina_axium[axis_index]);
                    break;
                case PFR_CL_SPATIUM:
                    rotatio_activa = !rotatio_activa;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Rotatio: %s",
                             rotatio_activa ? "activa" : "sistita");
                    break;
                case PFR_CL_AEQUALE:
                case PFR_CL_PLUS:
                    celeritas *= 1.3;
                    if (celeritas > 6.0) celeritas = 6.0;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Celeritas: %.2f rad/s", celeritas);
                    break;
                case PFR_CL_MINUS:
                    celeritas /= 1.3;
                    if (celeritas < 0.05) celeritas = 0.05;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Celeritas: %.2f rad/s", celeritas);
                    break;
                case PFR_CL_1:
                    radius_maior -= 0.05;
                    if (radius_maior < 0.3) radius_maior = 0.3;
                    superficies_obsoleta = 1;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "R=%.2f r=%.2f", radius_maior, radius_minor);
                    break;
                case PFR_CL_2:
                    radius_maior += 0.05;
                    if (radius_maior > 3.0) radius_maior = 3.0;
                    superficies_obsoleta = 1;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "R=%.2f r=%.2f", radius_maior, radius_minor);
                    break;
                case PFR_CL_3:
                    radius_minor -= 0.02;
                    if (radius_minor < 0.08) radius_minor = 0.08;
                    superficies_obsoleta = 1;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "R=%.2f r=%.2f", radius_maior, radius_minor);
                    break;
                case PFR_CL_4:
                    radius_minor += 0.02;
                    if (radius_minor > 1.5) radius_minor = 1.5;
                    superficies_obsoleta = 1;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "R=%.2f r=%.2f", radius_maior, radius_minor);
                    break;
                case PFR_CL_TABULA:
                    helvea_index_thematis = (helvea_index_thematis + 1) % helvea_numerus_thematum;
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Thema [%d]: %s",
                             helvea_index_thematis,
                             helvea_themata[helvea_index_thematis].nomen);
                    break;
                case 'm':
                    methodus = (methodus + 1) % HELVEA_NUMERUS_METHODORUM;
                    superficies_obsoleta = 1;
                    break;
                case PFR_CL_5:
                    helvea_strata--;
                    if (helvea_strata < 1) helvea_strata = 1;
                    superficies_obsoleta = 1;
                    break;
                case PFR_CL_6:
                    helvea_strata++;
                    if (helvea_strata > HELVEA_STRATA_MAX) helvea_strata = HELVEA_STRATA_MAX;
                    superficies_obsoleta = 1;
                    break;
                case PFR_CL_7:
                    if (gradus > 1) {
                        gradus--;
                        gradus_mutatus = 1;
                        superficies_obsoleta = 1;
                    }
                    break;
                case PFR_CL_8:
                    if (gradus < 7) {
                        gradus++;
                        gradus_mutatus = 1;
                        superficies_obsoleta = 1;
                    }
                    break;
                case 'c':
                    if (!inscr_mp4) {
                        /* incipe inscriptionem */
                        time_t nunc = time(NULL);
                        struct tm *tm = localtime(&nunc);
                        char via[256];
                        snprintf(via, sizeof(via),
                                 "caelae/inscr_%04d%02d%02d_%02d%02d%02d.mp4",
                                 tm->tm_year + 1900, tm->tm_mon + 1,
                                 tm->tm_mday, tm->tm_hour,
                                 tm->tm_min, tm->tm_sec);
                        inscr_mp4 = pfr_mp4_initia(via, LATITUDO_IMG, ALTITUDO_IMG, 30);
                        inscr_tabula = 0;
                        snprintf(status_nuntius, sizeof(status_nuntius),
                                 "● INSCRIPTIO: %s", via);
                    } else {
                        snprintf(status_nuntius, sizeof(status_nuntius),
                                 "■ INSCRIPTIO FINITA: %d tabulae",
                                 inscr_tabula);
                        pfr_mp4_fini(inscr_mp4);
                        inscr_mp4 = NULL;
                    }
                    break;
                case 'l':
                    if (!orbita_mp4 && rotatio_activa) {
                        time_t nunc = time(NULL);
                        struct tm *tm = localtime(&nunc);
                        snprintf(orbita_nomen, sizeof(orbita_nomen),
                                 "caelae/orbita_%04d%02d%02d_%02d%02d%02d",
                                 tm->tm_year + 1900, tm->tm_mon + 1,
                                 tm->tm_mday, tm->tm_hour,
                                 tm->tm_min, tm->tm_sec);
                        char via_mp4[280];
                        char via_gif[280];
                        snprintf(via_mp4, sizeof(via_mp4), "%s.mp4", orbita_nomen);
                        snprintf(via_gif, sizeof(via_gif), "%s.gif", orbita_nomen);
                        orbita_mp4 = pfr_mp4_initia(via_mp4,
                                                     LATITUDO_IMG, ALTITUDO_IMG, 30);
                        orbita_gif = pfr_gif_initia(via_gif,
                                                     LATITUDO_IMG, ALTITUDO_IMG, 3, 2);
                        orbita_tabula = 0;
                        orbita_angulus_init = angulus_rot;
                        snprintf(status_nuntius, sizeof(status_nuntius),
                                 "◎ ORBITA: %s (unus cyclus)",
                                 orbita_nomen);
                    }
                    break;
                case 'r':
                    theta     = 0.6;
                    phi       = 0.4;
                    distantia = 3.2;
                    angulus_rot = 0.0;
                    celeritas  = 0.12;
                    axis_index = 2;
                    rotatio_activa = 1;
                    if (radius_maior != HELVEA_RADIUS_MAIOR ||
                        radius_minor != HELVEA_RADIUS_MINOR ||
                        methodus != HELVEA_BORRELLI) {
                        radius_maior = HELVEA_RADIUS_MAIOR;
                        radius_minor = HELVEA_RADIUS_MINOR;
                        methodus = HELVEA_BORRELLI;
                        superficies_obsoleta = 1;
                    }
                    snprintf(status_nuntius, sizeof(status_nuntius),
                             "Conspectus restitutus");
                    break;
                default:
                    break;
                }
            }
        }

        const pfr_u8 *status_clavium = pfr_claves_status(NULL);
        int perturbatum = 0;
        if (status_clavium[PFR_SC_SINISTRUM]) { theta -= 1.5 * dt; perturbatum = 1; }
        if (status_clavium[PFR_SC_DEXTRUM])   { theta += 1.5 * dt; perturbatum = 1; }
        if (status_clavium[PFR_SC_SURSUM])    { phi   += 1.0 * dt; perturbatum = 1; }
        if (status_clavium[PFR_SC_DEORSUM])   { phi   -= 1.0 * dt; perturbatum = 1; }
        if (status_clavium[PFR_SC_W])         { distantia -= 2.0 * dt; perturbatum = 1; }
        if (status_clavium[PFR_SC_S])         { distantia += 2.0 * dt; perturbatum = 1; }
        if (perturbatum && orbita_mp4) {
            pfr_mp4_fini(orbita_mp4);
            pfr_gif_fini(orbita_gif);
            orbita_mp4 = NULL;
            orbita_gif = NULL;
            fprintf(stderr, "  orbita cancellata\n");
        }
        if (distantia < 1.8) distantia = 1.8;
        if (distantia > 8.0) distantia = 8.0;

        if (rotatio_activa)
            angulus_rot += celeritas * dt;

        if (gradus_mutatus) {
            n_vert = (size_t)(GRADUS_U(gradus) + 1) * (GRADUS_V(gradus) + 1);
            puncta_orig = (vec3_t *)realloc(puncta_orig, n_vert * sizeof(vec3_t));
            normae_orig = (vec3_t *)realloc(normae_orig, n_vert * sizeof(vec3_t));
            puncta_rot  = (vec3_t *)realloc(puncta_rot,  n_vert * sizeof(vec3_t));
            normae_rot  = (vec3_t *)realloc(normae_rot,  n_vert * sizeof(vec3_t));
            gradus_mutatus = 0;
        }

        if (superficies_obsoleta) {
            helvea_superficiem_computare(puncta_orig, normae_orig,
                                         GRADUS_U(gradus), GRADUS_V(gradus),
                                         radius_maior, radius_minor, methodus);
            superficies_obsoleta = 0;
        }

        functio_rotandi rotare = rotationes[axis_index];
        for (size_t i = 0; i < n_vert; i++) {
            puncta_rot[i] = rotare(puncta_orig[i], angulus_rot);
            normae_rot[i] = rotare(normae_orig[i], angulus_rot);
        }

        /* camera: positio ex coordinatis toroidalibus (theta, phi).
         * theta = horizontalis, phi = verticalis. ambo libere volvuntur.
         * camera orbitat in plano inclinato per phi. */
        vec3_t pos_cam = vec3(distantia * cos(theta),
                            distantia * sin(theta),
                            0.0);
        pos_cam = rotare_x(pos_cam, phi);
        vec3_t scopus = vec3(0.0, 0.0, 0.0);

        /* sursum: perpendiculare ad orbitam, rotatum per phi */
        vec3_t sursum_cam = rotare_x(vec3(0.0, 0.0, 1.0), phi);
        camera_t cam;
        cam.positio = pos_cam;
        cam.ante    = normalizare(differentia(scopus, pos_cam));
        cam.dextrum = normalizare(productum_vectoriale(cam.ante, sursum_cam));
        cam.sursum  = productum_vectoriale(cam.dextrum, cam.ante);
        cam.focalis = 1.6;

        /* fundum stellarum — toroidaliter volvitur cum (theta, phi) */
        tabulam_purgare(&tab);
        int delta_x = (int)(theta / DUO_PI * campus->latitudo);
        int delta_y = (int)(phi   / DUO_PI * campus->altitudo);
        fundum_implere(&tab, campus->pixels,
                              campus->latitudo, campus->altitudo,
                              delta_x, delta_y);

        scaenam_reddere(&tab, puncta_rot, normae_rot,
                               GRADUS_U(gradus), GRADUS_V(gradus), &cam,
                               helvea_illuminare_thema, pixel_bgra);

        pfx_applicare(&tab, helvea_themata[helvea_index_thematis].pfx,
                       helvea_themata[helvea_index_thematis].posteriza_niv);

        pfr_texturam_renova(textura, NULL, tab.imaginis, LATITUDO_IMG * 4);
        pfr_purga(pictor);
        pfr_texturam_pinge(pictor, textura, NULL, NULL);

        /* indicium inscriptionis — circulus ruber */
        if (inscr_mp4) {
            pfr_colorem_pone(pictor, 220, 30, 30, 255);
            for (int dy = -5; dy <= 5; dy++)
                for (int dx = -5; dx <= 5; dx++)
                    if (dx * dx + dy * dy <= 25)
                        pfr_punctum_pinge(pictor, 20 + dx, 20 + dy);
        }

        /* indicium orbitae — circulus caeruleus */
        if (orbita_mp4) {
            pfr_colorem_pone(pictor, 30, 120, 220, 255);
            for (int dy = -5; dy <= 5; dy++)
                for (int dx = -5; dx <= 5; dx++)
                    if (dx * dx + dy * dy <= 25)
                        pfr_punctum_pinge(pictor, inscr_mp4 ? 40 : 20,
                                            20 + dy + dx * 0);
            /* arcum progressus reddere */
            double prog = angulus_rot - orbita_angulus_init;
            if (prog < 0) prog = -prog;
            double frac = prog / DUO_PI;
            if (frac > 1.0) frac = 1.0;
            int arc_px = (int)(frac * (LATITUDO_IMG - 40));
            pfr_colorem_pone(pictor, 30, 120, 220, 255);
            pfr_rectum_t bar = {20, ALTITUDO_IMG - 8, arc_px, 4};
            pfr_rectum_imple(pictor, &bar);
        }

        pfr_praesenta(pictor);

        /* tabulam inscribere si inscribimus */
        if (inscr_mp4) {
            pfr_mp4_tabulam_adde(inscr_mp4, (const uint32_t *)tab.imaginis);
            inscr_tabula++;
        }
        if (orbita_mp4) {
            pfr_mp4_tabulam_adde(orbita_mp4, (const uint32_t *)tab.imaginis);
            pfr_gif_tabulam_adde(orbita_gif, (const uint32_t *)tab.imaginis);
            orbita_tabula++;
        }

        /* orbita: fini post unum cyclum (2π) */
        if (orbita_mp4) {
            double progressus = angulus_rot - orbita_angulus_init;
            if (progressus < 0) progressus = -progressus;
            if (progressus >= DUO_PI) {
                snprintf(status_nuntius, sizeof(status_nuntius),
                         "◎ ORBITA PERFECTA: %d tabulae",
                         orbita_tabula);
                pfr_mp4_fini(orbita_mp4);
                pfr_gif_fini(orbita_gif);
                orbita_mp4 = NULL;
                orbita_gif = NULL;
            }
        }

        /* indicium FPS in terminali */
        tabulae_fps++;
        pfr_u32 tempus_nunc_fps = pfr_tempus();
        pfr_u32 intervallum_fps = tempus_nunc_fps - tempus_fps;
        if (intervallum_fps >= 1000) {
            double fps = tabulae_fps * 1000.0 / intervallum_fps;
            fprintf(stderr, "\r\033[K%3.0f fps | %-12s | %-10s s%d g%d | %s | %4.2f rad/s",
                    fps,
                    helvea_themata[helvea_index_thematis].nomen,
                    helvea_nomina_methodorum[methodus],
                    helvea_strata,
                    gradus,
                    nomina_axium[axis_index],
                    celeritas);
            if (status_nuntius[0]) {
                fprintf(stderr, " | %s", status_nuntius);
                status_nuntius[0] = '\0';
            }
            tabulae_fps = 0;
            tempus_fps = tempus_nunc_fps;
        }
    }

    fprintf(stderr, "\r\033[K");

    /* inscriptiones finire */
    if (inscr_mp4) {
        fprintf(stderr, "■ INSCRIPTIO FINITA: %d tabulae\n", inscr_tabula);
        pfr_mp4_fini(inscr_mp4);
    }
    if (orbita_mp4) {
        fprintf(stderr, "Orbita cancellata (exit)\n");
        pfr_mp4_fini(orbita_mp4);
        pfr_gif_fini(orbita_gif);
    }

    astra_campum_destruere(campus);
    free(puncta_orig);
    free(normae_orig);
    free(puncta_rot);
    free(normae_rot);
    free(tab.imaginis);
    free(tab.profunditatis);

    pfr_texturam_destrue(textura);
    pfr_pictorem_destrue(pictor);
    pfr_fenestram_destrue(fenestra);
    pfr_fini();

    fprintf(stderr, "Vale.\n");
    return 0;
}
