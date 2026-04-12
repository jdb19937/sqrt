#include "../planeta_communia.h"
/* saxosum.c — renderer planetae saxosi (included from planeta.c) */

double continent_mask(double lon, double lat, const saxosum_t *p)
{
    if (p->res.aqua < 0.01)
        return 1.0; /* nulla aqua → tota terra */

    double sc = fmax(0.3, p->res.scala);
    double ox = (p->pro.semen & 0xFF) * 0.13;
    double oy = ((p->pro.semen >> 8) & 0xFF) * 0.13;

    /* blobs continentales — massae magnae terrae */
    double elevatio = 0.0;
    unsigned s = p->pro.semen * 7919 + 77777;
    int nc = fmax(1, p->res.continentes);
    for (int c = 0; c < nc; c++) {
        s = s * 1103515245 + 12345;
        double blon = (s >> 8 & 0xFFFF) / 65536.0 * DPI;
        s = s * 1103515245 + 12345;
        double blat = ((s >> 8 & 0xFFFF) / 65536.0 - 0.5) * PI * 0.85;
        s = s * 1103515245 + 12345;
        double brad = (1.0 + (s >> 8 & 0xFF) / 255.0 * 1.2) * sc / pow(nc, 0.35);

        double dl = lat - blat;
        double dn = lon - blon;
        if (dn > PI)
            dn -= DPI;
        if (dn < -PI)
            dn += DPI;
        double dd   = sqrt(dl * dl + dn * dn * cos(lat) * cos(lat));
        double blob = fmax(0.0, 1.0 - dd / brad);
        blob        = blob * blob;
        elevatio    = fmax(elevatio, blob);
    }

    /* detallum litorale per FBM */
    double nx   = lon / DPI * 5.0 * sc;
    double ny   = (lat / PI + 0.5) * 2.5 * sc;
    double warp = 1.0 + p->res.tectonica * 3.0;
    double det  = fbm_warp(nx + ox, ny + oy, 7, 0.55, warp);

    /* combinatio */
    double terrain = elevatio * 0.70 + det * 0.30;

    /* limen maris: aqua est fractio sub aqua.
     * terrain ∈ [0, ~0.9]. Invertimus: terra ubi terrain altior.
     * Exemplum: aqua=0.71 → ~29% pixels supra limen. */
    double limen = p->res.aqua * 0.30;
    return (terrain > limen) ? 1.0 : 0.0;
}

