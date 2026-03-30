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
    int ix = (int)floor(x), iy = (int)floor(y);
    double fx = x - ix, fy = y - iy;
    double sx = hermite(fx), sy = hermite(fy);
    double a = hash2(ix, iy) + (hash2(ix + 1, iy) - hash2(ix, iy)) * sx;
    double b = hash2(ix, iy + 1) + (hash2(ix + 1, iy + 1) - hash2(ix, iy + 1)) * sx;
    return a + (b - a) * sy;
}

static double fbm(double x, double y, int oct, double pers)
{
    double val = 0.0, amp = 1.0, freq = 1.0, mx = 0.0;
    for (int i = 0; i < oct; i++) {
        val += amp * strepitus2(x * freq, y * freq);
        mx += amp; amp *= pers; freq *= 2.0;
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
        mx += amp; amp *= pers; freq *= 2.0;
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
static const double SPEC_SILICATA[3] = {0.48, 0.46, 0.44};
static const double SPEC_FERRUM[3]   = {0.72, 0.35, 0.18};
static const double SPEC_SULPHUR[3]  = {0.82, 0.75, 0.25};
static const double SPEC_CARBO[3]    = {0.10, 0.09, 0.08};
static const double SPEC_GLACIES[3]  = {0.90, 0.93, 0.97};
static const double SPEC_CO2ICE[3]   = {0.94, 0.93, 0.91};
static const double SPEC_MALACHITA[3]= {0.20, 0.55, 0.12};

static color_t color_superficiei(const planeta_t *p, double var)
{
    /* variatio: strepitus localis [-0.5, 0.5] pro heterogeneitate */
    double r = 0.0, g = 0.0, b = 0.0;
    double weights[7];
    const double *spectra[7] = {
        SPEC_SILICATA, SPEC_FERRUM, SPEC_SULPHUR, SPEC_CARBO,
        SPEC_GLACIES, SPEC_CO2ICE, SPEC_MALACHITA
    };
    weights[0] = p->silicata;
    weights[1] = p->ferrum;
    weights[2] = p->sulphur;
    weights[3] = p->carbo;
    weights[4] = p->glacies;
    weights[5] = p->glacies_co2;
    weights[6] = p->malachita;

    double tot = 0.0;
    for (int i = 0; i < 7; i++) tot += weights[i];
    if (tot < 0.01) return (color_t){0.4, 0.38, 0.36, 1.0};

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
    altitudo = fmax(0.0, fmin(1.0, altitudo));
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

static color_t color_atmosphaerae(const planeta_t *p)
{
    double r = 0.0, g = 0.0, b = 0.0, tot = 0.0;

    /* N₂ + O₂ → Rayleigh caeruleum */
    double ray = p->n2 + p->o2;
    if (ray > 0.0) {
        r += ray * 0.40; g += ray * 0.60; b += ray * 1.00;
        tot += ray;
    }
    /* CO₂ → per se incolor; cum pulvere roseum (Mars) vel
     * cum H₂SO₄ nubibus flavum-album (Venus) */
    if (p->co2 > 0.0) {
        double dust = fmax(0.05, p->pulvis);
        r += p->co2 * (0.55 + dust * 0.30);
        g += p->co2 * (0.48 + dust * 0.15);
        b += p->co2 * (0.38);
        tot += p->co2;
    }
    /* CH₄ → absorptio ad 619nm, 727nm, 890nm → caeruleum forte.
     * Effectus non-linearis: etiam 2% CH₄ dat colorem profunde caeruleum
     * quia bandae absorptionis saturantur rapide (Beer-Lambert).
     * Uranus (2% CH₄) = cyan; Neptunus (1.5% CH₄ sed profundior) = caeruleum */
    if (p->ch4 > 0.0) {
        double ch4_eff = 1.0 - exp(-p->ch4 * 80.0); /* saturatio rapida */
        r += ch4_eff * 0.15; g += ch4_eff * 0.50; b += ch4_eff * 1.00;
        tot += ch4_eff;
    }
    /* H₂ + He → per se pallide; cum NH₃ nubibus → cremeum/aureum.
     * Gigantes gaseousi lucidi sunt (albedo ~0.5) */
    double hhe = p->h2 + p->he;
    if (hhe > 0.0) {
        double nh3_eff = fmin(1.0, p->nh3 * 50.0);
        r += hhe * (0.70 + nh3_eff * 0.20);
        g += hhe * (0.65 + nh3_eff * 0.15);
        b += hhe * (0.55 + nh3_eff * 0.00);
        tot += hhe;
    }
    /* NH₃ nubes: album/cremeum */
    if (p->nh3 > 0.0) {
        r += p->nh3 * 0.85; g += p->nh3 * 0.78; b += p->nh3 * 0.60;
        tot += p->nh3;
    }
    /* pulvis: Fe₂O₃ → roseum; sulphur → flavum */
    if (p->pulvis > 0.0) {
        r += p->pulvis * (0.50 + p->ferrum * 0.40 + p->sulphur * 0.30);
        g += p->pulvis * (0.35 + p->sulphur * 0.30);
        b += p->pulvis * (0.25);
        tot += p->pulvis;
    }

    if (tot < 0.01) return (color_t){0.5, 0.6, 0.8, 1.0};
    return (color_t){
        fmin(1.0, r / tot),
        fmin(1.0, g / tot),
        fmin(1.0, b / tot), 1.0
    };
}

/* ================================================================
 * geometria sphaerica
 * ================================================================ */

static int pixel_ad_sphaeram(int px, int py, double radius,
                             double inclinatio, double rotatio,
                             double *lon, double *lat)
{
    double dx = (px - SEMI) / (radius * SEMI);
    double dy = (py - SEMI) / (radius * SEMI);
    double r2 = dx * dx + dy * dy;
    if (r2 >= 1.0) return 0;
    double dz = sqrt(1.0 - r2);
    double ci = cos(inclinatio), si = sin(inclinatio);
    double y2 = dy * ci - dz * si;
    double z2 = dy * si + dz * ci;
    *lat = asin(fmax(-1.0, fmin(1.0, y2)));
    *lon = atan2(dx, z2) + rotatio;
    *lon = fmod(*lon, DPI);
    if (*lon < 0) *lon += DPI;
    return 1;
}

static double illuminatio(int px, int py, double radius,
                          double phase, double angulus_phase)
{
    double dx = (px - SEMI) / (radius * SEMI);
    double dy = (py - SEMI) / (radius * SEMI);
    double r2 = dx * dx + dy * dy;
    if (r2 >= 1.0) return 0.0;
    double dz = sqrt(1.0 - r2);
    double lx = -cos(phase * PI) * cos(angulus_phase);
    double ly = -cos(phase * PI) * sin(angulus_phase);
    double lz = sin(phase * PI) * 0.3;
    double ndotl = dx * lx + dy * ly + dz * lz;
    if (ndotl < 0.0) ndotl = 0.0;
    ndotl = ndotl * 0.75 + 0.25;  /* half-Lambert, umbrae profundiores */
    ndotl *= (0.60 + 0.40 * dz); /* limb darkening */
    return ndotl;
}

static void fen_pixel(unsigned char *fen, int x, int y,
                      double r, double g, double b, double a)
{
    if (x < 0 || x >= FEN || y < 0 || y >= FEN) return;
    int idx = (y * FEN + x) * 4;
    double oa = fen[idx + 3] / 255.0;
    double na = a + oa * (1.0 - a);
    if (na < 0.001) return;
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

static double crater_field(double lon, double lat, double densitas,
                           unsigned sem, double *bright_out)
{
    *bright_out = 0.0;
    if (densitas < 0.01) return 0.0;

    double total_d = 0.0, total_b = 0.0;
    unsigned s = sem;

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
            if (dlon > PI) dlon -= DPI;
            if (dlon < -PI) dlon += DPI;
            double cl = cos(lat);
            double d = sqrt(dlat * dlat + dlon * dlon * cl * cl);

            /* deformatio angularis — non perfecte circularis */
            double ang = atan2(dlat, dlon * cl);
            double deform = 1.0 + irreg * sin(ang * 3.0 + clon * 7.0)
                                + irreg * 0.4 * sin(ang * 5.0 + clat * 11.0)
                                + irreg * 0.2 * sin(ang * 8.0 + clon * 3.0);
            d *= deform;

            if (d > crad * 2.0) continue;
            double t = d / crad;

            double profunditas_c = 0.0, luciditas = 0.0;
            double recens = 1.0 - age * 0.7;

            if (t < 0.12 && stratum >= 1) {
                /* mons centralis (crateribus complexis solum) */
                double mt = t / 0.12;
                profunditas_c = 0.20 * (1.0 - mt * mt) * recens;
                luciditas = 0.12 * (1.0 - mt) * recens;
            } else if (t < 0.65) {
                /* pavimentum — excavatio */
                profunditas_c = -0.40 * recens * (1.0 - 0.3 * age);
                /* textura pavimenti: lava infusa in antiquis */
                double pav_tex = strepitus2(lon * 50.0, lat * 50.0);
                profunditas_c += pav_tex * 0.05 * age;
            } else if (t < 1.0) {
                /* margo — paries elevatus */
                double mt = (t - 0.65) / 0.35;
                double paries = sin(mt * PI);
                profunditas_c = 0.35 * paries * recens;
                luciditas = 0.18 * paries * recens;
                /* gradus in crateribus complexis */
                if (stratum >= 1) {
                    double gradus = sin(mt * PI * 3.0) * 0.06;
                    profunditas_c += gradus * recens;
                }
            } else if (t < 1.6) {
                /* eiecta — radii et tegmentum */
                double et = (t - 1.0) / 0.6;
                double n_radii = 5.0 + stratum * 4.0;
                double radius_e = sin(ang * n_radii + clon * 13.0);
                radius_e = fmax(0.0, radius_e);
                radius_e *= radius_e;
                double decasus = (1.0 - et) * (1.0 - et);
                luciditas = 0.15 * radius_e * decasus * recens;
                profunditas_c = 0.03 * decasus * (1.0 - radius_e * 0.5);
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
 * continentes — terrain procedurale
 *
 * Continentes procedurales per multi-octave FBM cum domain warping.
 * Numerus continentium controllat distributionem terrae:
 *   0: nulla terra (oceanus totus)
 *   1: pangaea (una massa magna)
 *   2-3: continentes magni
 *   5-7: Terra-simile
 *   >10: archipelagus
 *
 * Tectonica: complexitas litoralis (plate boundaries, fjords).
 * Scala: magnitudo featurum.
 *
 * Ref: Perlin, K. (1985). "An image synthesizer."
 *   SIGGRAPH '85. pp. 287-296.
 * ================================================================ */

static double continent_mask(double lon, double lat, const planeta_t *p)
{
    if (p->aqua < 0.01) return 1.0; /* nulla aqua → tota terra */

    double sc = fmax(0.3, p->scala);
    double ox = (p->semen & 0xFF) * 0.13;
    double oy = ((p->semen >> 8) & 0xFF) * 0.13;

    /* blobs continentales — massae magnae terrae */
    double elevatio = 0.0;
    unsigned s = p->semen * 7919 + 77777;
    int nc = fmax(1, p->continentes);
    for (int c = 0; c < nc; c++) {
        s = s * 1103515245 + 12345;
        double blon = (s >> 8 & 0xFFFF) / 65536.0 * DPI;
        s = s * 1103515245 + 12345;
        double blat = ((s >> 8 & 0xFFFF) / 65536.0 - 0.5) * PI * 0.85;
        s = s * 1103515245 + 12345;
        double brad = (1.0 + (s >> 8 & 0xFF) / 255.0 * 1.2) * sc / pow(nc, 0.35);

        double dl = lat - blat;
        double dn = lon - blon;
        if (dn > PI) dn -= DPI;
        if (dn < -PI) dn += DPI;
        double dd = sqrt(dl * dl + dn * dn * cos(lat) * cos(lat));
        double blob = fmax(0.0, 1.0 - dd / brad);
        blob = blob * blob;
        elevatio = fmax(elevatio, blob);
    }

    /* detallum litorale per FBM */
    double nx = lon / DPI * 5.0 * sc;
    double ny = (lat / PI + 0.5) * 2.5 * sc;
    double warp = 1.0 + p->tectonica * 3.0;
    double det = fbm_warp(nx + ox, ny + oy, 7, 0.55, warp);

    /* combinatio */
    double terrain = elevatio * 0.70 + det * 0.30;

    /* limen maris: aqua est fractio sub aqua.
     * terrain ∈ [0, ~0.9]. Invertimus: terra ubi terrain altior.
     * Exemplum: aqua=0.71 → ~29% pixels supra limen. */
    double limen = p->aqua * 0.30;
    return (terrain > limen) ? 1.0 : 0.0;
}

/* ================================================================
 * renderers
 * ================================================================ */

static void reddere_saxosum(unsigned char *fen, const planeta_t *p)
{
    double rad = p->radius;
    double pnorm = fmin(1.0, p->pressio_kPa / 101.0); /* [0,1] normalized */
    color_t atm_col = color_atmosphaerae(p);

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double lon, lat;
            if (!pixel_ad_sphaeram(px, py, rad, p->inclinatio, p->rotatio,
                                   &lon, &lat))
                continue;

            double illum = illuminatio(px, py, rad, p->phase, p->angulus_phase);
            if (illum < 0.003) continue;

            /* coordinatae strepitus — scala pro detallo ad 256px */
            double nx = lon / DPI * 10.0;
            double ny = (lat / PI + 0.5) * 5.0;

            /* variatio localis [-0.3, 0.3] */
            double var = (fbm(nx * 3.0 + p->semen * 0.01, ny * 3.0, 5, 0.5) - 0.5) * 0.6;

            /* terra an aqua? */
            int terra = 1;
            if (p->aqua > 0.01)
                terra = (int)continent_mask(lon, lat, p);

            color_t col;

            if (terra) {
                col = color_superficiei(p, var);

                /* biomes: variatio coloris intra continentes ex latitudine.
                 * Si malachita praesens (vegetatio), modulatur per lat:
                 *   tropicae (|lat|<25°): viridis saturatus
                 *   temperatae (25-55°): viridis moderatus
                 *   boreales (55-70°): viridis obscurior
                 *   aridae (deserta): flavum-brunneum per strepitum
                 * Hoc producit variegatam superficiem non uniformem. */
                if (p->malachita > 0.1) {
                    double abs_lat = fabs(lat) / (PI * 0.5);

                    /* ariditas: deserta per strepitum (non omnia virides) */
                    double arid = fbm(nx * 1.5 + 33.0, ny * 1.5 + 17.0, 4, 0.5);
                    arid = fmax(0.0, arid - 0.35) * 2.5; /* 0..1 */
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
                if (p->maria > 0.01) {
                    double mare = fbm_warp(nx * 0.6 + 3.7, ny * 0.6 + 1.2, 6, 0.5, 1.5);
                    if (mare < p->maria) {
                        double mt = fmin(1.0, (p->maria - mare) / fmax(0.01, p->maria) * 3.0);
                        color_t dark = col;
                        dark.r *= 0.35; dark.g *= 0.35; dark.b *= 0.38;
                        col = miscere(col, dark, mt);
                    }
                }

                /* craterae */
                if (p->craterae > 0.01) {
                    double bright;
                    double depth = crater_field(lon, lat, p->craterae,
                                               p->semen + 777, &bright);
                    double lmod = 1.0 + depth * 1.0;
                    lmod = fmax(0.4, fmin(1.5, lmod));
                    col.r *= lmod; col.g *= lmod; col.b *= lmod;
                    col.r = fmin(1.0, col.r + bright * 0.8);
                    col.g = fmin(1.0, col.g + bright * 0.8);
                    col.b = fmin(1.0, col.b + bright * 0.75);
                }

                /* vulcanismus */
                if (p->vulcanismus > 0.01) {
                    double v = fbm_warp(nx * 2.0 + 11.0, ny * 2.0 + 7.0, 5, 0.6, 2.5);
                    double thr = 1.0 - p->vulcanismus * 0.3;
                    if (v > thr) {
                        double t = (v - thr) / (1.0 - thr);
                        t = t * t;
                        /* lava blackbody: ~1000K=red, ~1500K=orange, ~2000K=yellow */
                        color_t lava;
                        if (t < 0.4) {
                            double t2 = t / 0.4;
                            lava = (color_t){0.8 * t2, 0.15 * t2, 0.02, 1.0};
                        } else {
                            double t2 = (t - 0.4) / 0.6;
                            lava = (color_t){0.8 + 0.2 * t2, 0.15 + 0.55 * t2, 0.02 + 0.2 * t2, 1.0};
                        }
                        col = miscere(col, lava, t);
                        illum = fmax(illum, t * 0.8);
                    }
                }
            } else {
                /* oceanus: color ex profunditate.
                 * Litora: aqua vadosa caeruleum clarum (scattering per arenam).
                 * Profunditas per distantiam a litore. */
                double depth_var = fbm(nx * 1.2 + 20.0, ny * 1.2 + 20.0, 5, 0.5);

                /* distantia a litore: approximatur per terra_elev valorem.
                 * terrain prope limen = litus, longe infra = profundum */
                double sc_l = fmax(0.3, p->scala);
                double ox_l = (p->semen & 0xFF) * 0.13;
                double oy_l = ((p->semen >> 8) & 0xFF) * 0.13;
                double nx_l = lon / DPI * 5.0 * sc_l;
                double ny_l = (lat / PI + 0.5) * 2.5 * sc_l;
                double terra_val = fbm_warp(nx_l + ox_l, ny_l + oy_l, 5, 0.55,
                                             1.0 + p->tectonica * 3.0) * 0.30;
                /* blob contributo */
                unsigned s_l = p->semen * 7919 + 77777;
                int nc_l = fmax(1, p->continentes);
                double blob_max = 0.0;
                for (int c = 0; c < nc_l; c++) {
                    s_l = s_l * 1103515245 + 12345;
                    double blon = (s_l >> 8 & 0xFFFF) / 65536.0 * DPI;
                    s_l = s_l * 1103515245 + 12345;
                    double blat = ((s_l >> 8 & 0xFFFF) / 65536.0 - 0.5) * PI * 0.85;
                    s_l = s_l * 1103515245 + 12345;
                    double brad = (1.0 + (s_l >> 8 & 0xFF) / 255.0 * 1.2) * sc_l / pow(nc_l, 0.35);
                    double dl = lat - blat, dn = lon - blon;
                    if (dn > PI) dn -= DPI; if (dn < -PI) dn += DPI;
                    double dd = sqrt(dl * dl + dn * dn * cos(lat) * cos(lat));
                    double blob = fmax(0.0, 1.0 - dd / brad);
                    blob_max = fmax(blob_max, blob * blob);
                }
                double terra_totum = terra_val + blob_max * 0.70;
                double limen_l = p->aqua * 0.30;
                double dist_litoris = limen_l - terra_totum; /* >0 = sub aqua */

                double prof;
                if (dist_litoris < 0.05) {
                    /* vadosa: prope litus */
                    prof = dist_litoris / 0.05 * 0.3;
                } else {
                    prof = 0.3 + (p->aqua_profunditas * 0.5 + depth_var * 0.2);
                }
                prof = fmax(0.0, fmin(1.0, prof));
                col = color_aquae(prof, var);

                /* specularis: reflexio solaris — fortior, punctum clarum */
                double dx = (px - SEMI) / (rad * SEMI);
                double dy = (py - SEMI) / (rad * SEMI);
                double sx = dx + cos(p->angulus_phase) * 0.20;
                double sy = dy + sin(p->angulus_phase) * 0.20;
                double spec = exp(-(sx * sx + sy * sy) * 6.0);
                spec *= (1.0 - p->phase) * 0.35;
                col.r = fmin(1.0, col.r + spec);
                col.g = fmin(1.0, col.g + spec * 0.95);
                col.b = fmin(1.0, col.b + spec * 0.85);

                /* micro undae oceani */
                double wave = strepitus2(nx * 30.0 + 91.0, ny * 30.0 + 97.0);
                col.r *= 0.95 + 0.05 * wave;
                col.g *= 0.95 + 0.05 * wave;
                col.b *= 0.96 + 0.04 * wave;
            }

            /* calottae polares */
            if (p->polaris > 0.01) {
                double abs_lat = fabs(lat) / (PI * 0.5);
                double lim = 1.0 - p->polaris;
                double gn = fbm(nx * 2.5, ny * 2.5, 4, 0.5) * 0.1;
                if (abs_lat > lim - gn) {
                    double t = fmin(1.0, (abs_lat - lim + gn) * 8.0);
                    color_t ice = {0.90, 0.92, 0.96, 1.0};
                    if (p->glacies_co2 > p->glacies)
                        ice = (color_t){0.94, 0.93, 0.91, 1.0};
                    col = miscere(col, ice, t);
                }
            }

            /* nubes */
            if (p->nubes > 0.01 && p->pressio_kPa > 0.3) {
                double cn = fbm_warp(nx * 1.0 + 50.0, ny * 1.0 + 50.0, 7, 0.55, 2.0);
                /* stratum secundum nubium minorum */
                double cn2 = fbm(nx * 2.5 + 80.0, ny * 2.5 + 80.0, 5, 0.5);
                cn = cn * 0.65 + cn2 * 0.35;

                double thr = 1.0 - p->nubes * 0.6;
                if (cn > thr) {
                    double t = (cn - thr) / fmax(0.01, 1.0 - thr);
                    t = t * t; /* margines molles */
                    /* color nubium ex compositione atmosphaerae */
                    color_t nub = {0.95, 0.95, 0.97, 1.0};  /* H₂O default */
                    if (p->co2 > 0.5)
                        nub = (color_t){0.94, 0.88, 0.68, 1.0}; /* H₂SO₄ Venus */
                    if (p->ch4 > 0.01)
                        nub = (color_t){0.93, 0.95, 0.98, 1.0}; /* CH₄ cirrus */
                    col = miscere(col, nub, t * 0.70);
                }
            }

            /* bump mapping: terrain elevatio modulat normalem superficiei
             * per differentias finitas → montanae proiciunt umbras.
             * Solum pro terra (non aqua). */
            if (terra) {
                double h = 0.002; /* passum differentialem in radianes */
                double e_here = fbm_warp(nx + (p->semen & 0xFF) * 0.13,
                                        ny + ((p->semen >> 8) & 0xFF) * 0.13,
                                        5, 0.5, 1.0 + p->tectonica * 3.0);
                /* gradiens in lon et lat */
                double nx_e = (lon + h) / DPI * 10.0;
                double ny_n = ((lat + h) / PI + 0.5) * 5.0;
                double e_east = fbm_warp(nx_e + (p->semen & 0xFF) * 0.13,
                                        ny + ((p->semen >> 8) & 0xFF) * 0.13,
                                        5, 0.5, 1.0 + p->tectonica * 3.0);
                double e_north = fbm_warp(nx + (p->semen & 0xFF) * 0.13,
                                         ny_n + ((p->semen >> 8) & 0xFF) * 0.13,
                                         5, 0.5, 1.0 + p->tectonica * 3.0);
                double dlon = (e_east - e_here) * 8.0;
                double dlat = (e_north - e_here) * 8.0;
                /* perturbatio illuminationis per normalem */
                double bump = -dlon * cos(p->angulus_phase) - dlat * sin(p->angulus_phase);
                illum *= fmax(0.3, 1.0 + bump * 0.6);
            }

            /* umbrae nubium: ubi nubes sunt, superficies sub eis obscurior.
             * Offset in directione lucis simulat umbram proiectam. */
            if (p->nubes > 0.05 && p->pressio_kPa > 0.3 && illum > 0.1) {
                /* positionem nubis offset per aliquot pixels in directione lucis */
                double shadow_dx = cos(p->angulus_phase) * 0.03;
                double shadow_dy = sin(p->angulus_phase) * 0.03;
                double shadow_lon = lon + shadow_dx;
                double shadow_lat = lat + shadow_dy;
                double snx = shadow_lon / DPI * 10.0;
                double sny = (shadow_lat / PI + 0.5) * 5.0;
                double scn = fbm_warp(snx * 0.1 + 50.0, sny * 0.1 + 50.0, 7, 0.55, 2.0);
                double scn2 = fbm(snx * 0.25 + 80.0, sny * 0.25 + 80.0, 5, 0.5);
                scn = scn * 0.65 + scn2 * 0.35;
                double sthr = 1.0 - p->nubes * 0.6;
                if (scn > sthr) {
                    double shadow_t = (scn - sthr) / fmax(0.01, 1.0 - sthr);
                    shadow_t = shadow_t * shadow_t;
                    illum *= 1.0 - shadow_t * 0.35;
                }
            }

            /* applicare illuminationem */
            col.r *= illum; col.g *= illum; col.b *= illum;

            /* atmosphaera limbus: crescit cum pressione */
            if (pnorm > 0.003) {
                double dx = (px - SEMI) / (rad * SEMI);
                double dy = (py - SEMI) / (rad * SEMI);
                double edge = sqrt(dx * dx + dy * dy);
                double atm = pow(edge, 4.0) * pnorm * 0.6;
                col = miscere(col, atm_col, fmin(0.7, atm));
            }

            fen_pixel(fen, px, py, col.r, col.g, col.b, 1.0);
        }
    }

    /* atmospheric halo outside disk */
    if (pnorm > 0.02) {
        double halo_ext = rad * (1.0 + pnorm * 0.07);
        for (int py = 0; py < FEN; py++) {
            for (int px = 0; px < FEN; px++) {
                double dx = (px - SEMI) / (halo_ext * SEMI);
                double dy = (py - SEMI) / (halo_ext * SEMI);
                double r2 = dx * dx + dy * dy;
                double inner = rad / halo_ext;
                if (r2 < inner * inner || r2 >= 1.0) continue;
                double d = sqrt(r2);
                double a = (1.0 - (d - inner) / (1.0 - inner));
                a = a * a * a * pnorm * 0.4;
                double il = illuminatio(px, py, halo_ext, p->phase, p->angulus_phase);
                a *= fmax(0.1, il);
                fen_pixel(fen, px, py, atm_col.r, atm_col.g, atm_col.b, a);
            }
        }
    }
}

static void reddere_gaseosum(unsigned char *fen, const planeta_t *p)
{
    double rad = p->radius;
    color_t atm_col = color_atmosphaerae(p);

    /* colores zonarum/cingulorum ex atmosphaera derivati */
    color_t zona = atm_col;
    zona.r = fmin(1.0, zona.r * 1.4 + 0.12);
    zona.g = fmin(1.0, zona.g * 1.4 + 0.10);
    zona.b = fmin(1.0, zona.b * 1.3 + 0.06);
    color_t belt = atm_col;
    belt.r *= 0.60; belt.g *= 0.48; belt.b *= 0.40;

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double lon, lat;
            if (!pixel_ad_sphaeram(px, py, rad, p->inclinatio, p->rotatio,
                                   &lon, &lat))
                continue;

            double illum = illuminatio(px, py, rad, p->phase, p->angulus_phase);
            if (illum < 0.003) continue;

            double band_y = lat / (PI * 0.5);
            double nx = lon / DPI * 8.0;
            double ny = (band_y + 1.0) * 2.0;

            /* banded structure */
            double band = sin(band_y * PI * p->fasciae * 0.5);

            /* turbulence within bands — multi-scale */
            double turb1 = fbm_warp(nx + p->semen * 0.003, ny, 6, 0.5,
                                   1.5 * p->fasciae_contrast);
            double turb2 = fbm(nx * 2.0 + band_y * 5.0, ny * 0.3, 4, 0.45);
            band += (turb1 - 0.5) * 0.8 * p->fasciae_contrast;
            band += (turb2 - 0.5) * 0.3;

            /* vortices fluxus zonalis ad limites fasciarum */
            double flow = ridged(nx * 1.5 + band_y * 2.0, band_y * 3.0, 3, 0.5);
            double at_boundary = 1.0 - fabs(band) * 2.0;
            at_boundary = fmax(0.0, at_boundary);
            band += (flow - 0.5) * 0.4 * at_boundary;

            double t = fmax(0.0, fmin(1.0, band * 0.5 + 0.5));
            color_t col = miscere(belt, zona, t);

            /* textura nubium multi-scala */
            double tex1 = fbm(nx * 4.0, ny * 4.0, 4, 0.45);
            double tex2 = fbm(nx * 10.0 + 31.0, ny * 10.0 + 37.0, 3, 0.4);
            double tex3 = strepitus2(nx * 25.0 + 71.0, ny * 25.0 + 73.0);
            double tex = tex1 * 0.5 + tex2 * 0.3 + tex3 * 0.2;
            /* variatio chromatica inter fascias */
            double chr = fbm(nx * 3.0 + 41.0, ny * 6.0 + 43.0, 3, 0.4);
            col.r *= 0.80 + 0.20 * tex;
            col.g *= 0.80 + 0.20 * tex * (0.9 + 0.1 * chr);
            col.b *= 0.82 + 0.18 * tex * (0.85 + 0.15 * chr);

            /* maculae — storm vortices */
            for (int mi = 0; mi < p->maculae; mi++) {
                double mlat = p->macula_lat * PI * 0.5;
                double mlon = p->macula_lon + mi * 1.8;
                double mrad = p->macula_radius * (1.0 - mi * 0.25);
                if (mrad < 0.02) continue;

                double dlat = lat - mlat;
                double dlon = lon - mlon;
                if (dlon > PI) dlon -= DPI;
                if (dlon < -PI) dlon += DPI;
                double md = sqrt(dlat * dlat * 2.5 + dlon * dlon) / (mrad * PI);
                if (md >= 1.0) continue;

                double mt = 1.0 - md * md;
                /* spiral structure inside vortex */
                double ang = atan2(dlat, dlon);
                double spiral = sin(ang * 2.0 - md * 8.0 + p->semen * 0.1);
                spiral = spiral * 0.5 + 0.5;
                double vort = fbm(md * 5.0 + ang * 0.4, ang * 0.5 + md * 3.0, 4, 0.5);
                mt *= (0.5 + 0.3 * spiral + 0.2 * vort);

                /* color tempestatis: chromophorae organicae (phosphorus,
                 * sulphur composita) dant colorem rubrum/brunneum.
                 * Macula Rubra Magna: vortex anticyclonicus persistens,
                 * coloratus per upwelling composita ex profunditate.
                 * Ref: Simon-Miller et al. (2001) Icarus 154:459-474 */
                double dark = p->macula_obscuritas;
                color_t mc;
                mc.r = fmin(1.0, 0.65 + dark * 0.15);
                mc.g = fmax(0.0, 0.30 - dark * 0.10);
                mc.b = fmax(0.0, 0.18 - dark * 0.10);
                mc.a = 1.0;
                col = miscere(col, mc, mt * 0.90);
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
}

static void reddere_glaciale(unsigned char *fen, const planeta_t *p)
{
    double rad = p->radius;
    color_t base = color_atmosphaerae(p);
    /* ice giants are brighter than raw atmosphere color */
    base.r = fmin(1.0, base.r * 1.2 + 0.08);
    base.g = fmin(1.0, base.g * 1.2 + 0.08);
    base.b = fmin(1.0, base.b * 1.15 + 0.10);

    for (int py = 0; py < FEN; py++) {
        for (int px = 0; px < FEN; px++) {
            double lon, lat;
            if (!pixel_ad_sphaeram(px, py, rad, p->inclinatio, p->rotatio,
                                   &lon, &lat))
                continue;

            double illum = illuminatio(px, py, rad, p->phase, p->angulus_phase);
            if (illum < 0.003) continue;

            double nx = lon / DPI * 4.0;
            double ny = (lat / PI + 0.5) * 2.0;
            color_t col = base;

            /* subtle bands */
            if (p->fasciae > 0) {
                double band_y = lat / (PI * 0.5);
                double band = sin(band_y * PI * p->fasciae * 0.5);
                double turb = fbm(nx + p->semen * 0.01, ny, 4, 0.5);
                band += (turb - 0.5) * 0.3;
                band = band * 0.5 + 0.5;
                band = fmax(0.0, fmin(1.0, band));
                /* subtle brightening/darkening */
                double mod = 0.92 + 0.08 * band * p->fasciae_contrast;
                col.r *= mod; col.g *= mod; col.b *= mod + 0.02;
            }

            /* spots */
            for (int mi = 0; mi < p->maculae; mi++) {
                double mlat = p->macula_lat * PI * 0.5;
                double mlon = p->macula_lon;
                double dlat = lat - mlat;
                double dlon = lon - mlon;
                if (dlon > PI) dlon -= DPI;
                if (dlon < -PI) dlon += DPI;
                double md = sqrt(dlat * dlat * 1.5 + dlon * dlon) / (p->macula_radius * PI);
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
            col.r *= tex; col.g *= tex; col.b *= tex;

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

static void reddere_parvum(unsigned char *fen, const planeta_t *p)
{
    reddere_saxosum(fen, p);
}

/* ================================================================
 * dispatcher
 * ================================================================ */

void planeta_reddere(unsigned char *fenestra, const planeta_t *planeta)
{
    memset(fenestra, 0, FEN * FEN * 4);
    switch (planeta->genus) {
    case PLANETA_SAXOSUM:  reddere_saxosum(fenestra, planeta);  break;
    case PLANETA_GASEOSUM: reddere_gaseosum(fenestra, planeta); break;
    case PLANETA_GLACIALE: reddere_glaciale(fenestra, planeta); break;
    case PLANETA_PARVUM:   reddere_parvum(fenestra, planeta);   break;
    }
}

/* ================================================================
 * ISON parser
 * ================================================================ */

static planeta_genus_t genus_ex_nomine(const char *s)
{
    if (!s) return PLANETA_SAXOSUM;
    if (strcmp(s, "saxosum") == 0)  return PLANETA_SAXOSUM;
    if (strcmp(s, "gaseosum") == 0) return PLANETA_GASEOSUM;
    if (strcmp(s, "glaciale") == 0) return PLANETA_GLACIALE;
    if (strcmp(s, "parvum") == 0)   return PLANETA_PARVUM;
    return PLANETA_SAXOSUM;
}

static double ison_f(const char *ison, const char *via, double praef)
{
    /* try string first, then integer */
    char *s = ison_da_chordam(ison, via);
    if (s) {
        double v = atof(s);
        free(s);
        return v;
    }
    long n = ison_da_numerum(ison, via);
    if (n != 0) return (double)n;
    return praef;
}

planeta_t planeta_ex_ison(const char *ison)
{
    char *genus_s = ison_da_chordam(ison, "genus");
    planeta_t p;
    memset(&p, 0, sizeof(p));

    p.genus           = genus_ex_nomine(genus_s);
    p.radius          = ison_f(ison, "radius", 0.9);       /* fractio fenestrae [0,1] */
    p.phase           = ison_f(ison, "phase", 0.0);        /* [0=plenus, 1=novus] */
    p.angulus_phase   = ison_f(ison, "angulus_phase", 0.0); /* radiani */
    p.inclinatio      = ison_f(ison, "inclinatio", 0.0);    /* radiani */
    p.rotatio         = ison_f(ison, "rotatio", 0.0);       /* radiani */
    p.semen           = (unsigned)ison_f(ison, "semen", 42);

    /* compositio superficiei [fractiones, summa ≤ 1] */
    p.silicata     = ison_f(ison, "silicata", 0.0);
    p.ferrum       = ison_f(ison, "ferrum", 0.0);
    p.sulphur      = ison_f(ison, "sulphur", 0.0);
    p.carbo        = ison_f(ison, "carbo", 0.0);
    p.glacies      = ison_f(ison, "glacies", 0.0);
    p.glacies_co2  = ison_f(ison, "glacies_co2", 0.0);
    p.malachita    = ison_f(ison, "malachita", 0.0);

    /* aqua liquida */
    p.aqua             = ison_f(ison, "aqua", 0.0);      /* fractio [0,1] */
    p.aqua_profunditas = ison_f(ison, "aqua_profunditas", 0.5); /* [0,1] */

    /* terrain */
    p.continentes  = (int)ison_f(ison, "continentes", 0);
    p.scala        = ison_f(ison, "scala", 1.0);          /* multiplicator */
    p.tectonica    = ison_f(ison, "tectonica", 0.3);       /* [0,1] */
    p.craterae     = ison_f(ison, "craterae", 0.0);        /* [0,1] */
    p.maria        = ison_f(ison, "maria", 0.0);            /* [0,1] */
    p.vulcanismus  = ison_f(ison, "vulcanismus", 0.0);      /* [0,1] */

    /* atmosphaera */
    p.pressio_kPa  = ison_f(ison, "pressio_kPa", 0.0);   /* kPa */
    p.n2           = ison_f(ison, "n2", 0.0);              /* fractio */
    p.o2           = ison_f(ison, "o2", 0.0);
    p.co2          = ison_f(ison, "co2", 0.0);
    p.ch4          = ison_f(ison, "ch4", 0.0);
    p.h2           = ison_f(ison, "h2", 0.0);
    p.he           = ison_f(ison, "he", 0.0);
    p.nh3          = ison_f(ison, "nh3", 0.0);
    p.pulvis       = ison_f(ison, "pulvis", 0.0);          /* [0,1] */
    p.nubes        = ison_f(ison, "nubes", 0.0);            /* [0,1] */

    p.polaris      = ison_f(ison, "polaris", 0.0);          /* [0,1] */

    p.fasciae          = (int)ison_f(ison, "fasciae", 0);
    p.fasciae_contrast = ison_f(ison, "fasciae_contrast", 0.5); /* [0,1] */

    p.maculae           = (int)ison_f(ison, "maculae", 0);
    p.macula_lat        = ison_f(ison, "macula_lat", 0.0);      /* [-1,1] */
    p.macula_lon        = ison_f(ison, "macula_lon", 0.0);      /* radiani */
    p.macula_radius     = ison_f(ison, "macula_radius", 0.1);   /* [0,1] */
    p.macula_obscuritas = ison_f(ison, "macula_obscuritas", 0.5);/* [-1,1] */

    free(genus_s);
    return p;
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

void planeta_instrumentum_applicare(const planeta_t *planeta,
                                    astra_instrumentum_t *instr)
{
    double a = fmin(1.0, planeta->pressio_kPa / 101.0);
    if (a < 0.005) return;

    double seeing = a * a * 3.0;
    double v2 = instr->visio * instr->visio + seeing * seeing;
    instr->visio = sqrt(v2);

    instr->scintillatio += a * 0.4;
    if (instr->scintillatio > 1.0) instr->scintillatio = 1.0;

    instr->caeli_lumen += a * a * 0.15;
    instr->refractio += a * 1.5;

    double desat = 1.0 - a * 0.15;
    if (desat < 0.3) desat = 0.3;
    instr->saturatio *= desat;
}
