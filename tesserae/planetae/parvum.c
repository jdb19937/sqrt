/* parvum.c — renderer planetae parvi (included from planeta.c) */

static void reddere_parvum(
    unsigned char *fen, const parvum_t *p,
    const planeta_perceptus_t *perc
) {
    reddere_saxosum(fen, (const saxosum_t *)p, perc);
}
