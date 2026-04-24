/*
 * colloquium.c — fenestra dialogi cum oraculo
 *
 * Fenestram aperit cum stellis in fundo (terra.ison), velo
 * rotundato super stellas, imagine personae in porta rotunda,
 * responso oraculi, et arca scriptionis infra.
 *
 * Usus: ./colloquium
 */

#include "formula.h"
#include "caela.h"
#include "campus.h"
#include "instrumentum.h"

#include <phantasma/phantasma.h>
#include <delphi/oraculum.h>
#include <proplasma/artista.h>
#include <ison/ison.h>

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ================================================================
 * dimensiones
 * ================================================================ */

#define FEN_LAT         1024
#define FEN_ALT         1024

#define PORTA_LAT       760
#define PORTA_ALT       860
#define PORTA_RADIUS    64

#define IMAGO_LAT       256
#define IMAGO_ALT       256

#define ATLAS_LAT       512
#define ATLAS_ALT       768
#define GLYPH_LAT_N     32
#define GLYPH_ALT_N     48
#define FONT_SCALA      4                   /* 16/4 = 4 blk; glyph = 8x12 */
#define GL              (GLYPH_LAT_N * FONT_SCALA / 16)
#define GA              (GLYPH_ALT_N * FONT_SCALA / 16)

#define ROGATUM_MAX     512
#define RESPONSUM_PRIMUM "Salve! Quomodo te habes hodie?"

/* ================================================================
 * status globalis
 * ================================================================ */

static uint32_t  framebuffer[FEN_LAT * FEN_ALT];
static uint32_t  stellae_bg[FEN_LAT * FEN_ALT];
static uint32_t  atlas[ATLAS_LAT * ATLAS_ALT];
static uint32_t  glyphi[256][GL * GA];
static uint8_t   glyph_opac[256][GL * GA];
static uint8_t   imago_rgba[IMAGO_LAT * IMAGO_ALT * 4];

static char      rogatum[ROGATUM_MAX];
static int       rogatum_len = 0;
static int       expectans   = 0;

static oraculum_sessio_t *sessio = NULL;
static pthread_t          sessio_thread;
static pthread_mutex_t    sessio_mutex     = PTHREAD_MUTEX_INITIALIZER;
static char              *sessio_rogatum   = NULL;
static char              *sessio_responsum = NULL;
static int                sessio_rc        = 0;
static volatile int       sessio_paratum   = 0;

static char     *historia     = NULL;
static size_t    historia_len = 0;
static size_t    historia_cap = 0;

static int       clip_y_min = 0;
static int       clip_y_max = FEN_ALT;

static void *sessio_currere(void *arg)
{
    (void)arg;
    char *resp = NULL;
    int rc     = oraculum_sessio_roga(sessio, sessio_rogatum, &resp);
    pthread_mutex_lock(&sessio_mutex);
    sessio_responsum = resp;
    sessio_rc        = rc;
    sessio_paratum   = 1;
    pthread_mutex_unlock(&sessio_mutex);
    return NULL;
}

static void historia_adde(const char *s)
{
    size_t n = strlen(s);
    if (historia_len + n + 1 >= historia_cap) {
        historia_cap = (historia_len + n + 2) * 2;
        historia     = realloc(historia, historia_cap);
    }
    memcpy(historia + historia_len, s, n);
    historia_len += n;
    historia[historia_len] = '\0';
}

/* ================================================================
 * font glyphi — ex xiterm titulus_16.gif
 * ================================================================ */

