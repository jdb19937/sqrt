static void reddere_vagans(
    unsigned char *fen,
    const vagans_t *s,
    const instrumentum_t *instr
) {
    (void)instr;
    color_t col = sidus_temperatura_ad_colorem(s->pro.temperatura);

    /* vagans: discus maior, matte, cum falce (phase) */
    double radius = 8.0 + pow(10.0, -s->pro.magnitudo * 0.4) * 6.0;
    if (radius > 25.0)
        radius = 25.0;

    double cos_ph = cos(s->res.angulus);
    double sin_ph = sin(s->res.angulus);

    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            double dx = x - SEMI, dy = y - SEMI;
            double d2 = dx * dx + dy * dy;
            double r2 = radius * radius;
            if (d2 > r2)
                continue;

            /* norma hemisphaerica */
            double nz = sqrt(1.0 - d2 / r2);
            double nx = dx / radius;
            double ny = dy / radius;

            /* illuminatio per directionem phase */
            double illum = nx * cos_ph + ny * sin_ph;
            /* phase: 0=plenus (totus illuminatus), 1=novus (obscurus) */
            double terminator = (1.0 - s->res.phase * 2.0);
            double vis        = illum - terminator;

            double f;
            if (vis > 0.05) {
                /* facies illuminata — Lambert matte */
                double lambert = nz * 0.5 + illum * 0.4 + 0.1;
                if (lambert < 0)
                    lambert = 0;
                f = lambert;
            } else if (vis > -0.05) {
                /* terminator — transitio levis */
                f = (vis + 0.05) * 10.0 * 0.3;
                if (f < 0)
                    f = 0;
            } else {
                /* facies obscura — paene nigra */
                f = 0.02;
            }

            /* limb darkening */
            f *= (0.6 + 0.4 * nz);

            int idx = (y * FEN + x) * 4;
            int r   = (int)(col.r * f * 255);
            int g   = (int)(col.g * f * 255);
            int b   = (int)(col.b * f * 255);
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = 255;
        }
    }
}
