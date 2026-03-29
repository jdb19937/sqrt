/*
 * TORUS PLANUS CORRUGATUS
 * Immersio isometrica C1 tori plani quadrati in spatium Euclideum R3
 * secundum methodum Nash-Kuiper corrugationis
 *
 * Auctore: machina Bernoulli
 * C99, sine ullis dependentiis externis
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* ================================================================ */
/*                    CONSTANTES UNIVERSALES                        */
/* ================================================================ */

#define PI_GRAECUM       3.14159265358979323846
#define DUO_PI           (2.0 * PI_GRAECUM)

/* Dimensiones imaginis */
#define LATITUDO_IMG     1920
#define ALTITUDO_IMG     1080

/* Discretio superficiei parametricae */
#define GRADUS_U         800
#define GRADUS_V         400

/* Strata corrugationis Nash-Kuiper */
#define STRATA_CORRUG    5

/* Parametri tori */
#define RADIUS_MAIOR     1.0
#define RADIUS_MINOR     0.42

/* ================================================================ */
/*                     TYPI MATHEMATICI                             */
/* ================================================================ */

typedef struct { double x, y, z; } Vec3;
typedef struct { double r, g, b; } Color;

/* ================================================================ */
/*               OPERATIONES VECTORIALES                            */
/* ================================================================ */

static inline Vec3 vec3(double x, double y, double z)
{
    return (Vec3){x, y, z};
}

