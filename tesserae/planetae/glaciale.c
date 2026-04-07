/* glaciale.c — renderer planetae glacialis (included from planeta.c) */

static void reddere_glaciale(
    unsigned char *fen, const glaciale_t *p,
    const planeta_perceptus_t *perc
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

            double illum = illuminatio(px, py, rad, perc->aspectus.situs, perc->aspectus.angulus) * perc->aspectus.lumen;
            if (perc->coaspectus.lumen > 0.001)
                illum += illuminatio(px, py, rad, perc->coaspectus.situs, perc->coaspectus.angulus) * perc->coaspectus.lumen;
            if (illum < 0.003) {
                /* latus obscurum — opacum nigrum ne stellae transluceant */
                fen_pixel(fen, px, py, 0.0, 0.0, 0.0, 1.0);
                continue;
            }

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