static void glyphos_praecalcula(void)
{
    int blk  = 16 / FONT_SCALA;
    int blk2 = blk * blk;

    for (int ch = 0; ch < 256; ch++) {
        uint32_t *g = glyphi[ch];
        uint8_t  *o = glyph_opac[ch];
        int bx0     = (ch % 16) * GLYPH_LAT_N;
        int by0     = (ch / 16) * GLYPH_ALT_N;

        for (int oy = 0; oy < GA; oy++) {
            for (int ox = 0; ox < GL; ox++) {
                uint32_t sr = 0, sg = 0, sb = 0;
                int n_opac  = 0;
                for (int by = 0; by < blk; by++) {
                    for (int bx = 0; bx < blk; bx++) {
                        int ax     = bx0 + ox * blk + bx;
                        int ay     = by0 + oy * blk + by;
                        uint32_t p = atlas[ay * ATLAS_LAT + ax];
                        if ((p >> 24) != 0 && (p & 0x00FFFFFF) != 0) {
                            sr += (p >> 16) & 0xFF;
                            sg += (p >> 8) & 0xFF;
                            sb += p & 0xFF;
                            n_opac++;
                        }
                    }
                }
                int idx = oy * GL + ox;
                if (n_opac * 2 > blk2) {
                    g[idx] = 0xFF000000
                        | ((sr / n_opac) << 16)
                        | ((sg / n_opac) << 8)
                        | (sb / n_opac);
                    o[idx] = (uint8_t)(255 * n_opac / blk2);
                } else {
                    g[idx] = 0;
                    o[idx] = 0;
                }
            }
        }
    }
}

static int fontem_onera(const char *via)
{
    pfr_gif_lector_t *l = pfr_gif_lege_initia(via);
    if (!l)
        return -1;
    int lat, alt;
    if (
        pfr_gif_lege_dimensiones(l, &lat, &alt) != 0 ||
        lat != ATLAS_LAT || alt != ATLAS_ALT
    ) {
        pfr_gif_lege_fini(l);
        return -1;
    }
    if (pfr_gif_lege_tabulam(l, atlas) != 0) {
        pfr_gif_lege_fini(l);
        return -1;
    }
    pfr_gif_lege_fini(l);
    glyphos_praecalcula();
    return 0;
}

/* ================================================================
 * compositio
 * ================================================================ */

static inline void pixel_pone(int x, int y, uint32_t argb)
{
    if (x < 0 || x >= FEN_LAT || y < 0 || y >= FEN_ALT)
        return;
    framebuffer[y * FEN_LAT + x] = argb;
}

static inline uint32_t misce(uint32_t a, uint32_t b, int alpha)
{
    /* b super a cum alpha 0..255 */
    int ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
    int br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
    int r  = ar + ((br - ar) * alpha) / 255;
    int g  = ag + ((bg - ag) * alpha) / 255;
    int bv = ab + ((bb - ab) * alpha) / 255;
    return 0xFF000000 | (r << 16) | (g << 8) | bv;
}

/* scribe unum characterem cum colore dato.  glyph alpha blended */
static void character_scribe(int x, int y, int ch, uint32_t color)
{
    uint8_t *o = glyph_opac[ch & 0xFF];
    for (int gy = 0; gy < GA; gy++) {
        int yy = y + gy;
        if (yy < clip_y_min || yy >= clip_y_max)
            continue;
        if (yy < 0 || yy >= FEN_ALT)
            continue;
        for (int gx = 0; gx < GL; gx++) {
            int a = o[gy * GL + gx];
            if (a == 0)
                continue;
            int xx = x + gx;
            if (xx < 0 || xx >= FEN_LAT)
                continue;
            uint32_t bg = framebuffer[yy * FEN_LAT + xx];
            framebuffer[yy * FEN_LAT + xx] = misce(bg, color, a);
        }
    }
}

/* scribe chordam cum involutione verborum, reddit y ultimae lineae */
static int chordam_scribe(
    int x0, int y0, int lat_max, const char *s, uint32_t color
) {
    int cx = x0, cy = y0;
    while (*s) {
        if (*s == '\n') {
            cx = x0;
            cy += GA + 2;
            s++;
            continue;
        }
        /* verbum proximum mensura */
        const char *w = s;
        while (*w && *w != ' ' && *w != '\n')
            w++;
        int verb_lat = (int)(w - s) * GL;
        if (cx + verb_lat > x0 + lat_max && cx > x0) {
            cx = x0;
            cy += GA + 2;
        }
        while (s < w) {
            if (cx + GL > x0 + lat_max) {
                cx = x0;
                cy += GA + 2;
            }
            character_scribe(cx, cy, (unsigned char)*s, color);
            cx += GL;
            s++;
        }
        if (*s == ' ') {
            if (cx + GL > x0 + lat_max) {
                cx = x0;
                cy += GA + 2;
            } else {
                character_scribe(cx, cy, ' ', color);
                cx += GL;
            }
            s++;
        }
    }
    return cy + GA;
}