void reddere_saxosum(
    unsigned char *fen, const saxosum_t *p
) {
    double rad      = p->pro.radius;
    double pnorm    = fmin(1.0, p->res.pressio_kPa / 101.0); /* [0,1] normalized */
    color_t atm_col = color_atmosphaerae(p->res.n2, p->res.o2, p->res.co2, p->res.ch4, p->res.h2, p->res.he, p->res.nh3, p->res.pulvis, p->res.ferrum, p->res.sulphur);

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

            /* coordinatae strepitus — scala pro detallo ad 256px */
            double nx = lon / DPI * 10.0;
            double ny = (lat / PI + 0.5) * 5.0;

            /* variatio localis [-0.3, 0.3] */
            double var = (fbm(nx * 3.0 + p->pro.semen * 0.01, ny * 3.0, 5, 0.5) - 0.5) * 0.6;

            /* terra an aqua? */
            int terra = 1;
            if (p->res.aqua > 0.01)
                terra = (int)continent_mask(lon, lat, p);

            color_t col;

            if (terra) {
                col = color_superficiei(p->res.silicata, p->res.ferrum, p->res.sulphur, p->res.carbo, p->res.glacies, p->res.glacies_co2, p->res.malachita, var);

                /* biomes: variatio coloris intra continentes ex latitudine.
                 * Si malachita praesens (vegetatio), modulatur per lat:
                 *   tropicae (|lat|<25°): viridis saturatus
                 *   temperatae (25-55°): viridis moderatus
                 *   boreales (55-70°): viridis obscurior
                 *   aridae (deserta): flavum-brunneum per strepitum
                 * Hoc producit variegatam superficiem non uniformem. */
                if (p->res.malachita > 0.1) {
                    double abs_lat = fabs(lat) / (PI * 0.5);

                    /* ariditas: deserta per strepitum (non omnia virides) */
                    double arid = fbm(nx * 1.5 + 33.0, ny * 1.5 + 17.0, 4, 0.5);
                    arid        = fmax(0.0, arid - 0.35) * 2.5; /* 0..1 */
                    /* deserta magis ad aequatorem lateralem (~30°) */
                    double desert_band = exp(-(abs_lat - 0.35) * (abs_lat - 0.35) * 40.0);
                    arid *= desert_band;

                    if (arid > 0.3) {
                        /* deserta: ferrum/silicata colores */
                        color_t desert = {0.65, 0.50, 0.32, 1.0};
                        col = miscere(col, desert, fmin(1.0, arid * 0.8));
                    }

                    /* latitudo modulat verdorem */
                    double green_mod = 1.0;
                    if (abs_lat < 0.3)
                        green_mod = 1.2; /* tropicae: saturatior */
                    else if (abs_lat < 0.65)
                        green_mod = 1.0; /* temperatae */
                    else if (abs_lat < 0.85)
                        green_mod = 0.6; /* boreales: tundra */
                    else
                        green_mod = 0.2; /* arcticae: vix */

                    col.g *= (0.7 + 0.3 * green_mod);
                }

                /* multi-scale textura superficiei.
                 * Tres scalae: macro (regiones), meso (montanae), micro (rugositas).
                 * Variatio chromatica — non solum luminositas sed color
                 * variat per canalem independenter. */
                double tex_macro = fbm_warp(nx * 2.0 + 7.0, ny * 2.0 + 3.0, 5, 0.5, 1.2);
                double tex_meso  = ridged(nx * 5.0 + 11.0, ny * 5.0 + 5.0, 4, 0.5);
                double tex_micro = fbm(nx * 15.0 + 23.0, ny * 15.0 + 19.0, 4, 0.45);
                double tex_finis = strepitus2(nx * 40.0 + 61.0, ny * 40.0 + 67.0);

                /* luminositas: macro + meso + micro + finis (pixel-scala) */
                double lum = 0.50 + 0.22 * tex_macro + 0.13 * tex_meso
                    + 0.09 * tex_micro + 0.06 * tex_finis;

                /* variatio chromatica: canalem separatim perturbat.
                 * R perturbatur per unam noise, G per aliam, B per tertiam.
                 * Hoc dat variationem coloris subtiliter non-uniformem. */
                double chr_r = fbm(nx * 4.0 + 31.0, ny * 4.0 + 37.0, 3, 0.4);
                double chr_g = fbm(nx * 4.0 + 43.0, ny * 4.0 + 47.0, 3, 0.4);
                double chr_b = fbm(nx * 4.0 + 53.0, ny * 4.0 + 59.0, 3, 0.4);
                col.r *= lum * (0.85 + 0.15 * chr_r);
                col.g *= lum * (0.85 + 0.15 * chr_g);
                col.b *= lum * (0.85 + 0.15 * chr_b);

                /* maria basaltica (lava plains — dark, smooth) */
                if (p->res.maria > 0.01) {
                    double mare = fbm_warp(nx * 0.6 + 3.7, ny * 0.6 + 1.2, 6, 0.5, 1.5);
                    if (mare < p->res.maria) {
                        double mt    = fmin(1.0, (p->res.maria - mare) / fmax(0.01, p->res.maria) * 3.0);
                        color_t dark = col;
                        dark.r *= 0.35;
                        dark.g *= 0.35;
                        dark.b *= 0.38;
                        col = miscere(col, dark, mt);
                    }
                }

                /* craterae */
                if (p->res.craterae > 0.01) {
                    double bright;
                    double depth = crater_field(
                        lon, lat, p->res.craterae,
                        p->pro.semen + 777, &bright
                    );
                    double lmod = 1.0 + depth * 1.0;
                    lmod        = fmax(0.4, fmin(1.5, lmod));
                    col.r *= lmod;
                    col.g *= lmod;
                    col.b *= lmod;
                    col.r = fmin(1.0, col.r + bright * 0.8);
                    col.g = fmin(1.0, col.g + bright * 0.8);
                    col.b = fmin(1.0, col.b + bright * 0.75);
                }

                /* vulcanismus */
                if (p->res.vulcanismus > 0.01) {
                    double v   = fbm_warp(nx * 2.0 + 11.0, ny * 2.0 + 7.0, 5, 0.6, 2.5);
                    double thr = 1.0 - p->res.vulcanismus * 0.3;
                    if (v > thr) {
                        double t = (v - thr) / (1.0 - thr);
                        t        = t * t;
                        /* lava blackbody: ~1000K=red, ~1500K=orange, ~2000K=yellow */
                        color_t lava;
                        if (t < 0.4) {
                            double t2 = t / 0.4;
                            lava = (color_t){0.8 * t2, 0.15 * t2, 0.02, 1.0};
                        } else {
                            double t2 = (t - 0.4) / 0.6;
                            lava = (color_t){0.8 + 0.2 * t2, 0.15 + 0.55 * t2, 0.02 + 0.2 * t2, 1.0};
                        }
                        col   = miscere(col, lava, t);
                    }
                }
            } else {
                /* oceanus: color ex profunditate.
                 * Litora: aqua vadosa caeruleum clarum (scattering per arenam).
                 * Profunditas per distantiam a litore. */
                double depth_var = fbm(nx * 1.2 + 20.0, ny * 1.2 + 20.0, 5, 0.5);

                /* distantia a litore: approximatur per terra_elev valorem.
                 * terrain prope limen = litus, longe infra = profundum */
                double sc_l = fmax(0.3, p->res.scala);
                double ox_l = (p->pro.semen & 0xFF) * 0.13;
                double oy_l = ((p->pro.semen >> 8) & 0xFF) * 0.13;
                double nx_l = lon / DPI * 5.0 * sc_l;
                double ny_l = (lat / PI + 0.5) * 2.5 * sc_l;
                double terra_val = fbm_warp(
                    nx_l + ox_l, ny_l + oy_l, 5, 0.55,
                    1.0 + p->res.tectonica * 3.0
                ) * 0.30;
                /* blob contributo */
                unsigned s_l    = p->pro.semen * 7919 + 77777;
                int nc_l        = fmax(1, p->res.continentes);
                double blob_max = 0.0;
                for (int c = 0; c < nc_l; c++) {
                    s_l         = s_l * 1103515245 + 12345;
                    double blon = (s_l >> 8 & 0xFFFF) / 65536.0 * DPI;
                    s_l         = s_l * 1103515245 + 12345;
                    double blat = ((s_l >> 8 & 0xFFFF) / 65536.0 - 0.5) * PI * 0.85;
                    s_l         = s_l * 1103515245 + 12345;
                    double brad = (1.0 + (s_l >> 8 & 0xFF) / 255.0 * 1.2) * sc_l / pow(nc_l, 0.35);
                    double dl   = lat - blat, dn = lon - blon;
                    if (dn > PI)
                        dn -= DPI;
                    if (dn < -PI)
                        dn += DPI;
                    double dd   = sqrt(dl * dl + dn * dn * cos(lat) * cos(lat));
                    double blob = fmax(0.0, 1.0 - dd / brad);
                    blob_max    = fmax(blob_max, blob * blob);
                }
                double terra_totum  = terra_val + blob_max * 0.70;
                double limen_l      = p->res.aqua * 0.30;
                double dist_litoris = limen_l - terra_totum; /* >0 = sub aqua */

                double prof;
                if (dist_litoris < 0.05) {
                    /* vadosa: prope litus */
                    prof = dist_litoris / 0.05 * 0.3;
                } else {
                    prof = 0.3 + (p->res.aqua_profunditas * 0.5 + depth_var * 0.2);
                }
                prof = fmax(0.0, fmin(1.0, prof));
                col  = color_aquae(prof, var);

                /* micro undae oceani */
                double wave = strepitus2(nx * 30.0 + 91.0, ny * 30.0 + 97.0);
                col.r *= 0.95 + 0.05 * wave;
                col.g *= 0.95 + 0.05 * wave;
                col.b *= 0.96 + 0.04 * wave;
            }

            /* calottae polares */
            if (p->res.polaris > 0.01) {
                double abs_lat = fabs(lat) / (PI * 0.5);
                double lim     = 1.0 - p->res.polaris;
                double gn      = fbm(nx * 2.5, ny * 2.5, 4, 0.5) * 0.1;
                if (abs_lat > lim - gn) {
                    double t = fmin(1.0, (abs_lat - lim + gn) * 8.0);
                    color_t ice = {0.90, 0.92, 0.96, 1.0};
                    if (p->res.glacies_co2 > p->res.glacies)
                        ice = (color_t){0.94, 0.93, 0.91, 1.0};
                    col = miscere(col, ice, t);
                }
            }

            /* nubes */
            if (p->res.nubes > 0.01 && p->res.pressio_kPa > 0.3) {
                double cn = fbm_warp(nx * 1.0 + 50.0, ny * 1.0 + 50.0, 7, 0.55, 2.0);
                /* stratum secundum nubium minorum */
                double cn2 = fbm(nx * 2.5 + 80.0, ny * 2.5 + 80.0, 5, 0.5);
                cn         = cn * 0.65 + cn2 * 0.35;

                double thr = 1.0 - p->res.nubes * 0.6;
                if (cn > thr) {
                    double t = (cn - thr) / fmax(0.01, 1.0 - thr);
                    t        = t * t; /* margines molles */
                    /* color nubium ex compositione atmosphaerae */
                    color_t nub = {0.95, 0.95, 0.97, 1.0};  /* H₂O default */
                    if (p->res.co2 > 0.5)
                        nub = (color_t){0.94, 0.88, 0.68, 1.0}; /* H₂SO₄ Venus */
                    if (p->res.ch4 > 0.01)
                        nub = (color_t){0.93, 0.95, 0.98, 1.0}; /* CH₄ cirrus */
                    col = miscere(col, nub, t * 0.70);
                }
            }

            /* atmosphaera limbus: crescit cum pressione */
            if (pnorm > 0.003) {
                double dx   = (px - SEMI) / (rad * SEMI);
                double dy   = (py - SEMI) / (rad * SEMI);
                double edge = sqrt(dx * dx + dy * dy);
                double atm  = pow(edge, 4.0) * pnorm * 0.6;
                col         = miscere(col, atm_col, fmin(0.7, atm));
            }

            fen_pixel(fen, px, py, col.r, col.g, col.b, 1.0);
        }
    }

    /* atmospheric halo outside disk */
    if (pnorm > 0.02) {
        double halo_ext = rad * (1.0 + pnorm * 0.07);
        for (int py = 0; py < FEN; py++) {
            for (int px = 0; px < FEN; px++) {
                double dx       = (px - SEMI) / (halo_ext * SEMI);
                double dy       = (py - SEMI) / (halo_ext * SEMI);
                double r2       = dx * dx + dy * dy;
                double interior = rad / halo_ext;
                if (r2 < interior * interior || r2 >= 1.0)
                    continue;
                double d  = sqrt(r2);
                double al = (1.0 - (d - interior) / (1.0 - interior));
                al        = al * al * al * pnorm * 0.4;
                double il = 1.0;
                al *= fmax(0.1, il);
                fen_pixel(fen, px, py, atm_col.r, atm_col.g, atm_col.b, al);
            }
        }
    }
}

