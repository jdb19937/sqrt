static void reddere_sequentia(
    unsigned char *fen,
    const sequentia_t *s,
    const instrumentum_t *instr
) {
    color_t col = sidus_temperatura_ad_colorem(s->pro.temperatura);

    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 2.0;

    /* nucleus */
    double r_nucl = 0.8 + luciditas * 0.3;
    if (r_nucl > 3.0)
        r_nucl = 3.0;
    fen_punctum(fen, SEMI, SEMI, r_nucl, col, luciditas);

    /* halo levis */
    if (luciditas > 0.3)
        fen_punctum(fen, SEMI, SEMI, r_nucl * 3.0, col, luciditas * 0.08);

    /* spiculae instrumenti */
    if (instr->spiculae > 0 && luciditas > 0.2) {
        for (int i = 0; i < instr->spiculae; i++) {
            double ang = instr->spiculae_ang
                + PI_GRAECUM * (double)i / (double)instr->spiculae;
            fen_spicula(
                fen, SEMI, SEMI, ang,
                instr->spiculae_long * luciditas,
                0.4, col, luciditas * 0.25
            );
        }
    }

    /* halo instrumenti */
    if (instr->halo_vis > 0.01)
        fen_punctum(
            fen, SEMI, SEMI, instr->halo_radius,
            col, instr->halo_vis * luciditas
        );
}
