/*
 * planeta.c — renderer planetarum et lunarum
 *
 * Colores derivantur ex compositione chimica et physica
 * atmosphaerica. Nullae overrides colorum.
 *
 * Unitates:
 *   pressio_kPa      kilopascal (Terra = 101.325 kPa)
 *   inclinatio        radiani
 *   rotatio           radiani
 *   compositiones     fractiones [0, 1]
 *   craterae          densitas [0, 1] (1 = Callisto)
 *   vulcanismus       activitas [0, 1] (1 = Io)
 *   tectonica         complexitas [0, 1]
 *   aqua              fractio superficiei [0, 1]
 *   aqua_profunditas  fractio [0, 1] (0 = vadosa, 1 = abyssalis)
 *   polaris           fractio superficiei [0, 1]
 */

#include "planeta.h"
#include "ison.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEN PLANETA_FENESTRA
#define SEMI PLANETA_SEMI
#define PI 3.14159265358979323846
#define DPI (2.0 * PI)

/* ================================================================
 * strepitus proceduralis
 * ================================================================ */

static double hash2(int ix, int iy)
{
    unsigned h = (unsigned)(ix * 374761393 + iy * 668265263);
    h = (h ^ (h >> 13)) * 1274126177;
    h ^= h >> 16;
    return (h & 0xFFFFFF) / 16777216.0;
}

static double hermite(double t) { return t * t * (3.0 - 2.0 * t); }

static double strepitus2(double x, double y)
{
    int ix    = (int)floor(x), iy = (int)floor(y);
    double fx = x - ix, fy = y - iy;
    double sx = hermite(fx), sy = hermite(fy);
    double a  = hash2(ix, iy) + (hash2(ix + 1, iy) - hash2(ix, iy)) * sx;
    double b  = hash2(ix, iy + 1) + (hash2(ix + 1, iy + 1) - hash2(ix, iy + 1)) * sx;
    return a + (b - a) * sy;
}

static double fbm(double x, double y, int oct, double pers)
{
    double val = 0.0, amp = 1.0, freq = 1.0, mx = 0.0;
    for (int i = 0; i < oct; i++) {
        val += amp * strepitus2(x * freq, y * freq);
        mx += amp;
        amp *= pers;
        freq *= 2.0;
    }
    return val / mx;
}

static double fbm_warp(double x, double y, int oct, double pers, double w)
{
    double dx = fbm(x + 5.2, y + 1.3, oct, pers) * w;
    double dy = fbm(x + 9.7, y + 6.8, oct, pers) * w;
    return fbm(x + dx, y + dy, oct, pers);
}

static double ridged(double x, double y, int oct, double pers)
{
    double val = 0.0, amp = 1.0, freq = 1.0, mx = 0.0;
    for (int i = 0; i < oct; i++) {
        double n = 1.0 - fabs(strepitus2(x * freq, y * freq) * 2.0 - 1.0);
        val += amp * n * n;
        mx += amp;
        amp *= pers;
        freq *= 2.0;
    }
    return val / mx;
}

/* ================================================================
 * spectra materialia — RGB ex compositione chimica
 *
 * Reflectantiae spectroscopicae approximatae:
 *
 *   SiO₂ silicata:  regolith lunaris highlands, albedo ~0.12-0.30
 *     R=0.48 G=0.46 B=0.44 (calida grisea, leve roseum)
 *
 *   Fe₂O₃ ferrum:   Mars regolith, albedo ~0.15-0.25
 *     R=0.72 G=0.35 B=0.18 (rubrum/brunneum profundum)
 *
 *   S/SO₂ sulphur:  Io superficies, albedo ~0.40-0.60
 *     R=0.82 G=0.75 B=0.25 (flavum saturatum)
 *
 *   C carbo:         asteroidea C-typus, albedo ~0.03-0.10
 *     R=0.10 G=0.09 B=0.08 (fere nigrum)
 *
 *   H₂O glacies:    Europa, Enceladus, albedo ~0.60-0.99
 *     R=0.90 G=0.93 B=0.97 (album caerulescens)
 *
 *   CO₂ glacies:    Mars polus australis
 *     R=0.94 G=0.93 B=0.91 (album calidum)
 *
 *   Cu₂CO₃(OH)₂ malachita / chlorophyllum:  vegetatio
 *     R=0.20 G=0.55 B=0.12 (viridis saturatus)
 *
 * Referentiae:
 *   Hapke, B. (2012). "Theory of Reflectance and Emittance
 *     Spectroscopy." Cambridge Univ. Press. 2nd ed.
 *   Clark, R.N. et al. (2007). "USGS Digital Spectral Library
 *     splib06a." USGS Data Series 231.
 * ================================================================ */

