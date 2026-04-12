#include "../sidus_communia.h"

void reddere_nanum_album(
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

void nanum_album_ex_ison(nanum_album_t *s, const char *ison)
{
    s->pro.magnitudo   = ison_da_f(ison, "sidulum.magnitudo", 5.0);
    s->pro.temperatura = ison_da_f(ison, "sidulum.temperatura", 10000);
}

void nanum_album_in_ison(FILE *f, const nanum_album_t *s)
{
    fprintf(f, "{\"sidulum\": {\"magnitudo\": %.3f, \"temperatura\": %.1f}", s->pro.magnitudo, s->pro.temperatura);
    fprintf(f, ", \"nanulum_album\": {}");
    fprintf(f, "}");
}