/* rectangulum rotundum imple cum argb */
static void rectum_rotundum_imple(
    int x0, int y0, int lat, int alt, int rad, uint32_t color, int alpha
) {
    for (int y = y0; y < y0 + alt; y++) {
        if (y < 0 || y >= FEN_ALT)
            continue;
        for (int x = x0; x < x0 + lat; x++) {
            if (x < 0 || x >= FEN_LAT)
                continue;
            int dx = 0, dy = 0;
            if (x < x0 + rad)
                dx = (x0 + rad) - x;
            else if (x >= x0 + lat - rad)
                dx = x - (x0 + lat - rad - 1);
            if (y < y0 + rad)
                dy = (y0 + rad) - y;
            else if (y >= y0 + alt - rad)
                dy = y - (y0 + alt - rad - 1);
            if (dx == 0 && dy == 0) {
                uint32_t bg = framebuffer[y * FEN_LAT + x];
                framebuffer[y * FEN_LAT + x] = misce(bg, color, alpha);
                continue;
            }
            double d = sqrt((double)(dx * dx + dy * dy));
            if (d > rad)
                continue;
            int a = alpha;
            if (d > rad - 1)
                a = (int)(alpha * (rad - d));
            if (a <= 0)
                continue;
            uint32_t bg = framebuffer[y * FEN_LAT + x];
            framebuffer[y * FEN_LAT + x] = misce(bg, color, a);
        }
    }
}

/* linea rotunda (margo) */
static void rectum_rotundum_margo(
    int x0, int y0, int lat, int alt, int rad, uint32_t color
) {
    int xi = x0 + rad, xj = x0 + lat - rad - 1;
    int yi = y0 + rad, yj = y0 + alt - rad - 1;
    for (int t = 0; t < 360 * 4; t++) {
        double a = t * M_PI / 720.0;
        double cx, cy;
        if (a < M_PI / 2) {
            cx = xj + cos(a) * rad;
            cy = yi - sin(a) * rad;
        } else if (a < M_PI) {
            cx = xi + cos(a) * rad;
            cy = yi - sin(a) * rad;
        } else if (a < 3 * M_PI / 2) {
            cx = xi + cos(a) * rad;
            cy = yj - sin(a) * rad;
        } else {
            cx = xj + cos(a) * rad;
            cy = yj - sin(a) * rad;
        }
        pixel_pone((int)cx, (int)cy, color);
    }
    for (int x = xi; x <= xj; x++) {
        pixel_pone(x, y0, color);
        pixel_pone(x, y0 + alt - 1, color);
    }
    for (int y = yi; y <= yj; y++) {
        pixel_pone(x0, y, color);
        pixel_pone(x0 + lat - 1, y, color);
    }
}

/* imagine personae pinge in circulo */
static void portam_imagine_pinge(int cx, int cy, int radius)
{
    int size = radius * 2;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int dx = x - radius, dy = y - radius;
            int d2 = dx * dx + dy * dy;
            if (d2 > radius * radius)
                continue;
            /* scala ex imago_rgba (128x128) ad size */
            int sx = x * IMAGO_LAT / size;
            int sy = y * IMAGO_ALT / size;
            if (sx >= IMAGO_LAT)
                sx = IMAGO_LAT - 1;
            if (sy >= IMAGO_ALT)
                sy = IMAGO_ALT - 1;
            int si = (sy * IMAGO_LAT + sx) * 4;
            uint32_t argb = 0xFF000000
                | ((uint32_t)imago_rgba[si + 0] << 16)
                | ((uint32_t)imago_rgba[si + 1] << 8)
                | ((uint32_t)imago_rgba[si + 2]);
            pixel_pone(cx - radius + x, cy - radius + y, argb);
        }
    }
    /* margo aureus */
    for (int t = 0; t < 720; t++) {
        double a = t * M_PI / 360.0;
        int x    = cx + (int)(cos(a) * radius);
        int y    = cy + (int)(sin(a) * radius);
        pixel_pone(x, y, 0xFFFFD070);
        pixel_pone(x + 1, y, 0xFFFFD070);
    }
}

