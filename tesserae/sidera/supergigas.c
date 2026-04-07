static void reddere_supergigas(
    unsigned char *fen,
    const supergigas_t *s,
    const instrumentum_t *instr
) {
    color_t col = sidus_temperatura_ad_colorem(s->pro.temperatura);

    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 4.0;

    /* discus vastus */
    double r_disc = 5.0 + luciditas * 3.0;
    if (r_disc > 15.0)
        r_disc = 15.0;

    fen_punctum(fen, SEMI, SEMI, r_disc * 0.3, col, luciditas * 1.5);
    fen_punctum(fen, SEMI, SEMI, r_disc * 0.7, col, luciditas * 0.6);
    fen_punctum(fen, SEMI, SEMI, r_disc, col, luciditas * 0.2);

    /* maculae (cellulae convectionis) — perturbationes coloris */
    semen_g = (unsigned int)(s->pro.temperatura * 1000);
    for (int i = 0; i < 5; i++) {
        double mx = SEMI + alea_gauss() * r_disc * 0.4;
        double my = SEMI + alea_gauss() * r_disc * 0.4;
        double dx = mx - SEMI, dy = my - SEMI;
        if (dx * dx + dy * dy > r_disc * r_disc * 0.6)
            continue;
        color_t mc = {col.r * 0.7, col.g * 0.6, col.b * 0.5, 1.0};
        fen_punctum(fen, mx, my, r_disc * 0.2, mc, luciditas * 0.15);
    }

    /* spiculae longissimae */
    if (instr->spiculae > 0) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(
                fen, SEMI, SEMI, ang,
                instr->spiculae_long * luciditas * 1.5,
                0.6, col, luciditas * 0.2
            );
        }
    }

    if (instr->halo_vis > 0.01)
        fen_punctum(
            fen, SEMI, SEMI, instr->halo_radius * 2.0,
            col, instr->halo_vis * luciditas * 0.5
        );
}
