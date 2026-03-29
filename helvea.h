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

/*
 * helvea_fundum_implere — bitmap toroidalem ut fundum tabulae pingit.
 * Bitmap per translationem (delta_x, delta_y) movetur et toroidaliter
 * involvitur. Profunditas non mutatur (manet 1e30).
 *
 * Cosmologia toroidalis quadrata:
 *
 *   Relativitas generalis metricam localem determinat, non topologiam
 *   globalem. Aequationes campi Einstein R_μν - ½Rg_μν + Λg_μν = 8πGT_μν
 *   curvaturam cum materia-energia ligant, sed de connexitate spatii
 *   nihil dicunt. Universum spatialiter planum est (Ω_k = 0.001 ± 0.002,
 *   Planck 2018). Topologia T³ — torus tribus dimensionibus — cum
 *   omnibus observationibus concordat et suppressionem potentiae CMB
 *   in scalis magnis naturaliter explicat, quam R³ non explicat.
 *
 *   Torus planus quadratus T² construitur per identificationem laterum
 *   oppositorum quadrati [0,L) × [0,L). Metrica Euclidea ds² = dx² + dy²
 *   ubique valet — curvatura Gaussiana K = 0 in omni puncto. Geometria
 *   locale indistinguibilis a plano infinito. Sed geodeticae (lineae
 *   rectae) post distantiam L ad originem redeunt. Spatium finitum est
 *   sine marginibus — exacte ut superficies nostri bitmap.
 *
 *   Theorema Nash (1954) et schema Nash-Kuiper (1955) demonstrant torum
 *   planum T² in R³ isometrice immergi posse per immersionem C¹ — i.e.
 *   superficiem in spatio tridimensionali quae distantias intrinsecas
 *   perfecte conservat. Hoc est paradoxale: torus in R³ debet curvari,
 *   sed metrica intrinseca plana manet. Solutio: corrugationes infinitae
 *   (fractales in limite) metricam compensant.
 *
 *   Helvea team (Borrelli, Jabrane, Lazarus, Thibert, 2012) primam
 *   visualizationem concretam huius immersionis computaverunt.
 *   Superficies characteristice "corrugata" apparet — crispae in crispis.
 *
 *   In hac scaena: observator intus torum T² stellarum sedet.
 *   Campus stellarum est universum — finitum, planum, toroidale.
 *   Translatio fundi cum rotatione camerae perioditatem cosmicam
 *   demonstrat: movendo per spatium, eaedem stellae repetuntur.
 *   Si satis longe spectares, tua propria terga videres.
 *
 *   Torus Hevea in centro est eiusdem topologiae T² imago concreta:
 *   quomodo spatium planum finitum in spatio Euclidiano R³ habitare
 *   possit. Intus unum T² sumus, alterum T² spectamus.
 *
 *   Signa observabilia:
 *   - Circuli congruentes in CMB: superficies ultimae dispersionis
 *     (r = 46 Gly) in universo T³ seipsam intersecat. Paria
 *     circulorum cum fluctuationibus identicis apparent
 *     (Cornish, Spergel & Starkman 1998). Investigationes WMAP
 *     et Planck magnitudinem domenii fundamentalis constringunt.
 *   - Suppressio potentiae in scalis magnis: modus longissimus
 *     in T³ est λ_max = L. Fluctuationes CMB in scalis magnis
 *     (ℓ < ~6) suppressae sunt — quadrupolum debile
 *     (S₁/₂ < 1200 μK², ubi ΛCDM expectat ~8000 μK²).
 *     Topologia compacta hoc naturaliter explicat
 *     (Luminet et al. 2003, Aurich et al. 2005).
 *   - Imagines repetitae: eaedem galaxiae in pluribus directionibus
 *     visibiles sunt si L intra horizontem cadit. Distantia
 *     periodica determinanda restat.
 *
 *   Referentiae:
 *
 *   Antiquae:
 *   - Parmenides (~500 a.C.n.) Περὶ Φύσεως, fragmentum 8:
 *     "Quod Est" finitum et sphaericum est — "instar massae sphaerae
 *     bene rotundae, a centro aequaliter in omnem partem." Primus
 *     argumentum rigorosum pro spatio finito et completo. Infinitum
 *     (ἄπειρον) imperfectum et incompletum est — finitum perfectum.
 *   - Archytas Tarentinus (~420 a.C.n.) argumentum limitis:
 *     si ad extremum caeli stas et hastam extendis, aut aliquid
 *     impedit aut hasta progrediatur — ergo semper ulterius.
 *     Hoc argumentum per duo millennia pro infinitate spatii
 *     adhibitum est. Sed torus T³ eleganter solvit: hasta
 *     progreditur sine impedimento et sine margine, sed post
 *     distantiam L ad initium redit. Finitum sine fine.
 *   - Aristoteles (~340 a.C.n.) De Caelo II.4: universum finitum,
 *     sphaericum, sine vacuo ultra. Corpus extra sphaeram impossibile
 *     quia omne corpus locum naturalem habet, et omnes loci intra
 *     sphaeram sunt. Archytae hastam non accipit — sed torus
 *     Aristoteli placuisset: finitum, completum, sine "ultra."
 *   - Lucretius (~55 a.C.n.) De Rerum Natura I.968-983:
 *     argumentum iaculi ad marginem — si iaculum proicis ad finem
 *     spatii, aut exit aut impeditur, utroque modo spatium ultra est.
 *     Reformulatio Archytae. Iterum: torus respondet.
 *   - Iohannes Philoponus (~530) Contra Aristotelem: spatium
 *     finitum possibile sine margine si in se recurrit. Commentator
 *     Alexandrinus qui finitudinem Aristotelicam cum argumentis
 *     Archytae reconciliare temptavit.
 *   - Nicolaus Cusanus (1440) De Docta Ignorantia II.11-12:
 *     "machina mundi habebit centrum ubique et circumferentiam
 *     nusquam." Universum infinitum sine centro — vel potius:
 *     omne punctum centrum est. Topologia T³ hoc literaliter
 *     efficit: nulla positio privilegiata, omnia puncta aequivalentia.
 *
 *   Modernae (ante relativitatem):
 *   - Riemann (1854) "Über die Hypothesen, welche der Geometrie
 *     zu Grunde liegen" — spatium physicum non necessarie Euclideum;
 *     curvatura et topologia quaestiones empiricae sunt.
 *   - Clifford (1873) "On the Space-Theory of Matter" — spatium
 *     physicum curvum est; materia = curvatura spatii. Ante Einstein!
 *   - Schwarzschild (1900) "Über das zulässige Krümmungsmaass
 *     des Raumes" — primus topologias cosmicas non-triviales
 *     systematice tractavit, includens torum T³. Ante relativitatem!
 *   - Friedmann (1924) "Über die Möglichkeit einer Welt mit
 *     konstanter negativer Krümmung des Raumes" — notavit aequationes
 *     cosmologicas topologiam non determinare.
 *
 *   Immersiones:
 *   - Nash (1954) "C¹ isometric imbeddings" — demonstravit omnem
 *     varietatem Riemannianam brevem in R^n isometrice immergi posse
 *     per immersionem C¹, dummodo n sufficiat. Resultatum "absurdum"
 *     quod torum planum in R³ collocare sinit.
 *   - Kuiper (1955) "On C¹-isometric imbeddings" — Nash resultatum
 *     ad codimensionem 1 extendit: sufficit R³ pro superficie.
 *     Immersio necessite non-differentiabilis (C¹ sed non C²) est.
 *   - Ellis (1971) "Topology and cosmology" — Gen. Rel. Grav. 2, 7.
 *     Revisio systematica topologiarum cosmologicarum.
 *
 *   Modernae:
 *   - Cornish, Spergel & Starkman (1998) "Circles in the sky"
 *     — methodus detectionis topologiae per CMB. astro-ph/9602039.
 *   - Luminet, Weeks, Riazuelo, Lehoucq & Uzan (2003) "Dodecahedral
 *     space topology" — Nature 425, 593. Suppressio quadrupoli.
 *   - Aurich, Lustig & Steiner (2005) "CMB alignment in multi-
 *     connected universes" — Class. Quant. Grav. 22, 3443.
 *   - Borrelli, Jabrane, Lazarus & Thibert (2012) "Flat tori in
 *     three-dimensional space and convex integration" — PNAS 109,
 *     7218. Prima visualizatio concreta immersionis Nash-Kuiper.
 *     Torus Hevea hic depictus ex hoc opere derivatur.
 *   - Planck Collaboration (2020) "Planck 2018 results. VII.
 *     Isotropy and statistics" — A&A 641, A7. Limites topologici.
 *   - Vaudrevange, Starkman, Cornish & Spergel (2012) "Constraints
 *     on the topology of the Universe" — Phys. Rev. D 86, 083526.
 *     Limites inferiores magnitudinis domenii fundamentalis.
 */
void helvea_fundum_implere(helvea_tabula_t *t,
                           const unsigned char *bitmap,
                           int bm_latitudo, int bm_altitudo,
                           int delta_x, int delta_y);

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
 * Omnia quadrata proicit et reddit.
 * Vocans tabulam purgare debet ante invocationem.
 * ================================================================ */

void helvea_scaenam_reddere(
    helvea_tabula_t *t,
    const Vec3 *puncta, const Vec3 *normae,
    int gradus_u, int gradus_v,
    const Camera *cam,
    helvea_illuminare_fn illum_fn,
    helvea_pixel_fn pixel_fn);

#endif /* HELVEA_H */
