#include "../planeta_communia.h"
/* sol.c — renderer solis (included from planeta.c) */

void reddere_sol(unsigned char *fen, const sol_t *p)
{
    double radius = p->pro.radius * SEMI;
    double t_f = (p->res.temperatura > 100.0) ? p->res.temperatura
        : temperatura_ex_compositione(p->res.h2, p->res.he, p->res.ch4);
    color_t col = temperatura_ad_colorem_f(t_f);
    double lux  = (p->res.luminositas > 0.001) ? p->res.luminositas : 1.0;

    double corona_v   = p->res.corona;
    double corona_ext = radius + (SEMI * 1.8 - radius) * corona_v * 0.75;

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double dx = px - SEMI, dy = py - SEMI;
            double d2 = dx * dx + dy * dy;
            double d  = sqrt(d2);

            int fi = (py * FEN + px) * 4;

            if (d <= radius) {
                /* ---- discus solis ---- */
                double mu     = sqrt(1.0 - d2 / (radius * radius));
                double bright = 0.4 + 0.6 * mu;  /* Eddington */

                /* granulatio — cellulae convectionis */
                if (p->res.granulatio > 0.001) {
                    double nx    = dx / radius * 7.0 + (double)(p->pro.semen % 31);
                    double ny    = dy / radius * 7.0 + 5.3;
                    double grain = fbm(nx, ny, 3, 0.55);
                    bright *= 1.0 + (grain - 0.5) * p->res.granulatio * 0.28;
                }

                /* maculae (sunspots) in fascia aequatoriali */
                for (int k = 0; k < p->res.maculae && k < 8; k++) {
                    double hx = hash2((int)(p->pro.semen) + k * 41,     1);
                    double hy = hash2((int)(p->pro.semen) + k * 41,     2);
                    double hs = hash2((int)(p->pro.semen) + k * 41,     3);
                    double cx = (hx - 0.5) * 1.5 * radius;
                    double cy = (hy - 0.5) * 0.75 * radius;
                    double mr = (0.04 + hs * 0.09) * radius
                    * fmax(0.1, p->res.macula_radius * 4.0);
                    double sdx = dx - cx, sdy = dy - cy;
                    double sd  = sqrt(sdx * sdx + sdy * sdy);
                    if (sd < mr) {
                        double t = sd / mr;
                        bright *= 1.0 - (1.0 - t * t)
                        * fmin(0.75, fabs(p->res.macula_obscuritas));
                    }
                }

                bright = fmax(0.0, bright) * lux;

                fen[fi + 0] = (unsigned char)fmin(255.0, col.r * bright * 255.0);
                fen[fi + 1] = (unsigned char)fmin(255.0, col.g * bright * 255.0);
                fen[fi + 2] = (unsigned char)fmin(255.0, col.b * bright * 255.0);
                fen[fi + 3] = 255;

            } else if (corona_v > 0.001 && d <= corona_ext) {
                /* ---- corona ---- */
                double t    = (d - radius) / (corona_ext - radius);
                double base = corona_v * exp(-t * t * 5.0) * 0.9;
                double ang  = atan2(dy, dx);
                double nv   = fbm(
                    cos(ang) * 3.0 + (double)(p->pro.semen % 17) * 0.1,
                    sin(ang) * 3.0 + 5.2, 3, 0.6
                );
                double cf = fmin(1.0, base * (0.35 + nv) * lux);
                if (cf < 0.003)
                    continue;

                fen[fi + 0] = (unsigned char)fmin(255.0, col.r * cf * 255.0);
                fen[fi + 1] = (unsigned char)fmin(255.0, col.g * cf * 255.0);
                fen[fi + 2] = (unsigned char)fmin(255.0, col.b * cf * 0.85 * 255.0);
                fen[fi + 3] = (unsigned char)(cf * 255.0);
            }
        }
    }
}

void sol_ex_ison(sol_t *v, const char *ison)
{
    v->pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);
    v->res.fusio            = ison_f(ison, "soliculum.fusio", 1.0);
    v->res.temperatura      = ison_f(ison, "soliculum.temperatura", 0.0);
    v->res.luminositas      = ison_f(ison, "soliculum.luminositas", 1.0);
    v->res.corona           = ison_f(ison, "soliculum.corona", 0.0);
    v->res.granulatio       = ison_f(ison, "soliculum.granulatio", 0.0);
    v->res.maculae          = (int)ison_f(ison, "soliculum.maculae", 0);
    v->res.macula_radius    = ison_f(ison, "soliculum.macula_radius", 0.1);
    v->res.macula_obscuritas = ison_f(ison, "soliculum.macula_obscuritas", 0.5);
    v->res.h2               = ison_f(ison, "soliculum.h2", 0.0);
    v->res.he               = ison_f(ison, "soliculum.he", 0.0);
    v->res.ch4              = ison_f(ison, "soliculum.ch4", 0.0);
    v->res.nh3              = ison_f(ison, "soliculum.nh3", 0.0);
}

void sol_in_ison(FILE *f, const sol_t *s)
{
    fprintf(f, "{\"planetella\": {\"radius\": %.2f, \"inclinatio\": %.3f, \"rotatio\": %.1f, \"semen\": %u}",
        s->pro.radius, s->pro.inclinatio, s->pro.rotatio, s->pro.semen);
    fprintf(f, ", \"soliculum\": {\"fusio\": %.1f, \"temperatura\": %.0f, \"luminositas\": %.1f, \"corona\": %.2f, \"granulatio\": %.2f, \"h2\": %.3f, \"he\": %.3f, \"ch4\": %.3f, \"nh3\": %.3f, \"maculae\": %d, \"macula_radius\": %.1f, \"macula_obscuritas\": %.2f}}",
        s->res.fusio, s->res.temperatura, s->res.luminositas,
        s->res.corona, s->res.granulatio,
        s->res.h2, s->res.he, s->res.ch4, s->res.nh3,
        s->res.maculae, s->res.macula_radius, s->res.macula_obscuritas);
}