void saxosum_ex_ison(saxosum_t *v, const char *ison)
{
    v->pro.radius     = ison_f(ison, "planetella.radius", 0.9);
    v->pro.inclinatio = ison_f(ison, "planetella.inclinatio", 0.0);
    v->pro.rotatio    = ison_f(ison, "planetella.rotatio", 0.0);
    v->pro.semen      = (unsigned)ison_f(ison, "planetella.semen", 42);
    v->res.silicata         = ison_f(ison, "saxosculum.silicata", 0.0);
    v->res.ferrum           = ison_f(ison, "saxosculum.ferrum", 0.0);
    v->res.sulphur          = ison_f(ison, "saxosculum.sulphur", 0.0);
    v->res.carbo            = ison_f(ison, "saxosculum.carbo", 0.0);
    v->res.glacies          = ison_f(ison, "saxosculum.glacies", 0.0);
    v->res.glacies_co2      = ison_f(ison, "saxosculum.glacies_co2", 0.0);
    v->res.malachita        = ison_f(ison, "saxosculum.malachita", 0.0);
    v->res.aqua             = ison_f(ison, "saxosculum.aqua", 0.0);
    v->res.aqua_profunditas = ison_f(ison, "saxosculum.aqua_profunditas", 0.5);
    v->res.continentes      = (int)ison_f(ison, "saxosculum.continentes", 0);
    v->res.scala            = ison_f(ison, "saxosculum.scala", 1.0);
    v->res.tectonica        = ison_f(ison, "saxosculum.tectonica", 0.3);
    v->res.craterae         = ison_f(ison, "saxosculum.craterae", 0.0);
    v->res.maria            = ison_f(ison, "saxosculum.maria", 0.0);
    v->res.vulcanismus      = ison_f(ison, "saxosculum.vulcanismus", 0.0);
    v->res.pressio_kPa      = ison_f(ison, "saxosculum.pressio_kPa", 0.0);
    v->res.n2               = ison_f(ison, "saxosculum.n2", 0.0);
    v->res.o2               = ison_f(ison, "saxosculum.o2", 0.0);
    v->res.co2              = ison_f(ison, "saxosculum.co2", 0.0);
    v->res.ch4              = ison_f(ison, "saxosculum.ch4", 0.0);
    v->res.h2               = ison_f(ison, "saxosculum.h2", 0.0);
    v->res.he               = ison_f(ison, "saxosculum.he", 0.0);
    v->res.nh3              = ison_f(ison, "saxosculum.nh3", 0.0);
    v->res.pulvis           = ison_f(ison, "saxosculum.pulvis", 0.0);
    v->res.nubes            = ison_f(ison, "saxosculum.nubes", 0.0);
    v->res.polaris          = ison_f(ison, "saxosculum.polaris", 0.0);
}