/* spinner: circulus punctorum rotans */
static void spinner_pinge(int cx, int cy, int radius, double t)
{
    int n = 8;
    for (int i = 0; i < n; i++) {
        double a = t * 4.0 + i * 2 * M_PI / n;
        int x = cx + (int)(cos(a) * radius);
        int y = cy + (int)(sin(a) * radius);
        int fade = 255 - (i * 200 / n);
        uint32_t color = (uint32_t)fade;
        color = 0xFF000000 | (color << 16) | (color << 8) | color;
        for (int dy = -2; dy <= 2; dy++)
            for (int dx = -2; dx <= 2; dx++)
                if (dx * dx + dy * dy <= 4)
                    pixel_pone(x + dx, y + dy, color);
    }
}

/* ================================================================
 * stellae — generatio fundi
 * ================================================================ */

static int stellae_genera(void)
{
    char *form_ison = ison_lege_plicam("formulae/terra.ison");
    if (!form_ison) {
        fprintf(stderr, "ERROR: formulae/terra.ison legere non possum\n");
        return -1;
    }
    formula_t form;
    formula_ex_ison(&form, form_ison);
    free(form_ison);

    caela_t *caela = caela_ex_formula(&form, 150);
    if (!caela) {
        formula_purgare(&form);
        return -1;
    }
    caela_orbitas_applicare(caela, &form, 150);
    formula_purgare(&form);

    char *inst_ison = ison_lege_plicam("instrumenta/jwst.ison");
    instrumentum_t inst;
    memset(&inst, 0, sizeof(inst));
    if (inst_ison) {
        instrumentum_ex_ison(&inst, inst_ison);
        free(inst_ison);
    }

    campus_t *c = campus_ex_caela(caela, &inst);
    caela_destruere(caela);
    if (!c)
        return -1;

    /* downsample cum mediatione capsae in stellae_bg */
    for (int y = 0; y < FEN_ALT; y++) {
        int sy0 = y * c->altitudo / FEN_ALT;
        int sy1 = (y + 1) * c->altitudo / FEN_ALT;
        if (sy1 <= sy0)
            sy1 = sy0 + 1;
        for (int x = 0; x < FEN_LAT; x++) {
            int sx0 = x * c->latitudo / FEN_LAT;
            int sx1 = (x + 1) * c->latitudo / FEN_LAT;
            if (sx1 <= sx0)
                sx1 = sx0 + 1;
            uint32_t sr = 0, sg = 0, sb = 0;
            int n       = 0;
            for (int sy = sy0; sy < sy1; sy++) {
                for (int sx = sx0; sx < sx1; sx++) {
                    int si = (sy * c->latitudo + sx) * 3;
                    sr += c->pixels[si + 0];
                    sg += c->pixels[si + 1];
                    sb += c->pixels[si + 2];
                    n++;
                }
            }
            uint32_t r = sr / n, g = sg / n, b = sb / n;
            stellae_bg[y * FEN_LAT + x] = 0xFF000000
                | (r << 16) | (g << 8) | b;
        }
    }
    campus_destruere(c);
    return 0;
}

/* ================================================================
 * imago personae
 * ================================================================ */

