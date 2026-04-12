#include "../planeta_communia.h"
/* glaciale.c — renderer planetae glacialis (included from planeta.c) */

void reddere_glaciale(
    unsigned char *fen, const glaciale_t *p
) {
    double rad   = p->pro.radius;
    color_t base = color_atmosphaerae(p->res.n2, p->res.o2, p->res.co2, p->res.ch4, p->res.h2, p->res.he, p->res.nh3, p->res.pulvis, 0, 0);
    /* gigantes glaciales clariores quam color purus atmosphaerae */
    base.r = fmin(1.0, base.r * 1.2 + 0.08);
    base.g = fmin(1.0, base.g * 1.2 + 0.08);
    base.b = fmin(1.0, base.b * 1.15 + 0.10);

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double lon, lat;
            if (
                !pixel_ad_sphaeram(
                    px, py, rad, p->pro.inclinatio, p->pro.rotatio,
                    &lon, &lat
                )
            )
                continue;

            double illum = 1.0;

            double nx   = lon / DPI * 4.0;
            double ny   = (lat / PI + 0.5) * 2.0;
            color_t col = base;

            /* subtle bands */
            if (p->res.fasciae > 0) {
                double band_y = lat / (PI * 0.5);
                double band   = sin(band_y * PI * p->res.fasciae * 0.5);
                double turb   = fbm(nx + p->pro.semen * 0.01, ny, 4, 0.5);
                band += (turb - 0.5) * 0.3;
                band = band * 0.5 + 0.5;
                band = fmax(0.0, fmin(1.0, band));
                /* subtle brightening/darkening */
                double mod = 0.92 + 0.08 * band * p->res.fasciae_contrast;
                col.r *= mod;
                col.g *= mod;
                col.b *= mod + 0.02;
            }

            /* spots */
            for (int mi = 0; mi < p->res.maculae; mi++) {
                double mlat = p->res.macula_lat * PI * 0.5;
                double mlon = p->res.macula_lon;
                double dlat = lat - mlat;
                double dlon = lon - mlon;
                if (dlon > PI)
                    dlon -= DPI;
                if (dlon < -PI)
                    dlon += DPI;
                double md = sqrt(dlat * dlat * 1.5 + dlon * dlon) / (p->res.macula_radius * PI);
                if (md < 1.0) {
                    double mt = (1.0 - md * md);
                    mt *= mt;
                    col.r *= (1.0 - mt * 0.35);
                    col.g *= (1.0 - mt * 0.25);
                    col.b *= (1.0 - mt * 0.10);
                }
            }

            /* texture */
            double tex = fbm(nx * 4.0, ny * 4.0, 3, 0.4) * 0.04 + 0.98;
            col.r *= tex;
            col.g *= tex;
            col.b *= tex;

            /* heavy limb darkening + atmospheric edge */
            double dx = (px - SEMI) / (rad * SEMI);
            double dy = (py - SEMI) / (rad * SEMI);
            double dz = sqrt(fmax(0.0, 1.0 - dx * dx - dy * dy));
            double ld = 0.25 + 0.75 * pow(dz, 0.30);
            double edge = pow(1.0 - dz, 1.5) * 0.45;
            color_t edge_col = base;
            edge_col.r = fmin(1.0, edge_col.r * 1.3);
            edge_col.g = fmin(1.0, edge_col.g * 1.3);
            edge_col.b = fmin(1.0, edge_col.b * 1.2);
            col = miscere(col, edge_col, fmin(0.7, edge));

            col.r *= illum * ld;
            col.g *= illum * ld;
            col.b *= illum * ld;

            fen_pixel(fen, px, py, col.r, col.g, col.b, 1.0);
        }
    }
}

void glaciale_ex_ison(glaciale_t *v, const char *ison)
{
    v->pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);
    v->res.n2               = ison_f(ison, "glaciellum.n2", 0.0);
    v->res.o2               = ison_f(ison, "glaciellum.o2", 0.0);
    v->res.co2              = ison_f(ison, "glaciellum.co2", 0.0);
    v->res.ch4              = ison_f(ison, "glaciellum.ch4", 0.0);
    v->res.h2               = ison_f(ison, "glaciellum.h2", 0.0);
    v->res.he               = ison_f(ison, "glaciellum.he", 0.0);
    v->res.nh3              = ison_f(ison, "glaciellum.nh3", 0.0);
    v->res.pulvis           = ison_f(ison, "glaciellum.pulvis", 0.0);
    v->res.fasciae          = (int)ison_f(ison, "glaciellum.fasciae", 0);
    v->res.fasciae_contrast = ison_f(ison, "glaciellum.fasciae_contrast", 0.5);
    v->res.maculae          = (int)ison_f(ison, "glaciellum.maculae", 0);
    v->res.macula_lat       = ison_f(ison, "glaciellum.macula_lat", 0.0);
    v->res.macula_lon       = ison_f(ison, "glaciellum.macula_lon", 0.0);
    v->res.macula_radius    = ison_f(ison, "glaciellum.macula_radius", 0.1);
}

void glaciale_in_ison(FILE *f, const glaciale_t *s)
{
    fprintf(f, "{\"planetella\": {\"radius\": %.2f, \"inclinatio\": %.3f, \"rotatio\": %.1f, \"semen\": %u}",
        s->pro.radius, s->pro.inclinatio, s->pro.rotatio, s->pro.semen);
    fprintf(f, ", \"glaciellum\": {\"n2\": %.3f, \"o2\": %.3f, \"co2\": %.3f, \"ch4\": %.3f, \"h2\": %.2f, \"he\": %.2f, \"nh3\": %.3f, \"pulvis\": %.1f, \"fasciae\": %d, \"fasciae_contrast\": %.2f, \"maculae\": %d, \"macula_lat\": %.2f, \"macula_lon\": %.2f, \"macula_radius\": %.2f}}",
        s->res.n2, s->res.o2, s->res.co2, s->res.ch4,
        s->res.h2, s->res.he, s->res.nh3, s->res.pulvis,
        s->res.fasciae, s->res.fasciae_contrast,
        s->res.maculae, s->res.macula_lat, s->res.macula_lon,
        s->res.macula_radius);
}