/* spectra tabulata [R, G, B] — lucidiora quam ante */
static const double SPEC_SILICATA[3]  = {0.48, 0.46, 0.44};
static const double SPEC_FERRUM[3]    = {0.72, 0.35, 0.18};
static const double SPEC_SULPHUR[3]   = {0.82, 0.75, 0.25};
static const double SPEC_CARBO[3]     = {0.10, 0.09, 0.08};
static const double SPEC_GLACIES[3]   = {0.90, 0.93, 0.97};
static const double SPEC_CO2ICE[3]    = {0.94, 0.93, 0.91};
static const double SPEC_MALACHITA[3] = {0.20, 0.55, 0.12};

static color_t color_superficiei(
    double silicata, double ferrum, double sulphur, double carbo,
    double glacies, double glacies_co2, double malachita, double var
)
{
    /* variatio: strepitus localis [-0.5, 0.5] pro heterogeneitate */
    double r = 0.0, g = 0.0, b = 0.0;
    double weights[7];
    const double *spectra[7] = {
        SPEC_SILICATA, SPEC_FERRUM, SPEC_SULPHUR, SPEC_CARBO,
        SPEC_GLACIES, SPEC_CO2ICE, SPEC_MALACHITA
    };
    weights[0] = silicata;
    weights[1] = ferrum;
    weights[2] = sulphur;
    weights[3] = carbo;
    weights[4] = glacies;
    weights[5] = glacies_co2;
    weights[6] = malachita;

    double tot = 0.0;
    for (int i = 0; i < 7; i++)
        tot += weights[i];
    if (tot < 0.01)
        return (color_t){0.4, 0.38, 0.36, 1.0};

    for (int i = 0; i < 7; i++) {
        double w = weights[i] / tot;
        /* variatio per materiam: non omnes variantur aequaliter */
        double v = 1.0 + var * (i == 6 ? 0.4 : 0.2);  /* malachita magis variat */
        r += w * spectra[i][0] * v;
        g += w * spectra[i][1] * v;
        b += w * spectra[i][2] * v;
    }

    return (color_t){
        fmin(1.0, fmax(0.0, r)),
        fmin(1.0, fmax(0.0, g)),
        fmin(1.0, fmax(0.0, b)),
        1.0
    };
}

/* ================================================================
 * aqua — color ex profunditate et angulo
 *
 * Absorptio spectroscopica H₂O liquidae:
 *   λ > 600nm absorbitur fortiter → aqua apparet caerulea.
 *   Vadosa: arena/coralli visibilia → caeruleum-viride.
 *   Profunda: omnis rubrum/viridis absorbitur → caeruleum obscurum.
 *
 * Ref: Pope & Fry (1997). "Absorption spectrum (380-700 nm)
 *   of pure water." Applied Optics 36(33):8710-8723.
 * ================================================================ */

static color_t color_aquae(double prof, double var)
{
    /* prof [0,1]: 0 = vadosa litoralis, 1 = abyssalis */
    double altitudo = prof * 0.6 + var * 0.4;
    altitudo        = fmax(0.0, fmin(1.0, altitudo));
    return (color_t){
        0.02 + 0.15 * (1.0 - altitudo),
        0.08 + 0.28 * (1.0 - altitudo),
        0.25 + 0.45 * (1.0 - altitudo) * 0.6,
        1.0
    };
}

/* ================================================================
 * atmosphaera — color ex compositione gasea
 *
 * Rayleigh (1871): σ ∝ λ⁻⁴. N₂+O₂ → caelum caeruleum.
 * CH₄ absorptio: banda ad 619nm, 727nm → caeruleum forte.
 * H₂+He: per se incolores. Cum NH₃ nubibus → cremeum.
 * CO₂: incolor sed cum Fe₂O₃ pulvere → roseum (Mars).
 * Tholin (Sagan & Khare 1979): organica → aurantium (Titan).
 * ================================================================ */