/* scribe GIF bytes ad plicam temporariam, lege cum pfr_gif, unlinca */
static int gif_bytes_ad_tabulam(
    const unsigned char *bytes, size_t n, uint32_t *out, int lat, int alt
) {
    char via[] = "/tmp/colloquium_imago_XXXXXX.gif";
    int fd = mkstemps(via, 4);
    if (fd < 0)
        return -1;
    ssize_t w = 0;
    while ((size_t)w < n) {
        ssize_t k = write(fd, bytes + w, n - w);
        if (k <= 0) { close(fd); unlink(via); return -1; }
        w += k;
    }
    close(fd);
    pfr_gif_lector_t *l = pfr_gif_lege_initia(via);
    if (!l) { unlink(via); return -1; }
    int glat = 0, galt = 0;
    if (pfr_gif_lege_dimensiones(l, &glat, &galt) != 0) {
        pfr_gif_lege_fini(l);
        unlink(via);
        return -1;
    }
    uint32_t *tmp = malloc((size_t)glat * galt * sizeof(uint32_t));
    if (!tmp) {
        pfr_gif_lege_fini(l);
        unlink(via);
        return -1;
    }
    int rc = pfr_gif_lege_tabulam(l, tmp);
    pfr_gif_lege_fini(l);
    unlink(via);
    if (rc != 0) { free(tmp); return -1; }
    /* scala glat x galt ad lat x alt */
    for (int y = 0; y < alt; y++) {
        int sy = y * galt / alt;
        for (int x = 0; x < lat; x++) {
            int sx = x * glat / lat;
            out[y * lat + x] = tmp[sy * glat + sx];
        }
    }
    free(tmp);
    return 0;
}

static void imago_genera(uint64_t semen)
{
    memset(imago_rgba, 0, sizeof(imago_rgba));

    char err[256];
    char *rogatum_img = artista_generare(semen, NULL, err, sizeof(err));
    if (!rogatum_img) {
        fprintf(stderr, "MONITUM: artista_generare defecit: %s\n", err);
        return;
    }

    unsigned char *bytes = NULL;
    size_t n             = 0;
    int rc = oraculum_imago_roga(
        "openai/gpt-5.4", rogatum_img, IMAGO_LAT, &bytes, &n
    );
    free(rogatum_img);
    if (rc != 0 || !bytes || n == 0) {
        fprintf(stderr, "MONITUM: oraculum_imago_roga defecit\n");
        free(bytes);
        return;
    }

    uint32_t *argb = malloc(IMAGO_LAT * IMAGO_ALT * sizeof(uint32_t));
    if (!argb) { free(bytes); return; }
    if (gif_bytes_ad_tabulam(bytes, n, argb, IMAGO_LAT, IMAGO_ALT) != 0) {
        free(argb);
        free(bytes);
        return;
    }
    free(bytes);

    for (int y = 0; y < IMAGO_ALT; y++) {
        for (int x = 0; x < IMAGO_LAT; x++) {
            uint32_t p = argb[y * IMAGO_LAT + x];
            int di = (y * IMAGO_LAT + x) * 4;
            imago_rgba[di + 0] = (p >> 16) & 0xFF;
            imago_rgba[di + 1] = (p >> 8)  & 0xFF;
            imago_rgba[di + 2] = p         & 0xFF;
            imago_rgba[di + 3] = 0xFF;
        }
    }
    free(argb);
}

/* ================================================================
 * redditio fenestrae
 * ================================================================ */

