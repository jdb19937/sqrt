/*
 * helvea.h — torus planus corrugatus, bibliotheca communis
 *
 * Typi, operationes vectoriales, superficies tori Hevea,
 * camera perspectiva, illuminatio, rasterizatio.
 * Omnia programmata tori hanc bibliothecam adhibent.
 */

#ifndef HELVEA_H
#define HELVEA_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * constantes
 * ================================================================ */

#define PI_GRAECUM  3.14159265358979323846
#define DUO_PI      (2.0 * PI_GRAECUM)

#define HELVEA_STRATA_CORRUG    5
#define HELVEA_RADIUS_MAIOR     1.0
#define HELVEA_RADIUS_MINOR     0.42

/* ================================================================
 * typi mathematici
 * ================================================================ */

typedef struct { double x, y, z; } Vec3;
typedef struct { double r, g, b; } Color;

/* ================================================================
 * operationes vectoriales (inline in capite)
 * ================================================================ */

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

static inline unsigned char gamma_corrigere(double valor)
{
    if (valor < 0.0) valor = 0.0;
    if (valor > 1.0) valor = 1.0;
    return (unsigned char)(pow(valor, 1.0 / 2.2) * 255.0 + 0.5);
}

/* ================================================================
 * superficies tori Hevea
 * ================================================================ */

typedef enum {
    HELVEA_CORRUGATA,   /* corrugatio multi-scalaris originalis */
    HELVEA_ITERATA,     /* integrationes convexae iteratae (Borrelli) */
    HELVEA_SPIRALIS,    /* corrugatio spiralis */
    HELVEA_NORMALIS     /* corrugatio per normam superficiei */
} helvea_methodus_t;

#define HELVEA_NUMERUS_METHODORUM 4

extern const char *helvea_nomina_methodorum[HELVEA_NUMERUS_METHODORUM];

/*
 * helvea_superficies — punctum superficiei.
 * methodus: algorithmum corrugationis eligit.
 */
Vec3 helvea_superficies(double u, double v,
                        double radius_maior, double radius_minor,
                        helvea_methodus_t methodus);

/* norma per differentias finitas centrales */
Vec3 helvea_norma(double u, double v,
                  double radius_maior, double radius_minor,
                  helvea_methodus_t methodus);

/*
 * superficiem praecomputare.
 * vertices marginales copiantur ut superficies claudatur
 * (corrugatio non est periodica in [0,2π]).
 */
void helvea_superficiem_computare(Vec3 *puncta, Vec3 *normae,
                                  int gradus_u, int gradus_v,
                                  double radius_maior, double radius_minor,
                                  helvea_methodus_t methodus);

/* ================================================================
 * camera perspectiva
 * ================================================================ */

typedef struct {
    Vec3   positio;
    Vec3   ante;
    Vec3   dextrum;
    Vec3   sursum;
    double focalis;
} Camera;

Camera helvea_cameram_constituere(Vec3 positio, Vec3 scopus);

int helvea_proicere(const Camera *cam, Vec3 p,
                    double *scr_x, double *scr_y, double *prof,
                    int latitudo, int altitudo);

/* ================================================================
 * illuminatio simplex (Blinn-Phong, aurum)
 * ================================================================ */

Color helvea_illuminare(Vec3 punct, Vec3 norm, Vec3 oculus);

/* ================================================================
 * themata — colores et modi lucis
 *
 * Systema thematum sine ulla dependentia SDL.
 * Programmata thema per indicem eligunt, deinde
 * helvea_illuminare_thema() ut illuminare_fn transmittunt.
 * ================================================================ */

typedef enum {
    LUX_IRIDESCENS,     /* pellicula tenuis — color ex angulo visus */
    LUX_PLANUS,         /* diffusum matte (half-Lambert) */
    LUX_TABULATA,       /* cel shading — gradus discreti */
    LUX_RAMPA           /* color ex NdotL in rampam 4 colorum */
} helvea_modus_lucis_t;

typedef struct {
    const char          *nomen;
    helvea_modus_lucis_t modus;
    int                  pfx;           /* vexilla post-effectuum (bit OR) */
    double               ir_freq;
    Color                ir_phase;
    double               ir_saturatio;
    Color                materia;
    Color                fresnel_color;
    double               fresnel_vis;
    double               spec_potentia;
    double               spec_vis;
    double               ambiens;
    int                  cel_gradus;
    int                  posteriza_niv;
    Color                rampa[4];
} helvea_thema_t;

/* vexilla post-effectuum (combinabilia per OR) */
#define HELVEA_PFX_NULLUS    0
#define HELVEA_PFX_LINEAE    1
#define HELVEA_PFX_POSTERIZA 2
#define HELVEA_PFX_NIGRESCO  4
#define HELVEA_PFX_GRANUM    8

/* tabula thematum et numerus */
extern const helvea_thema_t helvea_themata[];
extern const int helvea_numerus_thematum;

/* thema activum — programmata hunc indicem ponunt ante illuminationem */
extern int helvea_index_thematis;

/*
 * helvea_illuminare_thema — illuminatio secundum thema activum.
 * Signaturam helvea_illuminare_fn habet, ergo directe
 * ut callback transmitti potest.
 */
Color helvea_illuminare_thema(Vec3 punct, Vec3 norm, Vec3 oculus);

/* ================================================================
 * tabula imaginis (framebuffer)
 * ================================================================ */

typedef struct {
    unsigned char *imaginis;
    double        *profunditatis;
    int            latitudo;
    int            altitudo;
    int            bytes_pixel;     /* 3 = RGB (PPM), 4 = BGRA (SDL) */
} helvea_tabula_t;

/* tabulam purgare (fundum obscurum + profunditas infinita) */
void helvea_tabulam_purgare(helvea_tabula_t *t);

/* pixel scribere cum profunditate — RGB ordo (PPM) */
void helvea_pixel_rgb(helvea_tabula_t *t, int x, int y,
                      double prof, Color c);

/* pixel scribere cum profunditate — BGRA ordo (SDL ARGB8888) */
void helvea_pixel_bgra(helvea_tabula_t *t, int x, int y,
                       double prof, Color c);

/* ================================================================
 * rasterizatio triangulorum
 *
 * illuminare_fn: functio quae colorem pro puncto computat.
 *   si NULL, helvea_illuminare adhibetur.
 * pixel_fn: functio quae pixel scribit.
 * ================================================================ */

typedef Color (*helvea_illuminare_fn)(Vec3 punct, Vec3 norm, Vec3 oculus);

typedef void (*helvea_pixel_fn)(helvea_tabula_t *t, int x, int y,
                                double prof, Color c);

void helvea_triangulum_reddere(
    helvea_tabula_t *t,
    double sx0, double sy0, double sz0, Vec3 p0, Vec3 n0,
    double sx1, double sy1, double sz1, Vec3 p1, Vec3 n1,
    double sx2, double sy2, double sz2, Vec3 p2, Vec3 n2,
    Vec3 oculus,
    helvea_illuminare_fn illum_fn,
    helvea_pixel_fn pixel_fn);

/* ================================================================
 * scaenam reddere (auxilium commune)
 *
 * Tabulam purgat, omnia quadrata proicit et reddit.
 * ================================================================ */

void helvea_scaenam_reddere(
    helvea_tabula_t *t,
    const Vec3 *puncta, const Vec3 *normae,
    int gradus_u, int gradus_v,
    const Camera *cam,
    helvea_illuminare_fn illum_fn,
    helvea_pixel_fn pixel_fn);

#endif /* HELVEA_H */