static color_t color_atmosphaerae(
    double n2, double o2, double co2, double ch4,
    double h2, double he, double nh3, double pulvis,
    double ferrum, double sulphur
)
{
    double r = 0.0, g = 0.0, b = 0.0, tot = 0.0;

    /* N₂ + O₂ → Rayleigh caeruleum */
    double ray = n2 + o2;
    if (ray > 0.0) {
        r += ray * 0.40;
        g += ray * 0.60;
        b += ray * 1.00;
        tot += ray;
    }
    /* CO₂ → per se incolor; cum pulvere roseum (Mars) vel
     * cum H₂SO₄ nubibus flavum-album (Venus) */
    if (co2 > 0.0) {
        double dust = fmax(0.05, pulvis);
        r += co2 * (0.55 + dust * 0.30);
        g += co2 * (0.48 + dust * 0.15);
        b += co2 * (0.38);
        tot += co2;
    }
    /* CH₄ → absorptio ad 619nm, 727nm, 890nm → caeruleum forte.
     * Effectus non-linearis: etiam 2% CH₄ dat colorem profunde caeruleum
     * quia bandae absorptionis saturantur rapide (Beer-Lambert).
     * Uranus (2% CH₄) = cyan; Neptunus (1.5% CH₄ sed profundior) = caeruleum */
    if (ch4 > 0.0) {
        double ch4_eff = 1.0 - exp(-ch4 * 80.0); /* saturatio rapida */
        r += ch4_eff * 0.15;
        g += ch4_eff * 0.50;
        b += ch4_eff * 1.00;
        tot += ch4_eff;
    }
    /* H₂ + He → per se pallide; cum NH₃ nubibus → cremeum/aureum.
     * Gigantes gaseousi lucidi sunt (albedo ~0.5) */
    double hhe = h2 + he;
    if (hhe > 0.0) {
        double nh3_eff = fmin(1.0, nh3 * 50.0);
        r += hhe * (0.70 + nh3_eff * 0.20);
        g += hhe * (0.65 + nh3_eff * 0.15);
        b += hhe * (0.55 + nh3_eff * 0.00);
        tot += hhe;
    }
    /* NH₃ nubes: album/cremeum */
    if (nh3 > 0.0) {
        r += nh3 * 0.85;
        g += nh3 * 0.78;
        b += nh3 * 0.60;
        tot += nh3;
    }
    /* pulvis: Fe₂O₃ → roseum; sulphur → flavum */
    if (pulvis > 0.0) {
        r += pulvis * (0.50 + ferrum * 0.40 + sulphur * 0.30);
        g += pulvis * (0.35 + sulphur * 0.30);
        b += pulvis * (0.25);
        tot += pulvis;
    }

    if (tot < 0.01)
        return (color_t){0.5, 0.6, 0.8, 1.0};
    return (color_t){
        fmin(1.0, r / tot),
        fmin(1.0, g / tot),
        fmin(1.0, b / tot), 1.0
    };
}

/* ================================================================
 * geometria sphaerica
 * ================================================================ */

static int pixel_ad_sphaeram(
    int px, int py, double radius,
    double inclinatio, double rotatio,
    double *lon, double *lat
) {
    double dx = (px - SEMI) / (radius * SEMI);
    double dy = (py - SEMI) / (radius * SEMI);
    double r2 = dx * dx + dy * dy;
    if (r2 >= 1.0)
        return 0;
    double dz = sqrt(1.0 - r2);
    double ci = cos(inclinatio), si = sin(inclinatio);
    double y2 = dy * ci - dz * si;
    double z2 = dy * si + dz * ci;
    *lat      = asin(fmax(-1.0, fmin(1.0, y2)));
    *lon      = atan2(dx, z2) + rotatio;
    *lon      = fmod(*lon, DPI);
    if (*lon < 0)
        *lon += DPI;
    return 1;
}