void saxosum_in_ison(FILE *f, const saxosum_t *s)
{
    fprintf(f, "{\"planetella\": {\"radius\": %.2f, \"inclinatio\": %.3f, \"rotatio\": %.1f, \"semen\": %u}",
        s->pro.radius, s->pro.inclinatio, s->pro.rotatio, s->pro.semen);
    fprintf(f, ", \"saxosculum\": {\"silicata\": %.2f, \"ferrum\": %.2f, \"sulphur\": %.2f, \"carbo\": %.2f, \"glacies\": %.2f, \"glacies_co2\": %.2f, \"malachita\": %.2f, \"aqua\": %.2f, \"aqua_profunditas\": %.1f, \"continentes\": %d, \"scala\": %.1f, \"tectonica\": %.1f, \"craterae\": %.2f, \"maria\": %.2f, \"vulcanismus\": %.1f, \"pressio_kPa\": %.1f, \"n2\": %.3f, \"o2\": %.3f, \"co2\": %.3f, \"ch4\": %.3f, \"h2\": %.3f, \"he\": %.3f, \"nh3\": %.3f, \"pulvis\": %.1f, \"nubes\": %.2f, \"polaris\": %.2f}}",
        s->res.silicata, s->res.ferrum, s->res.sulphur, s->res.carbo,
        s->res.glacies, s->res.glacies_co2, s->res.malachita,
        s->res.aqua, s->res.aqua_profunditas, s->res.continentes, s->res.scala,
        s->res.tectonica, s->res.craterae, s->res.maria, s->res.vulcanismus,
        s->res.pressio_kPa, s->res.n2, s->res.o2, s->res.co2, s->res.ch4,
        s->res.h2, s->res.he, s->res.nh3, s->res.pulvis, s->res.nubes, s->res.polaris);
}
