/*
 * perceptus.c — pipeline convolutionalis perceptionis planetarum
 */

#include "perceptus.h"
#include "tessera.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FEN PLANETA_FENESTRA

/* ================================================================
 * strepitus
 * ================================================================ */

static double nhash(int x, int y)
{
    unsigned int h = (unsigned int)(x * 1619 + y * 31337);
    h ^= h >> 16;
    h *= 0x45d9f3bu;
    h ^= h >> 16;
    return (h & 0xFFFFu) / 65535.0;
}

/* strepitus bilinearis — scala per cellulam */
static double strepitus_bilis(double fx, double fy)
{
    int ix     = (int)fx;
    int iy     = (int)fy;
    double tx  = fx - ix;
    double ty  = fy - iy;
    tx         = tx * tx * (3.0 - 2.0 * tx);   /* smoothstep */
    ty         = ty * ty * (3.0 - 2.0 * ty);
    double v00 = nhash(ix,   iy  ) - 0.5;
    double v10 = nhash(ix+1, iy  ) - 0.5;
    double v01 = nhash(ix,   iy+1) - 0.5;
    double v11 = nhash(ix+1, iy+1) - 0.5;
    return v00*(1-tx)*(1-ty) + v10*tx*(1-ty)
        + v01*(1-tx)*ty     + v11*tx*ty;
}

/* ================================================================
 * auxiliaria
 * ================================================================ */

static inline unsigned char uc_clamp(double v)
{
    if (v < 0.0)
        return 0;
    if (v > 255.0)
        return 255;
    return (unsigned char)v;
}

/* ================================================================
 * parser
 * ================================================================ */

planeta_perceptus_t planeta_perceptus_ex_ison(const char *ison)
{
    planeta_perceptus_t p;
    p.aspectus         = planeta_aspectus_ex_ison(NULL); /* situs=0, lumen=1 */
    p.coaspectus       = planeta_aspectus_ex_ison(NULL);
    p.coaspectus.lumen = 0.0;                           /* inactivus nisi datus */
    p.acuitas   = 0.0;
    p.detallum  = 0.0;
    p.granum    = 0.0;
    p.aberratio = 0.0;
    if (!ison)
        return p;

    char *asp_s = ison_da_crudum(ison, "aspectus");
    if (asp_s) {
        p.aspectus = planeta_aspectus_ex_ison(asp_s);
        free(asp_s);
    }

    char *co_s = ison_da_crudum(ison, "coaspectus");
    if (co_s)  {
        p.coaspectus = planeta_aspectus_ex_ison(co_s);
        free(co_s);
    }

    p.acuitas   = ison_da_f(ison, "acuitas",   0.0);
    p.detallum  = ison_da_f(ison, "detallum",  0.0);
    p.granum    = ison_da_f(ison, "granum",    0.0);
    p.aberratio = ison_da_f(ison, "aberratio", 0.0);
    return p;
}

/* ================================================================
 * pipeline
 * ================================================================ */

