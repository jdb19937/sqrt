#include "../planeta_communia.h"
/* parvum.c — renderer planetae parvi (included from planeta.c) */

void reddere_parvum(
    unsigned char *fen, const parvum_t *p
) {
    reddere_saxosum(fen, (const saxosum_t *)p);
}

void parvum_ex_ison(parvum_t *v, const char *ison)
{
    v->pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);
    v->res.silicata         = ison_f(ison, "parvulum.silicata", 0.0);
    v->res.ferrum           = ison_f(ison, "parvulum.ferrum", 0.0);
    v->res.sulphur          = ison_f(ison, "parvulum.sulphur", 0.0);
    v->res.carbo            = ison_f(ison, "parvulum.carbo", 0.0);
    v->res.glacies          = ison_f(ison, "parvulum.glacies", 0.0);
    v->res.glacies_co2      = ison_f(ison, "parvulum.glacies_co2", 0.0);
    v->res.malachita        = ison_f(ison, "parvulum.malachita", 0.0);
    v->res.aqua             = ison_f(ison, "parvulum.aqua", 0.0);
    v->res.aqua_profunditas = ison_f(ison, "parvulum.aqua_profunditas", 0.5);
    v->res.continentes      = (int)ison_f(ison, "parvulum.continentes", 0);
    v->res.scala            = ison_f(ison, "parvulum.scala", 1.0);
    v->res.tectonica        = ison_f(ison, "parvulum.tectonica", 0.3);
    v->res.craterae         = ison_f(ison, "parvulum.craterae", 0.0);
    v->res.maria            = ison_f(ison, "parvulum.maria", 0.0);
    v->res.vulcanismus      = ison_f(ison, "parvulum.vulcanismus", 0.0);
    v->res.pressio_kPa      = ison_f(ison, "parvulum.pressio_kPa", 0.0);
    v->res.n2               = ison_f(ison, "parvulum.n2", 0.0);
    v->res.o2               = ison_f(ison, "parvulum.o2", 0.0);
    v->res.co2              = ison_f(ison, "parvulum.co2", 0.0);
    v->res.ch4              = ison_f(ison, "parvulum.ch4", 0.0);
    v->res.h2               = ison_f(ison, "parvulum.h2", 0.0);
    v->res.he               = ison_f(ison, "parvulum.he", 0.0);
    v->res.nh3              = ison_f(ison, "parvulum.nh3", 0.0);
    v->res.pulvis           = ison_f(ison, "parvulum.pulvis", 0.0);
    v->res.nubes            = ison_f(ison, "parvulum.nubes", 0.0);
    v->res.polaris          = ison_f(ison, "parvulum.polaris", 0.0);
}

void parvum_in_ison(FILE *f, const parvum_t *s)
{
    fprintf(f, "{\"planetella\": {\"radius\": %.2f, \"inclinatio\": %.3f, \"rotatio\": %.1f, \"semen\": %u}",
        s->pro.radius, s->pro.inclinatio, s->pro.rotatio, s->pro.semen);
    fprintf(f, ", \"parvulum\": {\"silicata\": %.2f, \"ferrum\": %.2f, \"sulphur\": %.2f, \"carbo\": %.2f, \"glacies\": %.2f, \"glacies_co2\": %.2f, \"malachita\": %.2f, \"aqua\": %.2f, \"aqua_profunditas\": %.1f, \"continentes\": %d, \"scala\": %.1f, \"tectonica\": %.1f, \"craterae\": %.2f, \"maria\": %.2f, \"vulcanismus\": %.1f, \"pressio_kPa\": %.1f, \"n2\": %.3f, \"o2\": %.3f, \"co2\": %.3f, \"ch4\": %.3f, \"h2\": %.3f, \"he\": %.3f, \"nh3\": %.3f, \"pulvis\": %.1f, \"nubes\": %.2f, \"polaris\": %.2f}}",
        s->res.silicata, s->res.ferrum, s->res.sulphur, s->res.carbo,
        s->res.glacies, s->res.glacies_co2, s->res.malachita,
        s->res.aqua, s->res.aqua_profunditas, s->res.continentes, s->res.scala,
        s->res.tectonica, s->res.craterae, s->res.maria, s->res.vulcanismus,
        s->res.pressio_kPa, s->res.n2, s->res.o2, s->res.co2, s->res.ch4,
        s->res.h2, s->res.he, s->res.nh3, s->res.pulvis, s->res.nubes, s->res.polaris);
}
