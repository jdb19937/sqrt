/*
 * torus_specta.c — spectator interactivus tori Hevea per libSDL2
 *
 * Torum planum corrugatum in fenestra SDL2 reddit.
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
 *   R                    — restituere
 *   Q / Escape           — exire
 */

#include "helvea.h"

#include <SDL.h>

#include <stdio.h>

#define LATITUDO_IMG  800
#define ALTITUDO_IMG  600
#define GRADUS_U      300
#define GRADUS_V      150

/* ================================================================
 * rotatio punctorum circa axem
 * ================================================================ */

static Vec3 rotare_x(Vec3 p, double a)
{
    double ca = cos(a), sa = sin(a);
    return vec3(p.x, p.y * ca - p.z * sa, p.y * sa + p.z * ca);
}

static Vec3 rotare_y(Vec3 p, double a)
{
    double ca = cos(a), sa = sin(a);
    return vec3(p.x * ca + p.z * sa, p.y, -p.x * sa + p.z * ca);
}

static Vec3 rotare_z(Vec3 p, double a)
{
    double ca = cos(a), sa = sin(a);
    return vec3(p.x * ca - p.y * sa, p.x * sa + p.y * ca, p.z);
}

typedef Vec3 (*functio_rotandi)(Vec3, double);

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

static void pfx_applicare(helvea_tabula_t *t, int pfx, int posteriza_niv)
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
    helvea_methodus_t methodus = HELVEA_CORRUGATA;

    size_t n_vert = (size_t)(GRADUS_U + 1) * (GRADUS_V + 1);
    Vec3 *puncta_orig = (Vec3 *)malloc(n_vert * sizeof(Vec3));
    Vec3 *normae_orig = (Vec3 *)malloc(n_vert * sizeof(Vec3));
    Vec3 *puncta_rot  = (Vec3 *)malloc(n_vert * sizeof(Vec3));
    Vec3 *normae_rot  = (Vec3 *)malloc(n_vert * sizeof(Vec3));

    if (!puncta_orig || !normae_orig || !puncta_rot || !normae_rot) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    helvea_superficiem_computare(puncta_orig, normae_orig,
                                 GRADUS_U, GRADUS_V,
                                 radius_maior, radius_minor, methodus);
    int superficies_obsoleta = 0;
    fprintf(stderr, "Superficies parata: %zu vertices.\n", n_vert);

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    helvea_tabula_t tab;
    tab.latitudo = LATITUDO_IMG;
    tab.altitudo = ALTITUDO_IMG;
    tab.bytes_pixel = 4;
    tab.imaginis = (unsigned char *)malloc(n_pix * 4);
    tab.profunditatis = (double *)malloc(n_pix * sizeof(double));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERROR SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *fenestra = SDL_CreateWindow(
        "Torus Hevea — Spectator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        LATITUDO_IMG, ALTITUDO_IMG, 0);
    if (!fenestra) {
        fprintf(stderr, "ERROR fenestra: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *pictor = SDL_CreateRenderer(
        fenestra, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pictor) {
        fprintf(stderr, "ERROR pictor: %s\n", SDL_GetError());
        SDL_DestroyWindow(fenestra);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *textura = SDL_CreateTexture(
        pictor, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        LATITUDO_IMG, ALTITUDO_IMG);
    if (!textura) {
        fprintf(stderr, "ERROR textura: %s\n", SDL_GetError());
        SDL_DestroyRenderer(pictor);
        SDL_DestroyWindow(fenestra);
        SDL_Quit();
        return 1;
    }

    double azimuthum   = 0.6;
    double elevatio    = 0.4;
    double distantia   = 3.2;
    double angulus_rot  = 0.0;
    double celeritas    = 0.8;
    int    axis_index   = 2;
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
    fprintf(stderr, "  R          — restituere\n");
    fprintf(stderr, "  Q/Escape   — exire\n");
    fprintf(stderr, "Thema [0]: %s\n", helvea_themata[helvea_index_thematis].nomen);
    fprintf(stderr, "Methodus [0]: %s\n", helvea_nomina_methodorum[methodus]);

    int currit = 1;
    Uint32 tempus_prius = SDL_GetTicks();

    while (currit) {
        Uint32 tempus_nunc = SDL_GetTicks();
        double dt = (double)(tempus_nunc - tempus_prius) / 1000.0;
        tempus_prius = tempus_nunc;

        SDL_Event eventus;
        while (SDL_PollEvent(&eventus)) {
            if (eventus.type == SDL_QUIT) {
                currit = 0;
            } else if (eventus.type == SDL_MOUSEWHEEL) {
                distantia -= eventus.wheel.y * 0.25;
                if (distantia < 1.8) distantia = 1.8;
                if (distantia > 8.0) distantia = 8.0;
            } else if (eventus.type == SDL_KEYDOWN) {
                switch (eventus.key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_q:
                    currit = 0;
                    break;
                case SDLK_LEFT:  azimuthum -= 0.08; break;
                case SDLK_RIGHT: azimuthum += 0.08; break;
                case SDLK_UP:
                    elevatio += 0.06;
                    if (elevatio > 1.5) elevatio = 1.5;
                    break;
                case SDLK_DOWN:
                    elevatio -= 0.06;
                    if (elevatio < -1.5) elevatio = -1.5;
                    break;
                case SDLK_w:
                    distantia -= 0.15;
                    if (distantia < 1.8) distantia = 1.8;
                    break;
                case SDLK_s:
                    distantia += 0.15;
                    if (distantia > 8.0) distantia = 8.0;
                    break;
                case SDLK_x:
                    axis_index = 0;
                    fprintf(stderr, "Axis: %s\n", nomina_axium[axis_index]);
                    break;
                case SDLK_y:
                    axis_index = 1;
                    fprintf(stderr, "Axis: %s\n", nomina_axium[axis_index]);
                    break;
                case SDLK_z:
                    axis_index = 2;
                    fprintf(stderr, "Axis: %s\n", nomina_axium[axis_index]);
                    break;
                case SDLK_SPACE:
                    rotatio_activa = !rotatio_activa;
                    fprintf(stderr, "Rotatio: %s\n",
                            rotatio_activa ? "activa" : "sistita");
                    break;
                case SDLK_EQUALS:
                case SDLK_PLUS:
                    celeritas *= 1.3;
                    if (celeritas > 6.0) celeritas = 6.0;
                    fprintf(stderr, "Celeritas: %.2f rad/s\n", celeritas);
                    break;
                case SDLK_MINUS:
                    celeritas /= 1.3;
                    if (celeritas < 0.05) celeritas = 0.05;
                    fprintf(stderr, "Celeritas: %.2f rad/s\n", celeritas);
                    break;
                case SDLK_1:
                    radius_maior -= 0.05;
                    if (radius_maior < 0.3) radius_maior = 0.3;
                    superficies_obsoleta = 1;
                    fprintf(stderr, "R=%.2f r=%.2f\n", radius_maior, radius_minor);
                    break;
                case SDLK_2:
                    radius_maior += 0.05;
                    if (radius_maior > 3.0) radius_maior = 3.0;
                    superficies_obsoleta = 1;
                    fprintf(stderr, "R=%.2f r=%.2f\n", radius_maior, radius_minor);
                    break;
                case SDLK_3:
                    radius_minor -= 0.02;
                    if (radius_minor < 0.08) radius_minor = 0.08;
                    superficies_obsoleta = 1;
                    fprintf(stderr, "R=%.2f r=%.2f\n", radius_maior, radius_minor);
                    break;
                case SDLK_4:
                    radius_minor += 0.02;
                    if (radius_minor > 1.5) radius_minor = 1.5;
                    superficies_obsoleta = 1;
                    fprintf(stderr, "R=%.2f r=%.2f\n", radius_maior, radius_minor);
                    break;
                case SDLK_TAB:
                    helvea_index_thematis = (helvea_index_thematis + 1) % helvea_numerus_thematum;
                    fprintf(stderr, "Thema [%d]: %s\n",
                            helvea_index_thematis, helvea_themata[helvea_index_thematis].nomen);
                    break;
                case SDLK_m:
                    methodus = (methodus + 1) % HELVEA_NUMERUS_METHODORUM;
                    superficies_obsoleta = 1;
                    fprintf(stderr, "Methodus [%d]: %s\n",
                            methodus, helvea_nomina_methodorum[methodus]);
                    break;
                case SDLK_r:
                    azimuthum = 0.6;
                    elevatio  = 0.4;
                    distantia = 3.2;
                    angulus_rot = 0.0;
                    celeritas  = 0.8;
                    axis_index = 2;
                    rotatio_activa = 1;
                    if (radius_maior != HELVEA_RADIUS_MAIOR ||
                        radius_minor != HELVEA_RADIUS_MINOR ||
                        methodus != HELVEA_CORRUGATA) {
                        radius_maior = HELVEA_RADIUS_MAIOR;
                        radius_minor = HELVEA_RADIUS_MINOR;
                        methodus = HELVEA_CORRUGATA;
                        superficies_obsoleta = 1;
                    }
                    fprintf(stderr, "Conspectus restitutus.\n");
                    break;
                default:
                    break;
                }
            }
        }

        const Uint8 *status_clavium = SDL_GetKeyboardState(NULL);
        if (status_clavium[SDL_SCANCODE_LEFT])  azimuthum -= 1.5 * dt;
        if (status_clavium[SDL_SCANCODE_RIGHT]) azimuthum += 1.5 * dt;
        if (status_clavium[SDL_SCANCODE_UP])    elevatio  += 1.0 * dt;
        if (status_clavium[SDL_SCANCODE_DOWN])  elevatio  -= 1.0 * dt;
        if (status_clavium[SDL_SCANCODE_W])     distantia -= 2.0 * dt;
        if (status_clavium[SDL_SCANCODE_S])     distantia += 2.0 * dt;

        if (elevatio >  1.5) elevatio =  1.5;
        if (elevatio < -1.5) elevatio = -1.5;
        if (distantia < 1.8) distantia = 1.8;
        if (distantia > 8.0) distantia = 8.0;

        if (rotatio_activa)
            angulus_rot += celeritas * dt;

        if (superficies_obsoleta) {
            helvea_superficiem_computare(puncta_orig, normae_orig,
                                         GRADUS_U, GRADUS_V,
                                         radius_maior, radius_minor, methodus);
            superficies_obsoleta = 0;
        }

        functio_rotandi rotare = rotationes[axis_index];
        for (size_t i = 0; i < n_vert; i++) {
            puncta_rot[i] = rotare(puncta_orig[i], angulus_rot);
            normae_rot[i] = rotare(normae_orig[i], angulus_rot);
        }

        Vec3 pos_cam = vec3(
            distantia * cos(elevatio) * cos(azimuthum),
            distantia * cos(elevatio) * sin(azimuthum),
            distantia * sin(elevatio));
        Vec3 scopus = vec3(0.0, 0.0, 0.0);
        Camera cam = helvea_cameram_constituere(pos_cam, scopus);

        helvea_scaenam_reddere(&tab, puncta_rot, normae_rot,
                               GRADUS_U, GRADUS_V, &cam,
                               helvea_illuminare_thema, helvea_pixel_bgra);

        pfx_applicare(&tab, helvea_themata[helvea_index_thematis].pfx,
                       helvea_themata[helvea_index_thematis].posteriza_niv);

        SDL_UpdateTexture(textura, NULL, tab.imaginis, LATITUDO_IMG * 4);
        SDL_RenderClear(pictor);
        SDL_RenderCopy(pictor, textura, NULL, NULL);
        SDL_RenderPresent(pictor);
    }

    free(puncta_orig);
    free(normae_orig);
    free(puncta_rot);
    free(normae_rot);
    free(tab.imaginis);
    free(tab.profunditatis);

    SDL_DestroyTexture(textura);
    SDL_DestroyRenderer(pictor);
    SDL_DestroyWindow(fenestra);
    SDL_Quit();

    fprintf(stderr, "Vale.\n");
    return 0;
}