static inline Vec3 summa(Vec3 a, Vec3 b)
{
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 differentia(Vec3 a, Vec3 b)
{
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline double productum_scalare(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 productum_vectoriale(Vec3 a, Vec3 b)
{
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline Vec3 multiplicare(Vec3 v, double s)
{
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

static inline double magnitudo(Vec3 v)
{
    return sqrt(productum_scalare(v, v));
}

static inline Vec3 normalizare(Vec3 v)
{
    double m = magnitudo(v);
    if (m < 1e-15) return (Vec3){0.0, 0.0, 1.0};
    return multiplicare(v, 1.0 / m);
}

/* ================================================================ */
/*            SUPERFICIES TORI PLANI CORRUGATI                      */
/*                                                                  */
/*  Corrugatio multi-scalaris ad metricam planam approximandam.     */
/*  Frequentiae geometrice crescunt; amplitudines decrescunt.       */
/*  Haec est approximatio finita schematis convexae integrationis   */
/*  Nash-Kuiper, quae metricum Euclideanum in toro inducit.        */
/* ================================================================ */

static Vec3 superficies_tori(double u, double v)
{
    /*
     * Schema convexae integrationis Nash-Kuiper:
     * Corrugationes alternantes in directionibus u et v,
     * frequentiis geometrice crescentibus (ratio ~3),
     * amplitudinibus decrescentibus.
     * Hoc est characteristicum tori Hévéa.
     */

    double corrugatio = 0.0;
    double perturb_u = 0.0, perturb_v = 0.0;

    for (int k = 0; k < STRATA_CORRUG; k++) {
        double freq = 7.0 * pow(2.8, (double)k);
        double amp  = 0.035 / pow(1.8, (double)k);

        if (k % 2 == 0) {
            /* Corrugatio in directione u (toroidali) */
            corrugatio += amp * sin(freq * u + 0.5 * k);
            perturb_v  += amp * 0.15 * cos(freq * u + 0.5 * k);
        } else {
            /* Corrugatio in directione v (poloidali) */
            corrugatio += amp * sin(freq * v + 0.8 * k);
            perturb_u  += amp * 0.15 * cos(freq * v + 0.8 * k);
        }

        /* Corrugatio transversa minor pro textura */
        double amp2 = amp * 0.3;
        corrugatio += amp2 * sin(freq * u + 1.3 * k)
                           * sin(freq * v + 0.9 * k);
    }

    double uu = u + perturb_u;
    double vv = v + perturb_v;

    double r_eff = RADIUS_MINOR + corrugatio;
    double cu = cos(uu), su = sin(uu);
    double cv = cos(vv), sv = sin(vv);

    return (Vec3){
        (RADIUS_MAIOR + r_eff * cv) * cu,
        (RADIUS_MAIOR + r_eff * cv) * su,
        r_eff * sv
    };
}

/* Norma per differentias finitas centrales */
static Vec3 norma_superficiei(double u, double v)
{
    double h = 5e-5;
    Vec3 du = differentia(superficies_tori(u + h, v),
                          superficies_tori(u - h, v));
    Vec3 dv = differentia(superficies_tori(u, v + h),
                          superficies_tori(u, v - h));
    return normalizare(productum_vectoriale(du, dv));
}

/* ================================================================ */
/*                   CAMERA PERSPECTIVA                             */
/* ================================================================ */

typedef struct {
    Vec3   positio;
    Vec3   ante;       /* directio "forward" */
    Vec3   dextrum;    /* directio "right"   */
    Vec3   sursum;     /* directio "up"      */
    double focalis;    /* distantia focalis  */
} Camera;

static Camera cameram_constituere(Vec3 positio, Vec3 scopus)
{
    Camera cam;
    cam.positio = positio;
    cam.ante    = normalizare(differentia(scopus, positio));

    Vec3 mundi_sursum = vec3(0.0, 0.0, 1.0);
    cam.dextrum = normalizare(productum_vectoriale(cam.ante, mundi_sursum));
    cam.sursum  = productum_vectoriale(cam.dextrum, cam.ante);
    cam.focalis = 1.6;
    return cam;
}

static int proicere(const Camera *cam, Vec3 p,
                    double *scr_x, double *scr_y, double *prof)
{
    Vec3 d  = differentia(p, cam->positio);
    double z = productum_scalare(d, cam->ante);
    if (z < 0.05) return 0;

    double x = productum_scalare(d, cam->dextrum);
    double y = productum_scalare(d, cam->sursum);
    double f = cam->focalis / z;

    double scala = (LATITUDO_IMG < ALTITUDO_IMG)
                 ? LATITUDO_IMG * 0.5
                 : ALTITUDO_IMG * 0.5;

    *scr_x = LATITUDO_IMG * 0.5 + x * f * scala;
    *scr_y = ALTITUDO_IMG * 0.5 - y * f * scala;
    *prof  = z;
    return 1;
}

/* ================================================================ */
/*                  ILLUMINATIO (PHONG)                             */
/* ================================================================ */

static Color illuminare(Vec3 punct, Vec3 norm, Vec3 oculus)
{
    /* Tres luces directae */
    Vec3 lux_dir[3] = {
        normalizare(vec3( 1.0, -0.6,  1.8)),
        normalizare(vec3(-0.7,  0.8,  0.4)),
        normalizare(vec3( 0.2, -1.0, -0.2))
    };
    Color lux_intens[3] = {
        {0.90, 0.80, 0.65},
        {0.20, 0.30, 0.55},
        {0.12, 0.10, 0.08}
    };

    /* Color materiei: aurum antiquum */
    double mat_r = 0.78, mat_g = 0.55, mat_b = 0.28;

    /* Lux ambiens */
    Color res = {mat_r * 0.06, mat_g * 0.06, mat_b * 0.06};

    Vec3 ad_oculum = normalizare(differentia(oculus, punct));

    /* Normam orientare versus oculum */
    if (productum_scalare(norm, ad_oculum) < 0.0)
        norm = multiplicare(norm, -1.0);

    for (int i = 0; i < 3; i++) {
        /* Diffusum (Lambert) */
        double NdotL = productum_scalare(norm, lux_dir[i]);
        if (NdotL < 0.0) NdotL = 0.0;

        /* Specularis (Blinn-Phong) */
        Vec3 semi = normalizare(summa(lux_dir[i], ad_oculum));
        double NdotH = productum_scalare(norm, semi);
        if (NdotH < 0.0) NdotH = 0.0;
        double spec = pow(NdotH, 60.0);

        res.r += mat_r * NdotL * lux_intens[i].r + spec * lux_intens[i].r * 0.45;
        res.g += mat_g * NdotL * lux_intens[i].g + spec * lux_intens[i].g * 0.45;
        res.b += mat_b * NdotL * lux_intens[i].b + spec * lux_intens[i].b * 0.45;
    }

    /* Effectus Fresnel (reflexio ad margines) */
    double fresnel = 1.0 - fabs(productum_scalare(norm, ad_oculum));
    fresnel = fresnel * fresnel * fresnel * 0.35;
    res.r += fresnel * 0.4;
    res.g += fresnel * 0.5;
    res.b += fresnel * 0.65;

    return res;
}

/* ================================================================ */
/*               TABULAE IMAGINIS ET PROFUNDITATIS                  */
/* ================================================================ */

static unsigned char *tabula_imaginis;
static double        *tabula_profunditatis;

static inline unsigned char gamma_corrigere(double valor)
{
    if (valor < 0.0) valor = 0.0;
    if (valor > 1.0) valor = 1.0;
    return (unsigned char)(pow(valor, 1.0 / 2.2) * 255.0 + 0.5);
}

static inline void pixel_scribere(int x, int y, double prof, Color c)
{
    if (x < 0 || x >= LATITUDO_IMG || y < 0 || y >= ALTITUDO_IMG) return;
    int idx = y * LATITUDO_IMG + x;
    if (prof >= tabula_profunditatis[idx]) return;
    tabula_profunditatis[idx] = prof;

    int base = idx * 3;
    tabula_imaginis[base + 0] = gamma_corrigere(c.r);
    tabula_imaginis[base + 1] = gamma_corrigere(c.g);
    tabula_imaginis[base + 2] = gamma_corrigere(c.b);
}

/* ================================================================ */
/*            RASTERIZATIO TRIANGULORUM                             */
/*                                                                  */
/*  Coordinatae barycentricae cum interpolatione normalium          */
/*  pro illuminatione Phong laevi.                                  */
/* ================================================================ */

static void triangulum_reddere(
    double sx0, double sy0, double sz0, Vec3 p0, Vec3 n0,
    double sx1, double sy1, double sz1, Vec3 p1, Vec3 n1,
    double sx2, double sy2, double sz2, Vec3 p2, Vec3 n2,
    Vec3 oculus)
{
    /* Area dupla in spatio scrinii */
    double area = (sx1 - sx0) * (sy2 - sy0) - (sx2 - sx0) * (sy1 - sy0);
    if (fabs(area) < 0.5) return;
    double inv_area = 1.0 / area;

    /* Involucrum (bounding box) */
    int min_x = (int)floor(fmin(fmin(sx0, sx1), sx2));
    int max_x = (int)ceil (fmax(fmax(sx0, sx1), sx2));
    int min_y = (int)floor(fmin(fmin(sy0, sy1), sy2));
    int max_y = (int)ceil (fmax(fmax(sy0, sy1), sy2));

    if (min_x < 0) min_x = 0;
    if (max_x >= LATITUDO_IMG) max_x = LATITUDO_IMG - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= ALTITUDO_IMG) max_y = ALTITUDO_IMG - 1;

    for (int y = min_y; y <= max_y; y++) {
        double py = y + 0.5;
        for (int x = min_x; x <= max_x; x++) {
            double px = x + 0.5;

            /* Coordinatae barycentricae */
            double w0 = ((sx1 - px) * (sy2 - py) - (sx2 - px) * (sy1 - py))
                        * inv_area;
            double w1 = ((sx2 - px) * (sy0 - py) - (sx0 - px) * (sy2 - py))
                        * inv_area;
            double w2 = 1.0 - w0 - w1;

            if (w0 < -0.001 || w1 < -0.001 || w2 < -0.001) continue;

            /* Profunditas interpolata */
            double z = w0 * sz0 + w1 * sz1 + w2 * sz2;

            /* Punctum et norma interpolata */
            Vec3 punct = summa(summa(multiplicare(p0, w0),
                                     multiplicare(p1, w1)),
                               multiplicare(p2, w2));
            Vec3 norm  = normalizare(
                         summa(summa(multiplicare(n0, w0),
                                     multiplicare(n1, w1)),
                               multiplicare(n2, w2)));

            Color c = illuminare(punct, norm, oculus);
            pixel_scribere(x, y, z, c);
        }
    }
}

/* ================================================================ */
/*                      PRINCIPIUM                                  */
/* ================================================================ */

int main(void)
{
    fprintf(stderr, "=== TORUS PLANUS CORRUGATUS ===\n");
    fprintf(stderr, "Immersio isometrica C1 in R3\n\n");

    /* Tabulam imaginis et profunditatis assignare */
    size_t n_pixels = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    tabula_imaginis       = (unsigned char *)calloc(n_pixels * 3, 1);
    tabula_profunditatis  = (double *)malloc(n_pixels * sizeof(double));

    if (!tabula_imaginis || !tabula_profunditatis) {
        fprintf(stderr, "ERROR: memoria insufficiens!\n");
        return 1;
    }

    /* Fundum obscurum initiare */
    for (size_t i = 0; i < n_pixels; i++) {
        tabula_profunditatis[i] = 1e30;
        tabula_imaginis[i * 3 + 0] = 8;
        tabula_imaginis[i * 3 + 1] = 8;
        tabula_imaginis[i * 3 + 2] = 14;
    }

    /* Cameram constituere */
    Vec3 positio_camerae = vec3(2.6, -2.0, 1.5);
    Vec3 scopus          = vec3(0.0, 0.0, -0.05);
    Camera cam = cameram_constituere(positio_camerae, scopus);

    /* ------------------------------------------------------------ */
    /*  Puncta et normae superficiei praecomputare                  */
    /* ------------------------------------------------------------ */
    fprintf(stderr, "Superficiem computans (%d x %d)...\n",
            GRADUS_U, GRADUS_V);

    size_t n_vertices = (size_t)(GRADUS_U + 1) * (GRADUS_V + 1);
    Vec3 *puncta = (Vec3 *)malloc(n_vertices * sizeof(Vec3));
    Vec3 *normae = (Vec3 *)malloc(n_vertices * sizeof(Vec3));

    if (!puncta || !normae) {
        fprintf(stderr, "ERROR: memoria insufficiens pro superficie!\n");
        return 1;
    }

    for (int i = 0; i <= GRADUS_U; i++) {
        double u = DUO_PI * (double)i / (double)GRADUS_U;
        for (int j = 0; j <= GRADUS_V; j++) {
            double v = DUO_PI * (double)j / (double)GRADUS_V;
            size_t idx = (size_t)i * (GRADUS_V + 1) + j;
            puncta[idx] = superficies_tori(u, v);
            normae[idx] = norma_superficiei(u, v);
        }
    }

    /* ------------------------------------------------------------ */
    /*  Triangula proicere et reddere                               */
    /* ------------------------------------------------------------ */
    fprintf(stderr, "Triangula reddens...\n");

    long triangula_reddita = 0;

    for (int i = 0; i < GRADUS_U; i++) {
        if (i % 100 == 0)
            fprintf(stderr, "  %d / %d\n", i, GRADUS_U);

        for (int j = 0; j < GRADUS_V; j++) {
            /* Quattuor vertices quadrati */
            size_t vi[4] = {
                (size_t)i       * (GRADUS_V + 1) + j,
                (size_t)(i + 1) * (GRADUS_V + 1) + j,
                (size_t)(i + 1) * (GRADUS_V + 1) + (j + 1),
                (size_t)i       * (GRADUS_V + 1) + (j + 1)
            };

            /* Proicere in spatium scrinii */
            double sx[4], sy[4], sz[4];
            int omnes_visibiles = 1;

            for (int k = 0; k < 4; k++) {
                if (!proicere(&cam, puncta[vi[k]],
                              &sx[k], &sy[k], &sz[k])) {
                    omnes_visibiles = 0;
                    break;
                }
            }
            if (!omnes_visibiles) continue;

            /* Triangulum primum: 0-1-2 */
            triangulum_reddere(
                sx[0], sy[0], sz[0], puncta[vi[0]], normae[vi[0]],
                sx[1], sy[1], sz[1], puncta[vi[1]], normae[vi[1]],
                sx[2], sy[2], sz[2], puncta[vi[2]], normae[vi[2]],
                cam.positio);

            /* Triangulum secundum: 0-2-3 */
            triangulum_reddere(
                sx[0], sy[0], sz[0], puncta[vi[0]], normae[vi[0]],
                sx[2], sy[2], sz[2], puncta[vi[2]], normae[vi[2]],
                sx[3], sy[3], sz[3], puncta[vi[3]], normae[vi[3]],
                cam.positio);

            triangula_reddita += 2;
        }
    }

    fprintf(stderr, "Triangula reddita: %ld\n", triangula_reddita);

    /* ------------------------------------------------------------ */
    /*  Imaginem in tabulam PPM scribere                            */
    /* ------------------------------------------------------------ */
    const char *via_ppm = "torus_planus.ppm";
    fprintf(stderr, "Imaginem scribens: %s\n", via_ppm);

    FILE *fasciculus = fopen(via_ppm, "wb");
    if (!fasciculus) {
        fprintf(stderr, "ERROR: fasciculum aperire non possum!\n");
        return 1;
    }
    fprintf(fasciculus, "P6\n%d %d\n255\n", LATITUDO_IMG, ALTITUDO_IMG);
    fwrite(tabula_imaginis, 1, n_pixels * 3, fasciculus);
    fclose(fasciculus);

    /* Memoriam liberare */
    free(tabula_imaginis);
    free(tabula_profunditatis);
    free(puncta);
    free(normae);

    fprintf(stderr, "\nOpus perfectum est.\n");
    return 0;
}
