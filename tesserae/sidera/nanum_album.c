static void reddere_nanum_album(
    unsigned char *fen,
    const nanum_album_t *s,
    const instrumentum_t *instr
) {
    color_t col = sidus_temperatura_ad_colorem(s->pro.temperatura);

    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 2.0;

    /* nucleus intensissimus, minimus */
    fen_punctum(fen, SEMI, SEMI, 0.6, col, luciditas * 1.5);

    /* halo tenuis caeruleus */
    color_t halo_col = {col.r * 0.6, col.g * 0.7, col.b * 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 2.5, halo_col, luciditas * 0.3);

    (void)instr;
}
