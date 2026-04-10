/* parvum.c — renderer planetae parvi (included from planeta.c) */

static void reddere_parvum(
    unsigned char *fen, const parvum_t *p,
    const planeta_perceptus_t *perc
) {
    reddere_saxosum(fen, (const saxosum_t *)p, perc);
}

static planeta_t *parvum_ex_ison(const char *ison)
{
    planeta_t *v = calloc(1, sizeof(planeta_t));
    v->qui = PLANETA_PARVUM;
    v->ubi.parvum.pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->ubi.parvum.pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->ubi.parvum.pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->ubi.parvum.pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);
    v->ubi.parvum.res.silicata         = ison_f(ison, "parvulum.silicata", 0.0);
    v->ubi.parvum.res.ferrum           = ison_f(ison, "parvulum.ferrum", 0.0);
    v->ubi.parvum.res.sulphur          = ison_f(ison, "parvulum.sulphur", 0.0);
    v->ubi.parvum.res.carbo            = ison_f(ison, "parvulum.carbo", 0.0);
    v->ubi.parvum.res.glacies          = ison_f(ison, "parvulum.glacies", 0.0);
    v->ubi.parvum.res.glacies_co2      = ison_f(ison, "parvulum.glacies_co2", 0.0);
    v->ubi.parvum.res.malachita        = ison_f(ison, "parvulum.malachita", 0.0);
    v->ubi.parvum.res.aqua             = ison_f(ison, "parvulum.aqua", 0.0);
    v->ubi.parvum.res.aqua_profunditas = ison_f(ison, "parvulum.aqua_profunditas", 0.5);
    v->ubi.parvum.res.continentes      = (int)ison_f(ison, "parvulum.continentes", 0);
    v->ubi.parvum.res.scala            = ison_f(ison, "parvulum.scala", 1.0);
    v->ubi.parvum.res.tectonica        = ison_f(ison, "parvulum.tectonica", 0.3);
    v->ubi.parvum.res.craterae         = ison_f(ison, "parvulum.craterae", 0.0);
    v->ubi.parvum.res.maria            = ison_f(ison, "parvulum.maria", 0.0);
    v->ubi.parvum.res.vulcanismus      = ison_f(ison, "parvulum.vulcanismus", 0.0);
    v->ubi.parvum.res.pressio_kPa      = ison_f(ison, "parvulum.pressio_kPa", 0.0);
    v->ubi.parvum.res.n2               = ison_f(ison, "parvulum.n2", 0.0);
    v->ubi.parvum.res.o2               = ison_f(ison, "parvulum.o2", 0.0);
    v->ubi.parvum.res.co2              = ison_f(ison, "parvulum.co2", 0.0);
    v->ubi.parvum.res.ch4              = ison_f(ison, "parvulum.ch4", 0.0);
    v->ubi.parvum.res.h2               = ison_f(ison, "parvulum.h2", 0.0);
    v->ubi.parvum.res.he               = ison_f(ison, "parvulum.he", 0.0);
    v->ubi.parvum.res.nh3              = ison_f(ison, "parvulum.nh3", 0.0);
    v->ubi.parvum.res.pulvis           = ison_f(ison, "parvulum.pulvis", 0.0);
    v->ubi.parvum.res.nubes            = ison_f(ison, "parvulum.nubes", 0.0);
    v->ubi.parvum.res.polaris          = ison_f(ison, "parvulum.polaris", 0.0);
    return v;
}
