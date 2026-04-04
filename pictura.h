/*
 * pictura.h — camera, tabula imaginis, rasterizatio
 *
 * Camera perspectiva, framebuffer cum z-buffer,
 * scriptio pixelorum, rasterizatio triangulorum,
 * redditio scaenae. Generica — nullam dependentiam
 * a superficie specifica habet.
 */

#ifndef PICTURA_H
#define PICTURA_H

#include "vectores.h"
#include "color.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * camera perspectiva
 * ================================================================ */

typedef struct {
    vec3_t   positio;
    vec3_t   ante;
    vec3_t   dextrum;
    vec3_t   sursum;
    double focalis;
} camera_t;

camera_t cameram_constituere(vec3_t positio, vec3_t scopus);

int pictura_proicere(
    const camera_t *cam, vec3_t p,
    double *scr_x, double *scr_y, double *prof,
    int latitudo, int altitudo
);

/* ================================================================
 * tabula imaginis (framebuffer)
 * ================================================================ */

typedef struct {
    unsigned char *imaginis;
    double        *profunditatis;
    int            latitudo;
    int            altitudo;
    int            bytes_pixel;     /* 3 = RGB (PPM), 4 = BGRA (SDL) */
} tabula_t;

/* tabulam purgare (fundum obscurum + profunditas infinita) */
void tabulam_purgare(tabula_t *t);

/*
 * fundum_implere — bitmap toroidalem ut fundum tabulae pingit.
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
void fundum_implere(
    tabula_t *t,
    const unsigned char *bitmap,
    int bm_latitudo, int bm_altitudo,
    int delta_x, int delta_y
);

/* pixel scribere cum profunditate — RGB ordo (PPM) */
void pixel_rgb(
    tabula_t *t, int x, int y,
    double prof, color_t c
);

/* pixel scribere cum profunditate — BGRA ordo (SDL ARGB8888) */
void pixel_bgra(
    tabula_t *t, int x, int y,
    double prof, color_t c
);

/* ================================================================
 * rasterizatio triangulorum
 *
 * illuminare_fn: functio quae colorem pro puncto computat.
 * pixel_fn: functio quae pixel scribit.
 * ================================================================ */

typedef color_t (*illuminare_fn)(vec3_t punct, vec3_t norm, vec3_t oculus);

typedef void (*pixel_fn)(
    tabula_t *t, int x, int y,
    double prof, color_t c
);

void triangulum_reddere(
    tabula_t *t,
    double sx0, double sy0, double sz0, vec3_t p0, vec3_t n0,
    double sx1, double sy1, double sz1, vec3_t p1, vec3_t n1,
    double sx2, double sy2, double sz2, vec3_t p2, vec3_t n2,
    vec3_t oculus,
    illuminare_fn illum_fn,
    pixel_fn pixel_fn
);

/* ================================================================
 * scaenam reddere
 *
 * Omnia quadrata proicit et reddit.
 * Vocans tabulam purgare debet ante invocationem.
 * ================================================================ */

void scaenam_reddere(
    tabula_t *t,
    const vec3_t *puncta, const vec3_t *normae,
    int gradus_u, int gradus_v,
    const camera_t *cam,
    illuminare_fn illum_fn,
    pixel_fn pixel_fn
);

#endif /* PICTURA_H */
