/* sol.c — renderer solis (included from planeta.c) */

static void reddere_sol(unsigned char *fen, const sol_t *p)
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