static double illuminatio(
    int px, int py, double radius,
    double situs, double angulus
) {
    double dx = (px - SEMI) / (radius * SEMI);
    double dy = (py - SEMI) / (radius * SEMI);
    double r2 = dx * dx + dy * dy;
    if (r2 >= 1.0)
        return 0.0;
    double dz = sqrt(1.0 - r2);
    /* situs=0 → luce plena (sol post oculum), situs=0.5 → dimidius, situs=1 → novus */
    double lx    = sin(situs * PI) * cos(angulus);
    double ly    = sin(situs * PI) * sin(angulus);
    double lz    = cos(situs * PI);
    double ndotl = dx * lx + dy * ly + dz * lz;
    if (ndotl < 0.0)
        ndotl = 0.0;
    ndotl *= (0.60 + 0.40 * dz); /* limb darkening */
    return ndotl;
}

static void fen_pixel(
    unsigned char *fen, int x, int y,
    double r, double g, double b, double a
) {
    if (x < 0 || x >= FEN || y < 0 || y >= FEN)
        return;
    int idx   = (y * FEN + x) * 4;
    double oa = fen[idx + 3] / 255.0;
    double na = a + oa * (1.0 - a);
    if (na < 0.001)
        return;
    fen[idx + 0] = (unsigned char)(fmin(255.0, (r * a + fen[idx + 0] / 255.0 * oa * (1.0 - a)) / na * 255.0));
    fen[idx + 1] = (unsigned char)(fmin(255.0, (g * a + fen[idx + 1] / 255.0 * oa * (1.0 - a)) / na * 255.0));
    fen[idx + 2] = (unsigned char)(fmin(255.0, (b * a + fen[idx + 2] / 255.0 * oa * (1.0 - a)) / na * 255.0));
    fen[idx + 3] = (unsigned char)(fmin(255.0, na * 255.0));
}

/* miscere() nunc in color.h */

/* ================================================================
 * craterae — morphologia detallata
 *
 * Morphologia crateris (Melosh 1989, "Impact Cratering"):
 * - simplex (d < 15 km): excavatio simplex, profunditas/diametrum ~ 1/5
 * - complex (d > 15 km): pavimentum planum, mons centralis, parietes gradati
 * - basin (d > 200 km): multiple rings (Caloris, Imbrium)
 *
 * Hic tres scalae simulantur per layers: parvae, mediae, magnae.
 * Unaquaeque habet: excavatio, margo, eiecta radialia, mons centralis.
 * Irregularitas formae per strepitum angulare.
 *
 * Ref: Melosh, H.J. (1989). "Impact Cratering: A Geologic Process."
 *   Oxford Univ. Press.
 * ================================================================ */