static void redde_omnia(double t)
{
    /* fundum: stellae */
    memcpy(framebuffer, stellae_bg, sizeof(framebuffer));

    /* velum rotundatum semi-obscurum */
    int px = (FEN_LAT - PORTA_LAT) / 2;
    int py = (FEN_ALT - PORTA_ALT) / 2;
    rectum_rotundum_imple(
        px, py, PORTA_LAT, PORTA_ALT, PORTA_RADIUS,
        0xFF0A1020, 190
    );
    rectum_rotundum_margo(
        px, py, PORTA_LAT, PORTA_ALT, PORTA_RADIUS, 0xFF4A6890
    );

    /* imago personae (porta rotunda) */
    int imago_cx = FEN_LAT / 2;
    int imago_cy = py + 24 + 90;
    portam_imagine_pinge(imago_cx, imago_cy, 90);

    /* historia colloquii — area scrollens sub imagine */
    int hist_y0 = imago_cy + 90 + 24;
    int txt_x0  = px + 40;
    int txt_lat = PORTA_LAT - 80;
    int hist_y1 = py + PORTA_ALT - GA - 40 - 16;  /* super arcam rogati */

    /* mensura: altitudo totalis historiae cum involutione */
    clip_y_min = -100000;
    clip_y_max = -50000;      /* totum extra clip — nulla scriptio */
    int total_end = chordam_scribe(
        txt_x0, 0, txt_lat, historia ? historia : "", 0xFFE0E8FF
    );
    int total_h = total_end;

    /* y initialis ut ultima linea ad hist_y1 consistat */
    int y_start = hist_y0;
    if (total_h > (hist_y1 - hist_y0))
        y_start = hist_y1 - total_h;

    clip_y_min = hist_y0;
    clip_y_max = hist_y1;
    chordam_scribe(
        txt_x0, y_start, txt_lat,
        historia ? historia : "", 0xFFE0E8FF
    );
    clip_y_min = 0;
    clip_y_max = FEN_ALT;

    /* arca rogati infra */
    int arc_y   = py + PORTA_ALT - GA - 40;
    int arc_x   = px + 32;
    int arc_lat = PORTA_LAT - 64;
    int arc_alt = GA + 16;
    rectum_rotundum_imple(
        arc_x, arc_y, arc_lat, arc_alt, 12, 0xFF1A2840, 220
    );
    rectum_rotundum_margo(
        arc_x, arc_y, arc_lat, arc_alt, 12, 0xFF6080A0
    );

    /* rogatum inscribe */
    int rog_x = arc_x + 10;
    int rog_y = arc_y + 8;
    chordam_scribe(
        rog_x, rog_y, arc_lat - 20, rogatum, 0xFFFFFFFF
    );

    /* cursor in arca rogati */
    int cursor_x = rog_x + (rogatum_len * GL) % (arc_lat - 20);
    if (((int)(t * 2)) & 1) {
        for (int dy = 0; dy < GA; dy++)
            pixel_pone(cursor_x, rog_y + dy, 0xFFFFFFFF);
    }

    /* spinner si expectans */
    if (expectans) {
        spinner_pinge(imago_cx, imago_cy, 110, t);
    }
}

