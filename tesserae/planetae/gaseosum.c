/* gaseosum.c — renderer planetae gaseosi (included from planeta.c) */

static void aplicare_fusionem(unsigned char *fen, const gaseosum_t *p)
{
    if (p->res.fusio < 0.001)
        return;
    double f   = p->res.fusio;
    double rad = p->pro.radius * SEMI;
    double t_f = (p->res.temperatura > 100.0) ? p->res.temperatura
        : temperatura_ex_compositione(p->res.h2, p->res.he, p->res.ch4);
    color_t scol = temperatura_ad_colorem_f(t_f);

    /* --- discus: limb darkening + blend + fulgor --- */
    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double dx = px - SEMI, dy = py - SEMI;
            double d2 = dx * dx + dy * dy;
            if (d2 > rad * rad)
                continue;
            int fi = (py * FEN + px) * 4;
            if (fen[fi + 3] == 0)
                continue;

            double mu   = sqrt(1.0 - d2 / (rad * rad));
            double limb = 0.4 + 0.6 * mu;  /* Eddington */

            double cr = fen[fi + 0] / 255.0;
            double cg = fen[fi + 1] / 255.0;
            double cb = fen[fi + 2] / 255.0;

            /* aplica limb darkening proportionaliter ad fusionem */
            cr = cr * (1.0 - f) + cr * limb * f;
            cg = cg * (1.0 - f) + cg * limb * f;
            cb = cb * (1.0 - f) + cb * limb * f;

            /* blend versus colorem stellarem */
            double bv = f * 0.75;
            cr        = cr * (1.0 - bv) + scol.r * bv;
            cg        = cg * (1.0 - bv) + scol.g * bv;
            cb        = cb * (1.0 - bv) + scol.b * bv;

            /* fulgor internus: minima luciditas e fusione */
            double fl = f * 0.25;
            if (cr < fl * scol.r)
                cr = fl * scol.r;
            if (cg < fl * scol.g)
                cg = fl * scol.g;
            if (cb < fl * scol.b)
                cb = fl * scol.b;

            fen[fi + 0] = (unsigned char)fmin(255.0, cr * 255.0);
            fen[fi + 1] = (unsigned char)fmin(255.0, cg * 255.0);
            fen[fi + 2] = (unsigned char)fmin(255.0, cb * 255.0);
            fen[fi + 3] = 255;
        }
    }

    /* --- corona per fusionem --- */
    if (p->res.corona < 0.001)
        return;
    double corona_ext = rad + (SEMI * 1.5 - rad) * p->res.corona * f;

    if (corona_ext <= rad)
        return;
    color_t ccol = temperatura_ad_colorem_f(t_f * 1.05);  /* corona paullo calidior */

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double dx = px - SEMI, dy = py - SEMI;
            double d  = sqrt(dx * dx + dy * dy);
            if (d <= rad || d > corona_ext)
                continue;
            double t   = (d - rad) / (corona_ext - rad);
            double cf  = p->res.corona * f * exp(-t * t * 5.0) * 0.85;
            double ang = atan2(dy, dx);
            double nv = fbm(
                cos(ang) * 3.0 + (double)(p->pro.semen % 19) * 0.1,
                sin(ang) * 3.0 + 4.7, 3, 0.6
            );
            cf *= 0.35 + nv;
            if (cf < 0.003)
                continue;
            if (cf > 1.0)
                cf = 1.0;
            int fi = (py * FEN + px) * 4;
            unsigned char ca = (unsigned char)(cf * 255.0);
            /* additiva saturans supra existentem */
            int nr      = fen[fi + 0] + (int)(ccol.r * cf * 255.0);
            int ng      = fen[fi + 1] + (int)(ccol.g * cf * 255.0);
            int nb      = fen[fi + 2] + (int)(ccol.b * cf * 0.88 * 255.0);
            fen[fi + 0] = (unsigned char)fmin(255, nr);
            fen[fi + 1] = (unsigned char)fmin(255, ng);
            fen[fi + 2] = (unsigned char)fmin(255, nb);
            if (fen[fi + 3] < ca)
                fen[fi + 3] = ca;
        }
    }
}