static double crater_field(
    double lon, double lat, double densitas,
    unsigned sem, double *bright_out
) {
    *bright_out = 0.0;
    if (densitas < 0.01)
        return 0.0;

    double total_d = 0.0, total_b = 0.0;
    unsigned s     = sem;

    /* tres scalae: parvae (multi), mediae, magnae (paucae) */
    int numeri[3] = {
        (int)(densitas * 40) + 5,
        (int)(densitas * 15) + 2,
        (int)(densitas * 4) + 1
    };
    double magnitudines[3] = {0.025, 0.06, 0.15};

    for (int stratum = 0; stratum < 3; stratum++) {
        for (int i = 0; i < numeri[stratum]; i++) {
            s = s * 1103515245 + 12345;
            double clon = (s >> 8 & 0xFFFF) / 65536.0 * DPI;
            s = s * 1103515245 + 12345;
            double clat = asin((s >> 8 & 0xFFFF) / 65536.0 * 2.0 - 1.0);
            s = s * 1103515245 + 12345;
            double crad = magnitudines[stratum] * (0.4 + (s >> 8 & 0xFF) / 255.0 * 0.8);
            s = s * 1103515245 + 12345;
            double age = (s >> 8 & 0xFF) / 255.0;
            s = s * 1103515245 + 12345;
            double irreg = 0.1 + (s >> 8 & 0xFF) / 255.0 * 0.25;

            double dlat = lat - clat;
            double dlon = lon - clon;
            if (dlon > PI)
                dlon -= DPI;
            if (dlon < -PI)
                dlon += DPI;
            double cl = cos(lat);
            double d  = sqrt(dlat * dlat + dlon * dlon * cl * cl);

            /* deformatio angularis — non perfecte circularis */
            double ang = atan2(dlat, dlon * cl);
            double deform = 1.0 + irreg * sin(ang * 3.0 + clon * 7.0)
                + irreg * 0.4 * sin(ang * 5.0 + clat * 11.0)
                + irreg * 0.2 * sin(ang * 8.0 + clon * 3.0);
            d *= deform;

            if (d > crad * 2.0)
                continue;
            double t = d / crad;

            double profunditas_c = 0.0, luciditas = 0.0;
            double recens        = 1.0 - age * 0.7;

            if (t < 0.12 && stratum >= 1) {
                /* mons centralis (crateribus complexis solum) */
                double mt     = t / 0.12;
                profunditas_c = 0.20 * (1.0 - mt * mt) * recens;
                luciditas     = 0.12 * (1.0 - mt) * recens;
            } else if (t < 0.65) {
                /* pavimentum — excavatio */
                profunditas_c = -0.40 * recens * (1.0 - 0.3 * age);
                /* textura pavimenti: lava infusa in antiquis */
                double pav_tex = strepitus2(lon * 50.0, lat * 50.0);
                profunditas_c += pav_tex * 0.05 * age;
            } else if (t < 1.0) {
                /* margo — paries elevatus */
                double mt     = (t - 0.65) / 0.35;
                double paries = sin(mt * PI);
                profunditas_c = 0.35 * paries * recens;
                luciditas     = 0.18 * paries * recens;
                /* gradus in crateribus complexis */
                if (stratum >= 1) {
                    double gradus = sin(mt * PI * 3.0) * 0.06;
                    profunditas_c += gradus * recens;
                }
            } else if (t < 1.6) {
                /* eiecta — radii et tegmentum */
                double et       = (t - 1.0) / 0.6;
                double n_radii  = 5.0 + stratum * 4.0;
                double radius_e = sin(ang * n_radii + clon * 13.0);
                radius_e        = fmax(0.0, radius_e);
                radius_e *= radius_e;
                double decasus = (1.0 - et) * (1.0 - et);
                luciditas      = 0.15 * radius_e * decasus * recens;
                profunditas_c  = 0.03 * decasus * (1.0 - radius_e * 0.5);
                /* tegmentum eiecta continuum */
                luciditas += 0.04 * decasus * recens;
            }

            total_d += profunditas_c;
            total_b += luciditas;
        }
    }

    *bright_out = total_b;
    return total_d;
}

/* ================================================================
 * temperatura → RGB (Tanner Helland 2012, errore < 1% in 1000-40000K)
 * ================================================================ */

static color_t temperatura_ad_colorem_f(double kelvin)
{
    double t = kelvin / 100.0;
    double r, g, b;

    r = (t <= 66.0) ? 1.0
        : fmin(1.0, fmax(0.0, 329.698727446 * pow(t - 60.0, -0.1332047592) / 255.0));

    g = (t <= 66.0)
        ? fmin(1.0, fmax(0.0, (99.4708025861 * log(t) - 161.1195681661) / 255.0))
        : fmin(1.0, fmax(0.0, 288.1221695283 * pow(t - 60.0, -0.0755148492) / 255.0));

    b = (t >= 66.0) ? 1.0
        : (t <= 19.0) ? 0.0
        : fmin(1.0, fmax(0.0, (138.5177312231 * log(t - 10.0) - 305.0447927307) / 255.0));

    return (color_t){r, g, b, 1.0};
}

/*
 * temperatura_ex_compositione — temperatura effectiva ex H/He/CH4/NH3.
 * Approximatio: corpus H-He cum fusione ~ 5000-30000K ex densitate.
 * Sine fusione ~ temperatura effeciva nubium (100-1000K).
 * Usus: si temperatura == 0, derivatur ex h2+he fractione.
 */
static double temperatura_ex_compositione(double h2, double he, double ch4)
{
    double h_tot = h2 + he;
    /* 5000K (nana rubra) .. 25000K (B stella) pro fractionibus H-He */
    if (h_tot > 0.5)
        return 5000.0 + h_tot * 20000.0;
    if (ch4 > 0.1)
        return 1200.0 + ch4 * 2000.0;  /* nana brunnea methanosa */
    return 4000.0;
}

