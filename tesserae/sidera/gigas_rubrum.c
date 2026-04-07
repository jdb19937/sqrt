static void reddere_gigas_rubrum(
    unsigned char *fen,
    const gigas_rubrum_t *s,
    const instrumentum_t *instr
) {
    color_t col = sidus_temperatura_ad_colorem(s->pro.temperatura);

    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 2.5;

    /* discus maior, limbi obscuriores (limb darkening) */
    double r_disc = 2.5 + luciditas * 1.5;
    if (r_disc > 8.0)
        r_disc = 8.0;

    /* centrum lucidum */
    fen_punctum(fen, SEMI, SEMI, r_disc * 0.4, col, luciditas * 1.2);
    /* discus latus */
    fen_punctum(fen, SEMI, SEMI, r_disc, col, luciditas * 0.5);
    /* halo rubrum */
    color_t halo = {col.r, col.g * 0.5, col.b * 0.3, 1.0};
    fen_punctum(fen, SEMI, SEMI, r_disc * 2.0, halo, luciditas * 0.06);

    /* spiculae */
    if (instr->spiculae > 0 && luciditas > 0.3) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(
                fen, SEMI, SEMI, ang,
                instr->spiculae_long * luciditas * 0.7,
                0.5, col, luciditas * 0.15
            );
        }
    }
}
