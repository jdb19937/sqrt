static void reddere_neutronium(
    unsigned char *fen,
    const neutronium_t *s,
    const instrumentum_t *instr
) {
    (void)instr;
    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 3.0;

    /* punctum album intensissimum */
    color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.4, album, luciditas * 2.0);

    /* annuli pulsantes — duae tenues fasciae */
    color_t cyan = {0.3, 0.7, 1.0, 1.0};
    double r1 = 2.0 + luciditas;
    double r2 = 4.0 + luciditas * 2.0;

    for (int y = 0; y < FEN; y++) {
        for (int x = 0; x < FEN; x++) {
            double dx = x - SEMI, dy = y - SEMI;
            double d  = sqrt(dx * dx + dy * dy);
            double f1 = exp(-(d - r1) * (d - r1) * 2.0) * luciditas * 0.4;
            double f2 = exp(-(d - r2) * (d - r2) * 1.0) * luciditas * 0.15;
            double f  = f1 + f2;
            if (f < 0.002)
                continue;

            int idx = (y * FEN + x) * 4;
            int r   = (int)(fen[idx + 0] + cyan.r * f * 255);
            int g   = (int)(fen[idx + 1] + cyan.g * f * 255);
            int b   = (int)(fen[idx + 2] + cyan.b * f * 255);
            int a   = (int)(fen[idx + 3] + f * 255);
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            if (a > 255)
                a = 255;
            fen[idx + 0] = (unsigned char)r;
            fen[idx + 1] = (unsigned char)g;
            fen[idx + 2] = (unsigned char)b;
            fen[idx + 3] = (unsigned char)a;
        }
    }

    /* bipolar jets — duae spiculae oppositae */
    double ang_jet = alea_f() * PI_GRAECUM;
    color_t jet_col = {0.5, 0.6, 1.0, 1.0};
    fen_spicula(
        fen, SEMI, SEMI, ang_jet, 12.0 * luciditas, 0.3,
        jet_col, luciditas * 0.3
    );
    fen_spicula(
        fen, SEMI, SEMI, ang_jet + PI_GRAECUM, 12.0 * luciditas, 0.3,
        jet_col, luciditas * 0.3
    );
}

static void neutronium_ex_ison(sidus_t *s, const char *ison)
{
    s->qui = SIDUS_NEUTRONIUM;
    s->ubi.neutronium.pro.magnitudo   = ison_da_f(ison, "sidulum.magnitudo", 5.0);
    s->ubi.neutronium.pro.temperatura = ison_da_f(ison, "sidulum.temperatura", 20000);
}
