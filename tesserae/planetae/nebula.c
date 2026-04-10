/* nebula.c — renderer nebulae (included from planeta.c) */

static void reddere_nebula(unsigned char *fen, const nebula_t *p)
{
    double ox = (double)( p->pro.semen        & 0xFF) * 0.37;
    double oy = (double)((p->pro.semen >>  8) & 0xFF) * 0.37;
    double oz = (double)((p->pro.semen >> 16) & 0xFF) * 0.37;

    double rad       = p->pro.radius * SEMI;
    double lum       = (p->res.luminositas > 0.001) ? p->res.luminositas : 1.0;
    double turb      = (p->res.tectonica  > 0.0)   ? p->res.tectonica   : 0.5;
    double fil       = (p->res.nubes      > 0.0)   ? p->res.nubes       : 0.4;
    double dark_frac = p->res.carbo;

    double t_K   = (p->res.temperatura > 100.0) ? p->res.temperatura : 5000.0;
    color_t base = temperatura_ad_colorem_f(t_K);

    double ha   = p->res.h2;   /* H-alpha: λ=656nm, rubrum */
    double oiii = p->res.o2;   /* OIII:    λ=501nm, cyaneum */

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double dx = (px - SEMI) / rad;
            double dy = (py - SEMI) / rad;
            double r2 = dx * dx + dy * dy;
            if (r2 > 5.0)
                continue;

            /* forma: terminus angulariter modulatus — non circulus */
            double phi     = atan2(dy, dx);
            double shape   = fbm(cos(phi) * 1.2 + ox, sin(phi) * 1.2 + oy, 3, 0.6);
            double local_r = 0.7 + shape * 0.6;   /* [0.7, 1.3] */
            double d_norm  = sqrt(r2) / local_r;
            if (d_norm > 1.8)
                continue;

            double involucrum = exp(-d_norm * d_norm * 1.8);

            /* densitas gasei ex fbm_warp */
            double density = fbm_warp(
                dx * 2.5 + ox, dy * 2.5 + oy,
                5, 0.58, turb * 1.5
            );
            /* filamenta ex strepitu cresto */
            double filament = ridged(dx * 3.8 + ox + 5.3, dy * 3.8 + oy + 3.7, 4, 0.55);
            density         = density * (1.0 - fil * 0.6) + filament * fil * 0.6;
            density         = fmax(0.0, fmin(1.0, density)) * involucrum;

            /* nubecula absorbens — tenebra stellaris */
            if (dark_frac > 0.001) {
                double dark = fbm_warp(
                    dx * 2.6 + oz, dy * 2.6 + oz + 11.1,
                    4, 0.65, turb * 0.7
                );
                dark *= exp(-d_norm * d_norm * 2.5);
                double limen = 1.0 - dark_frac;
                if (dark > limen) {
                    double prof     = (dark - limen) / fmax(0.01, dark_frac);
                    double opacitas = fmin(1.0, prof * prof * lum * 3.0);
                    if (opacitas > 0.05) {
                        int fi      = (py * FEN + px) * 4;
                        fen[fi + 0] = 0;
                        fen[fi + 1] = 0;
                        fen[fi + 2] = 0;
                        fen[fi + 3] = (unsigned char)(opacitas * 255.0);
                        continue;
                    }
                }
            }

            /* mensura emissionis: densitas × luminositas
             * alpha et fulgor ex eodem — EM ∝ n²·dl (optice tenue) */
            double em = density * lum;
            if (em < 0.005)
                continue;

            double alpha  = fmin(1.0, em);
            double fulgor = fmin(1.0, em * (1.0 + em * 0.4));  /* superlinearis */

            double var = fbm(dx * 5.0 + ox + 3.1, dy * 5.0 + oy + 7.9, 2, 0.5);

            /* color: Planck + lineae emissionis additivae */
            double r = base.r * fulgor;
            double g = base.g * fulgor;
            double b = base.b * fulgor;

            r += ha * (0.90 + var * 0.10) * fulgor;
            g += ha * 0.04 * fulgor;
            b += ha * 0.01 * fulgor;

            r += oiii * 0.02 * fulgor;
            g += oiii * (0.75 + var * 0.08) * fulgor;
            b += oiii * 0.95 * fulgor;

            r = fmin(1.0, fmax(0.0, r));
            g = fmin(1.0, fmax(0.0, g));
            b = fmin(1.0, fmax(0.0, b));

            int fi      = (py * FEN + px) * 4;
            fen[fi + 0] = (unsigned char)(r * 255.0);
            fen[fi + 1] = (unsigned char)(g * 255.0);
            fen[fi + 2] = (unsigned char)(b * 255.0);
            fen[fi + 3] = (unsigned char)(alpha * 255.0);
        }
    }
}

static planeta_t *nebula_ex_ison(const char *ison)
{
    planeta_t *v = calloc(1, sizeof(planeta_t));
    v->qui = PLANETA_NEBULA;
    v->ubi.nebula.pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->ubi.nebula.pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->ubi.nebula.pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->ubi.nebula.pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);
    v->ubi.nebula.res.temperatura      = ison_f(ison, "nebulula.temperatura", 0.0);
    v->ubi.nebula.res.luminositas      = ison_f(ison, "nebulula.luminositas", 1.0);
    v->ubi.nebula.res.h2               = ison_f(ison, "nebulula.h2", 0.0);
    v->ubi.nebula.res.o2               = ison_f(ison, "nebulula.o2", 0.0);
    v->ubi.nebula.res.carbo            = ison_f(ison, "nebulula.carbo", 0.0);
    v->ubi.nebula.res.tectonica        = ison_f(ison, "nebulula.tectonica", 0.5);
    v->ubi.nebula.res.nubes            = ison_f(ison, "nebulula.nubes", 0.4);
    return v;
}