static void reddere_gaseosum(
    unsigned char *fen, const gaseosum_t *p,
    const planeta_perceptus_t *perc
) {
    double rad      = p->pro.radius;
    color_t atm_col = color_atmosphaerae(p->res.n2, p->res.o2, p->res.co2, p->res.ch4, p->res.h2, p->res.he, p->res.nh3, p->res.pulvis, 0, 0);

    /* colores zonarum/cingulorum ex atmosphaera derivati */
    color_t zona = atm_col;
    zona.r       = fmin(1.0, zona.r * 1.4 + 0.12);
    zona.g       = fmin(1.0, zona.g * 1.4 + 0.10);
    zona.b       = fmin(1.0, zona.b * 1.3 + 0.06);
    color_t belt = atm_col;
    belt.r *= 0.60;
    belt.g *= 0.48;
    belt.b *= 0.40;

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

            double band_y = lat / (PI * 0.5);
            double nx     = lon / DPI * 8.0;
            double ny     = (band_y + 1.0) * 2.0;

            /* banded structure */
            double band = sin(band_y * PI * p->res.fasciae * 0.5);

            /* turbulence within bands — multi-scale */
            double turb1 = fbm_warp(
                nx + p->pro.semen * 0.003, ny, 6, 0.5,
                1.5 * p->res.fasciae_contrast
            );
            double turb2 = fbm(nx * 2.0 + band_y * 5.0, ny * 0.3, 4, 0.45);
            band += (turb1 - 0.5) * 0.8 * p->res.fasciae_contrast;
            band += (turb2 - 0.5) * 0.3;

            /* vortices fluxus zonalis ad limites fasciarum */
            double flow        = ridged(nx * 1.5 + band_y * 2.0, band_y * 3.0, 3, 0.5);
            double at_boundary = 1.0 - fabs(band) * 2.0;
            at_boundary        = fmax(0.0, at_boundary);
            band += (flow - 0.5) * 0.4 * at_boundary;

            double t    = fmax(0.0, fmin(1.0, band * 0.5 + 0.5));
            color_t col = miscere(belt, zona, t);

            /* textura nubium multi-scala */
            double tex1 = fbm(nx * 4.0, ny * 4.0, 4, 0.45);
            double tex2 = fbm(nx * 10.0 + 31.0, ny * 10.0 + 37.0, 3, 0.4);
            double tex3 = strepitus2(nx * 25.0 + 71.0, ny * 25.0 + 73.0);
            double tex  = tex1 * 0.5 + tex2 * 0.3 + tex3 * 0.2;
            /* variatio chromatica inter fascias */
            double chr = fbm(nx * 3.0 + 41.0, ny * 6.0 + 43.0, 3, 0.4);
            col.r *= 0.80 + 0.20 * tex;
            col.g *= 0.80 + 0.20 * tex * (0.9 + 0.1 * chr);
            col.b *= 0.82 + 0.18 * tex * (0.85 + 0.15 * chr);

            /* maculae — storm vortices */
            for (int mi = 0; mi < p->res.maculae; mi++) {
                double mlat = p->res.macula_lat * PI * 0.5;
                double mlon = p->res.macula_lon + mi * 1.8;
                double mrad = p->res.macula_radius * (1.0 - mi * 0.25);
                if (mrad < 0.02)
                    continue;

                double dlat = lat - mlat;
                double dlon = lon - mlon;
                if (dlon > PI)
                    dlon -= DPI;
                if (dlon < -PI)
                    dlon += DPI;
                double md = sqrt(dlat * dlat * 2.5 + dlon * dlon) / (mrad * PI);
                if (md >= 1.0)
                    continue;

                double mt = 1.0 - md * md;
                /* spiral structure inside vortex */
                double ang    = atan2(dlat, dlon);
                double spiral = sin(ang * 2.0 - md * 8.0 + p->pro.semen * 0.1);
                spiral        = spiral * 0.5 + 0.5;
                double vort   = fbm(md * 5.0 + ang * 0.4, ang * 0.5 + md * 3.0, 4, 0.5);
                mt *= (0.5 + 0.3 * spiral + 0.2 * vort);

                /* color tempestatis: chromophorae organicae (phosphorus,
                 * sulphur composita) dant colorem rubrum/brunneum.
                 * Macula Rubra Magna: vortex anticyclonicus persistens,
                 * coloratus per upwelling composita ex profunditate.
                 * Ref: Simon-Miller et al. (2001) Icarus 154:459-474 */
                double dark = p->res.macula_obscuritas;
                color_t mc;
                mc.r = fmin(1.0, 0.65 + dark * 0.15);
                mc.g = fmax(0.0, 0.30 - dark * 0.10);
                mc.b = fmax(0.0, 0.18 - dark * 0.10);
                mc.a = 1.0;
                col  = miscere(col, mc, mt * 0.90);
            }

            /* limbi obscuratio fortis pro gigantibus gaseouis */
            double dx = (px - SEMI) / (rad * SEMI);
            double dy = (py - SEMI) / (rad * SEMI);
            double dz = sqrt(fmax(0.0, 1.0 - dx * dx - dy * dy));
            double ld = 0.30 + 0.70 * pow(dz, 0.35);

            col.r *= illum * ld;
            col.g *= illum * ld;
            col.b *= illum * ld;

            fen_pixel(fen, px, py, col.r, col.g, col.b, 1.0);
        }
    }

    aplicare_fusionem(fen, p);
}