/* ================================================================
 * ISON auxiliarium
 * ================================================================ */

static double ison_f(const char *ison, const char *via, double praef)
{
    return ison_da_f(ison, via, praef);
}

/* ================================================================
 * renderers et parsores per genus — inclusi ex tabellis separatis
 * ================================================================ */

#include "planetae/saxosum.c"
#include "planetae/gaseosum.c"
#include "planetae/glaciale.c"
#include "planetae/parvum.c"
#include "planetae/sol.c"
#include "planetae/nebula.c"

/* ================================================================
 * dispatcher
 * ================================================================ */

void planeta_reddere(
    unsigned char *fenestra, const planeta_t *planeta,
    const planeta_perceptus_t *perceptus
) {
    memset(fenestra, 0, FEN * FEN * 4);
    switch (planeta->qui) {
    case PLANETA_SAXOSUM:  reddere_saxosum(fenestra, &planeta->ubi.saxosum, perceptus);  break;
    case PLANETA_GASEOSUM: reddere_gaseosum(fenestra, &planeta->ubi.gaseosum, perceptus); break;
    case PLANETA_GLACIALE: reddere_glaciale(fenestra, &planeta->ubi.glaciale, perceptus); break;
    case PLANETA_PARVUM:   reddere_parvum(fenestra, &planeta->ubi.parvum, perceptus);   break;
    case PLANETA_SOL:      reddere_sol(fenestra, &planeta->ubi.sol);                     break;
    case PLANETA_NEBULA:   reddere_nebula(fenestra, &planeta->ubi.nebula);               break;
    }
}

/* ================================================================
 * ISON parser
 * ================================================================ */

planeta_t *planeta_ex_ison(const char *ison)
{
    /* detege genus ex clavibus praesentibus */
    planetarius_t g = PLANETA_SAXOSUM;
    static const struct { const char *clavis; planetarius_t genus; } detectio[] = {
        {"soliculum",   PLANETA_SOL},
        {"nebulula",    PLANETA_NEBULA},
        {"gaseosculum", PLANETA_GASEOSUM},
        {"glaciellum",  PLANETA_GLACIALE},
        {"parvulum",    PLANETA_PARVUM},
    };
    for (int i = 0; i < 5; i++) {
        char *tmp = ison_da_crudum(ison, detectio[i].clavis);
        if (tmp) { free(tmp); g = detectio[i].genus; break; }
    }

    switch (g) {
    case PLANETA_SAXOSUM:  return saxosum_ex_ison(ison);
    case PLANETA_GASEOSUM: return gaseosum_ex_ison(ison);
    case PLANETA_GLACIALE: return glaciale_ex_ison(ison);
    case PLANETA_PARVUM:   return parvum_ex_ison(ison);
    case PLANETA_SOL:      return sol_ex_ison(ison);
    case PLANETA_NEBULA:   return nebula_ex_ison(ison);
    default:               return NULL;
    }
}

/* ================================================================
 * atmosphaera → instrumentum
 *
 * Mutat instrumentum basale superponendo effectus atmosphaericos.
 * Seeing additur in quadratura (fontes independentes).
 *
 * Unitates derivatae:
 *   pressio_kPa / 101 → normalized [0,1]
 *   seeing       = norm² × 3.0    [pixels]
 *   scintillatio = norm × 0.4     [amplitudo]
 *   caeli_lumen  = norm² × 0.15   [intensitas]
 *   refractio    = norm × 1.5     [pixels]
 * ================================================================ */

void planeta_instrumentum_applicare(
    double pressio_kPa,
    instrumentum_t *instr
) {
    double a = fmin(1.0, pressio_kPa / 101.0);
    if (a < 0.005)
        return;

    double seeing = a * a * 3.0;
    double v2     = instr->visio * instr->visio + seeing * seeing;
    instr->visio  = sqrt(v2);

    instr->scintillatio += a * 0.4;
    if (instr->scintillatio > 1.0)
        instr->scintillatio = 1.0;

    instr->caeli_lumen += a * a * 0.15;
    instr->refractio += a * 1.5;

    double desat = 1.0 - a * 0.15;
    if (desat < 0.3)
        desat = 0.3;
    instr->saturatio *= desat;
}