/* ================================================================
 * principale
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    srand((unsigned)time(NULL));

    fprintf(stderr, "Fontem onerans...\n");
    if (fontem_onera("/opt/apotheca/var/xiterm/titulus_16.gif") != 0) {
        fprintf(stderr, "ERROR: fons onerari non potest\n");
        return 1;
    }

    fprintf(stderr, "Stellas generans (mora expectata)...\n");
    if (stellae_genera() != 0) {
        fprintf(stderr, "ERROR: stellae generari non possunt\n");
        return 1;
    }

    fprintf(stderr, "Imaginem personae generans...\n");
    uint64_t semen = ((uint64_t)rand() << 32) ^ (uint64_t)rand();
    imago_genera(semen);

    historia_adde(RESPONSUM_PRIMUM);
    historia_adde("\n\n");

    fprintf(stderr, "Oraculum initians...\n");
    if (oraculum_initia() < 0) {
        fprintf(stderr, "MONITUM: oraculum_initia defecit — sine oraculo\n");
    } else {
        sessio = oraculum_sessio_novam(
            "openai/gpt-5.4",
            "Es persona in colloquio humano. Responde breviter."
        );
        if (!sessio)
            fprintf(stderr, "MONITUM: sessio creari non potest\n");
    }

    if (pfr_initia(PFR_INITIA_VIDEO) != 0) {
        fprintf(stderr, "ERROR: phantasma: %s\n", pfr_erratum());
        return 1;
    }

    pfr_fenestra_t *fen = pfr_fenestram_crea(
        "colloquium", PFR_POS_MEDIUM, PFR_POS_MEDIUM,
        FEN_LAT, FEN_ALT, 0
    );
    pfr_pictor_t *pic = pfr_pictorem_crea(
        fen, -1, PFR_PICTOR_CELER | PFR_PICTOR_SYNC
    );
    pfr_textura_t *tex = pfr_texturam_crea(
        pic, PFR_PIXEL_ARGB8888, PFR_TEXTURA_FLUENS, FEN_LAT, FEN_ALT
    );

    int currens = 1;
    uint32_t t0 = pfr_tempus();

    while (currens) {
        pfr_eventus_t ev;
        while (pfr_eventum_lege(&ev)) {
            if (ev.typus == PFR_EXITUS) {
                currens = 0;
                break;
            }
            if (ev.typus == PFR_CLAVIS_INF) {
                int sym = ev.clavis.signum.symbolum;
                if (sym == PFR_CL_EFFUGIUM) {
                    currens = 0;
                    break;
                } else if (sym == '\r' || sym == '\n') {
                    if (rogatum_len > 0 && !expectans && sessio) {
                        historia_adde("> ");
                        historia_adde(rogatum);
                        historia_adde("\n");
                        free(sessio_rogatum);
                        sessio_rogatum   = strdup(rogatum);
                        sessio_responsum = NULL;
                        sessio_paratum   = 0;
                        expectans        = 1;
                        if (
                            pthread_create(
                                &sessio_thread, NULL,
                                sessio_currere, NULL
                            ) != 0
                        ) {
                            historia_adde("[thread defecit]\n\n");
                            expectans = 0;
                        } else {
                            pthread_detach(sessio_thread);
                        }
                        rogatum_len = 0;
                        rogatum[0]  = '\0';
                    }
                } else if (sym == 0x7F || sym == '\b') {
                    if (rogatum_len > 0) {
                        rogatum_len--;
                        rogatum[rogatum_len] = '\0';
                    }
                } else if (sym >= 0x20 && sym < 0x7F) {
                    if (rogatum_len < ROGATUM_MAX - 1) {
                        rogatum[rogatum_len++] = (char)sym;
                        rogatum[rogatum_len]   = '\0';
                    }
                }
            }
        }
        if (!currens)
            break;

        /* sessio: proba an thread perfecta est */
        if (expectans) {
            pthread_mutex_lock(&sessio_mutex);
            int paratum = sessio_paratum;
            int rc      = sessio_rc;
            char *resp  = sessio_responsum;
            if (paratum) {
                sessio_responsum = NULL;
                sessio_paratum   = 0;
            }
            pthread_mutex_unlock(&sessio_mutex);
            if (paratum) {
                if (rc == 0 && resp) {
                    historia_adde(resp);
                    historia_adde("\n\n");
                } else {
                    historia_adde(resp ? resp : "[erratum]");
                    historia_adde("\n\n");
                }
                free(resp);
                free(sessio_rogatum);
                sessio_rogatum = NULL;
                expectans      = 0;
            }
        }

        double t = (pfr_tempus() - t0) / 1000.0;
        redde_omnia(t);
        /* phantasma (macOS) y-invertit; inverte hic */
        static uint32_t fb_inv[FEN_LAT * FEN_ALT];
        for (int y = 0; y < FEN_ALT; y++)
            memcpy(
                fb_inv + y * FEN_LAT,
                framebuffer + (FEN_ALT - 1 - y) * FEN_LAT,
                FEN_LAT * 4
            );
        pfr_texturam_renova(tex, NULL, fb_inv, FEN_LAT * 4);
        pfr_purga(pic);
        pfr_texturam_pinge(pic, tex, NULL, NULL);
        pfr_praesenta(pic);
        pfr_pausa(16);
    }

    if (sessio)
        oraculum_sessio_dimitte(sessio);
    oraculum_fini();
    pfr_texturam_destrue(tex);
    pfr_pictorem_destrue(pic);
    pfr_fenestram_destrue(fen);
    pfr_fini();
    free(sessio_rogatum);
    free(sessio_responsum);
    free(historia);
    return 0;
}
