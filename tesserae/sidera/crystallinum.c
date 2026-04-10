static void reddere_crystallinum(
    unsigned char *fen,
    const crystallinum_t *s,
    const instrumentum_t *instr
) {
    (void)instr;
    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 2.5;

    /* stella crystallina: globus Koosh — multa filamenta recta
     * ex centro irradiantia, diversis coloribus spectralibus */

    color_t col = sidus_temperatura_ad_colorem(s->pro.temperatura);

    /* nucleus album intensum */
    color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 1.2, album, luciditas * 1.0);
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 1.5);

    /* multa filamenta — ut globus Koosh */
    semen_g   = (unsigned int)(s->pro.temperatura * 137);
    int n_fil = 20 + (int)(luciditas * 15);
    if (n_fil > 60)
        n_fil = 60;

    /* 6 colores spectrales discreti */
    color_t spectra[6] = {
        {1.0, 0.15, 0.1, 1.0},   /* rubrum */
        {1.0, 0.6, 0.05, 1.0},   /* aurantiacum */
        {0.9, 1.0, 0.1, 1.0},    /* flavum */
        {0.1, 1.0, 0.3, 1.0},    /* viride */
        {0.15, 0.4, 1.0, 1.0},   /* caeruleum */
        {0.7, 0.15, 1.0, 1.0}    /* violaceum */
    };

    for (int i = 0; i < n_fil; i++) {
        /* angulus aleatorius — distributio uniformis */
        double ang = alea_f() * DUO_PI;
        double dx  = cos(ang), dy = sin(ang);

        /* longitudo variat */
        double len = 3.0 + alea_f() * (8.0 + luciditas * 8.0);
        if (len > 26.0)
            len = 26.0;

        /* color spectrale — unum ex 6 */
        color_t fc = spectra[alea() % 6];

        /* intensitas decrescens ab centro */
        int n_steps = (int)(len * 3);
        if (n_steps < 4)
            n_steps = 4;

        for (int j = 0; j <= n_steps; j++) {
            double t  = (double)j / (double)n_steps;
            double px = SEMI + t * len * dx;
            double py = SEMI + t * len * dy;

            /* intensitas: fortis prope centrum, evanescens */
            double f = luciditas * 0.4 * (1.0 - t) * (1.0 - t);
            fen_punctum(fen, px, py, 0.35, fc, f);
        }
    }

    /* halo tenuis multicolor circa centrum */
    fen_punctum(fen, SEMI, SEMI, 4.0, col, luciditas * 0.06);
}

static void crystallinum_ex_ison(sidus_t *s, const char *ison)
{
    s->qui = SIDUS_CRYSTALLINUM;
    s->ubi.crystallinum.pro.magnitudo   = ison_da_f(ison, "sidulum.magnitudo", 5.0);
    s->ubi.crystallinum.pro.temperatura = ison_da_f(ison, "sidulum.temperatura", 15000);
}