void planeta_perceptum_applicare(
    unsigned char *fen,
    const planeta_perceptus_t *p
) {
    if (!p)
        return;
    if (
        p->acuitas < 0.001 && p->detallum < 0.001
        && p->granum  < 0.001 && p->aberratio < 0.001
    ) return;

    unsigned char *orig = (unsigned char *)malloc((size_t)FEN * FEN * 4);
    if (!orig)
        return;
    memcpy(orig, fen, (size_t)FEN * FEN * 4);

    /* 1. acuitas — Laplacian sharpening (unsharp mask, nucleum 3x3)
     *    vicini obscuri (alpha=0 vel umbra) valorem centri accipiunt
     *    ne frangia chromatica ad terminum umbrae oriatur */
    if (p->acuitas > 0.001) {
        for (int y = 1; y < FEN - 1; y++) {
            for (int x = 1; x < FEN - 1; x++) {
                int i = (y * FEN + x) * 4;
                if (orig[i + 3] == 0)
                    continue;
                for (int c = 0; c < 3; c++) {
                    int cv = orig[i + c];
                    /* vicinus: si transparens vel umbra (0,0,0), pro centro substituitur */
#define VIC(oy, ox) (orig[((y+(oy))*FEN+(x+(ox)))*4+3] == 0 \
                  || (orig[((y+(oy))*FEN+(x+(ox)))*4+0] == 0 \
                   && orig[((y+(oy))*FEN+(x+(ox)))*4+1] == 0 \
                   && orig[((y+(oy))*FEN+(x+(ox)))*4+2] == 0) \
                     ? cv : orig[((y+(oy))*FEN+(x+(ox)))*4+c])
                    int lap = 4 * cv - VIC(-1, 0) - VIC(1, 0) - VIC(0, -1) - VIC(0, 1);
#undef VIC
                    fen[i + c] = uc_clamp(cv + p->acuitas * lap);
                }
            }
        }
    }

    /* 2. detallum — synthesia texturarum per strepitum bilineare
     *    scala duplex: fine (4px) + medium (20px), multiplicativa */
    if (p->detallum > 0.001) {
        for (int y = 0; y < FEN; y++) {
            for (int x = 0; x < FEN; x++) {
                int i = (y * FEN + x) * 4;
                if (fen[i + 3] == 0)
                    continue;
                double nf     = strepitus_bilis(x / 4.0,  y / 4.0);
                double nm     = strepitus_bilis(x / 20.0 + 100.0, y / 20.0 + 100.0);
                double n      = (nf * 0.6 + nm * 0.4) * p->detallum;
                double factor = 1.0 + n * 0.45;
                for (int c = 0; c < 3; c++)
                    fen[i + c] = uc_clamp(fen[i + c] * factor);
            }
        }
    }

    /* 3. granum — strepitus per pixelem (granum pelliculae) */
    if (p->granum > 0.001) {
        for (int y = 0; y < FEN; y++) {
            for (int x = 0; x < FEN; x++) {
                int i = (y * FEN + x) * 4;
                if (fen[i + 3] == 0)
                    continue;
                double n = (nhash(x + 9973, y + 7919) - 0.5) * p->granum * 28.0;
                for (int c = 0; c < 3; c++)
                    fen[i + c] = uc_clamp(fen[i + c] + n);
            }
        }
    }

    /* 4. aberratio — aberratio chromatica lateralis:
     *    canalis R sinistrorsum, B dextrorsum */
    if (p->aberratio > 0.001) {
        int shift = (int)(p->aberratio * 4.0 + 0.5);
        if (shift > 0) {
            memcpy(orig, fen, (size_t)FEN * FEN * 4);
            for (int y = 0; y < FEN; y++) {
                for (int x = 0; x < FEN; x++) {
                    int i = (y * FEN + x) * 4;
                    if (orig[i + 3] == 0)
                        continue;
                    int xr = x - shift;
                    if (xr >= 0 && orig[(y*FEN+xr)*4+3])
                        fen[i + 0] = orig[(y*FEN+xr)*4 + 0];
                    int xb = x + shift;
                    if (xb < FEN && orig[(y*FEN+xb)*4+3])
                        fen[i + 2] = orig[(y*FEN+xb)*4 + 2];
                }
            }
        }
    }

    free(orig);
}

void planeta_perceptus_in_ison(FILE *f, const planeta_perceptus_t *p)
{
    fprintf(f, "{\"aspectus\": ");
    planeta_aspectus_in_ison(f, &p->aspectus);
    if (p->coaspectus.lumen > 0.001) {
        fprintf(f, ", \"coaspectus\": ");
        planeta_aspectus_in_ison(f, &p->coaspectus);
    }
    fprintf(
        f, ", \"acuitas\": %.1f, \"detallum\": %.2f, \"granum\": %.2f",
        p->acuitas, p->detallum, p->granum
    );
    if (p->aberratio > 0.001)
        fprintf(f, ", \"aberratio\": %.2f", p->aberratio);
    fprintf(f, "}");
}
