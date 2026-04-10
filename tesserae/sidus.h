/*
 * sidus.h — sidera singula, renderer
 *
 * Reddit singula sidera in fenestra 64x64 pixelorum.
 *
 * Fundamenta physica:
 *
 *   Color stellae ex radiatio corporis nigri (Planck 1900) derivatur.
 *   Lex Wien: λ_max = b/T ubi b = 2.898e-3 m·K.
 *   Approximatio Tanner Helland RGB adhibetur — errore < 1% in
 *   campo 1000-40000 K contra integrationem Planckianam plenam
 *   per functiones colorimetricas CIE 1931.
 *
 *   Magnitudo apparens sequitur scalam Pogson (1856):
 *   m = -2.5 log10(F/F_0). Differentia 5 magnitudinum = factor 100
 *   in fluxu. Oculus nudus videt usque ad mag ~6 (Bortle 1).
 *
 *   Distributio generum stellarum ex functione initiali massae
 *   (IMF, Kroupa 2001) derivatur: dN/dM ∝ M^(-2.3) pro M > 0.5 M☉.
 *   Hoc significat stellas parvas (M, K) enormiter dominare.
 *   Gigantes et supergigantes rarissimi sunt quia:
 *   (a) progenitores massivi rari per IMF,
 *   (b) phase gigantis brevis (~10% vitae stellae).
 *
 *   Densitas stellaris in vicinitate solari: ~0.14 pc^(-3)
 *   (Reylé et al. 2021). In caelo Bortle 2 (ut Vallis Mortis)
 *   ~4500 stellae oculo nudo visibiles, ~15000 cum averted vision.
 */

#ifndef SIDUS_H
#define SIDUS_H

#include "../tessella.h"
#include "../color.h"
#include "../instrumentum.h"

#include <math.h>

#define SIDUS_FENESTRA  64      /* latitudo et altitudo fenestrae sideris */

/*
 * Genus sideris — ex diagrammate Hertzsprung-Russell (1911).
 *
 * Sequentia principalis: fusio H→He in nucleo. 90% omnium stellarum.
 *   Classes spectrales O B A F G K M per temperaturam superficiei.
 *   Sol = G2V, T_eff = 5778 K, mag_abs = +4.83.
 *
 * Nanum album: nucleus degenere post eiectionem nebulae planetariae.
 *   Massa ~0.6 M☉ in volumine Terrae. Pressure degenerationis
 *   electronium (Chandrasekhar 1931, limitus 1.4 M☉).
 *   T_eff = 4000-150000 K, refrigerans per ~10^10 annos.
 *   ~6% stellarum visibilium (Holberg et al. 2008).
 *
 * Gigas rubrum: post exhaustionem H in nucleo, involucrum expandit.
 *   Radius 10-100 R☉, T_eff = 2500-5000 K. Phase brevis (~10^8 a).
 *   Rara in campo visibili quia (a) progenitores 1-8 M☉ rari per IMF,
 *   (b) tempus in hac phase breve.
 *
 * Supergigas: stellae massivissimae (>8 M☉), omnes classes spectrales.
 *   Radius 30-1500 R☉. Vita brevissima (~10^6-10^7 a).
 *   Extremiter rara: ~0.001% stellarum, sed lucidissimae (M_abs -5 ad -9).
 *   Betelgeuse (M2Iab), Rigel (B8Ia), Deneb (A2Ia).
 *
 * Neutronium: post supernovam, nucleus > 1.4 M☉ collapsus.
 *   Radius ~10 km, densitas ~10^17 kg/m^3. T_superficiei ~10^6 K
 *   (initiale), refrigerans. Radiat maxime in X-ray.
 *   Pulsar: fasciculus radio rotans (Hewish & Bell 1967).
 *   Campus magneticus ~10^8-10^15 T (magnetar).
 *
 * Magnetar: stella neutronium cum campo magnetico extremo.
 *   B ~10^9-10^11 T (Duncan & Thompson 1992).
 *   Cf pulsar ordinarius: B ~10^8 T. Cf Terra: B ~5×10^-5 T.
 *   Campus tam fortis ut vacuum QED birefringentiam exhibeat
 *   (Heisenberg & Euler 1936): photones in duos modos polarizantur
 *   cum velocitatibus diversis. Prope superficiem, B > B_Schwinger
 *   (4.4×10^9 T) et paria e+e- ex vacuo creari possunt.
 *   Soft Gamma Repeaters (SGR) et Anomalous X-ray Pulsars (AXP)
 *   nunc ut magnetares identificantur (Mereghetti 2008).
 *   SGR 1806-20 (2004): eruptio γ cum E ~10^39 J in 0.2 s,
 *   ionosphaeram Terrae ex 50000 ly perturbavit.
 *   Apparentia: nucleus intensissimus cum halo asymmetrico
 *   ex birefringentia vacui — lux in duos modos separatur,
 *   creans imaginem duplicem vel elongatam.
 *
 * Crystallinum (RARUM — rarum):
 *   Stella ex materia quark in phase "color-flavor locked" (CFL).
 *   Alford, Rajagopal & Wilczek (1999) praedixerunt materiam quark
 *   densam transituram in statum CFL ubi quarks in tribus coloribus
 *   et tribus saporibus crystallinam symmetriam formant.
 *   Gap energiae ~100 MeV spectrale emissionem in lineas discretas
 *   frangit — non continuum Plancki sed colores puros spectrales.
 *   Apparentia: filamenta luminosa in coloribus spectralibus discretis
 *   irradiantia ex nucleo — quasi "globus Koosh" stellaris.
 *   Firmum theoretice (Alford+ 1999, Rajagopal+ 2001).
 *
 * Galaxia (objectum distans):
 *   Galaxiae sunt systemata gravitatione ligata stellarum, gas,
 *   pulveris, et materiae obscurae. primam distinctionem a nebulis
 *   Hubble (1925) fecit demonstrans Cepheidas in M31 (Andromeda)
 *   distantiam ~900 kpc (hodie 780 kpc, Riess+ 2012) habere —
 *   longe extra Viam Lacteam.
 *
 *   Classificatio morphologica Hubble (1926, "tuning fork"):
 *
 *   GALAXIA_ELLIPTICA (E0-E7):
 *     Systemata spheroidalia stellarum veterum (Populatio II).
 *     Profilo de Vaucouleurs (1948): I(r) ∝ exp(-7.67·((r/r_e)^{1/4}-1))
 *     ubi r_e = radius effectivus (dimidium lucis inclusae).
 *     Parum gas et pulveris → formatio stellarum fere nulla.
 *     Colores rubri-aurei (stellae K/M gigantes dominantur).
 *     Ellipticitas apparens ab inclinatione et forma intrinseca
 *     pendet: E0 = circularis (sphaerica vel face-on oblata),
 *     E7 = maxime elongata. classificatio n = 10(1-b/a).
 *     Exempla: M87 (E0, gigans in Virgo, cum jet relativistica!),
 *     M32 (cE2, nana compacta, satelles M31).
 *     Massae: 10⁷ M☉ (nanae) — 10¹³ M☉ (cD gigantes).
 *     ~15% galaxiarum luminosarum (Lintott+ 2008, Galaxy Zoo).
 *
 *   GALAXIA_SPIRALIS (Sa-Sd, classificatio Hubble extensa):
 *     Discus rotans cum brachiis spiralibus et nucleus centralis.
 *     Theoria densitatis undae (Lin & Shu 1964): brachia non sunt
 *     materialia sed undae densitatis in disco — stellae intrant
 *     et exeunt, sed compressio gas in unda formationem stellarum
 *     novam excitat, ergo brachia caerulea apparent (stellae O/B
 *     iuvenes, vita ~10⁷ a, moriuntur ante egressum e brachio).
 *     Sa: nucleus magnus, brachia stricte involuta.
 *     Sd: nucleus parvus, brachia laxa et fragmentata.
 *     Relatio Tully-Fisher (1977): L ∝ v_rot^4 — luminositas
 *     ex velocitate rotationis determinatur.
 *     Curvae rotationis planae (Rubin & Ford 1970, Rubin+ 1980)
 *     primam evidentiam materiae obscurae in galaxiis dederunt:
 *     v(r) = const ad radios magnos, ergo M(r) ∝ r.
 *     ~60% galaxiarum luminosarum.
 *     Exempla: M31 (Sb, mag_V=3.4), M51 (Sc, "Whirlpool"),
 *     M101 (Scd, "Pinwheel"), Via Lactea ipsa (SBbc).
 *
 *   GALAXIA_SPIRALIS_BARRATA (SBa-SBd):
 *     Ut spirales sed cum barra stellari centrali.
 *     Barra ex instabilitate orbitali in disco oritur
 *     (Combes & Sanders 1981, Athanassoula 2003):
 *     resonantiae orbitales (ILR, CR, OLR) materiam in
 *     orbitas elongatas dirigunt. barra gas ad centrum
 *     fundit, nucleum activum (AGN) potentialiter alens.
 *     ~65% spiralium barram habent (Eskridge+ 2000,
 *     Menéndez-Delmestre+ 2007). Via Lactea barrata est
 *     (longitudine ~5 kpc, Wegg+ 2015).
 *     ~30% omnium galaxiarum luminosarum.
 *
 *   GALAXIA_LENTICULARIS (S0):
 *     Inter ellipticas et spirales in schema Hubble.
 *     Discus sine brachiis manifestis, nucleus prominens.
 *     Hypothesis: spirales quae gas amiserunt per
 *     "ram pressure stripping" in clusteribus (Gunn & Gott
 *     1972) vel per "strangulation" (Larson+ 1980).
 *     Frequentes in clusteribus densis (Dressler 1980,
 *     relatio morphologia-densitas).
 *     ~15% galaxiarum luminosarum.
 *
 *   GALAXIA_IRREGULARIS (Irr):
 *     Forma asymmetrica, sine structura regulari.
 *     Saepe ex interactione gravitationali (Toomre & Toomre
 *     1972, "Merging Sequence"). Nubes Magellanicae (LMC =
 *     Irr/SBm, SMC = Irr) exempla proxima. Gas abundans,
 *     formatio stellarum activa (starburst).
 *     ~10% galaxiarum. dominantur in universo distante
 *     (z > 1, Conselice+ 2005) quia mergers frequentiores.
 *
 *   Distributio magnitudinis — functio luminositatis Schechter (1976):
 *     φ(L) dL = φ* (L/L*)^α exp(-L/L*) dL/L*
 *     ubi α ≈ -1.25, L* ≈ 2×10¹⁰ L☉, φ* ≈ 0.012 Mpc⁻³.
 *     hoc significat galaxias debiles enormiter dominare:
 *     pro quaque galaxia lucida ut M31 (M_V ≈ -21.5),
 *     centena galaxiae nanae (M_V > -16) existunt.
 *     in campo stellarum: pleraeque galaxiae ut maculae parvae
 *     diffusae apparent, raro magna ut Andromeda.
 *
 *   Apparentia per morphologiam:
 *     Elliptica: nebula diffusa elliptica, colore aureo-rubro.
 *     Spiralis face-on: nucleus + brachia curva caerulea.
 *     Spiralis edge-on: linea tenuis cum tumore nuclei centrali
 *       et fascia pulveris (similis magnetari sed latior, diffusior).
 *     Lenticularis: discus diffusus cum nucleo lucido.
 *     Irregularis: macula asymmetrica informis, nodis luminosis.
 */
typedef enum {
    SIDUS_NANUM_ALBUM,
    SIDUS_SEQUENTIA,
    SIDUS_GIGAS_RUBRUM,
    SIDUS_SUPERGIGAS,
    SIDUS_NEUTRONIUM,
    SIDUS_CRYSTALLINUM,
    SIDUS_MAGNETAR,         /* magnetar */
    SIDUS_GALAXIA,          /* galaxia distans */
    SIDUS_VAGANS,
    SIDUS_NUMERUS
} sidereus_t;

extern const char *sidus_nomina_generum[SIDUS_NUMERUS];

/* proprietates communes omnium siderum (diminutiva) */
typedef struct {
    double   magnitudo;     /* magnitudo apparens (0=lucidissimum, 6=vix visibile) */
    double   temperatura;   /* temperatura coloris (Kelvin), 2000-40000 */
} sidulum_t;

/* ================================================================
 * genera siderum
 * ================================================================ */

#include "sidera/nanum_album.h"
#include "sidera/sequentia.h"
#include "sidera/gigas_rubrum.h"
#include "sidera/supergigas.h"
#include "sidera/neutronium.h"
#include "sidera/crystallinum.h"
#include "sidera/magnetar.h"
#include "sidera/galaxia.h"
#include "sidera/vagans.h"

/* ================================================================
 * sidus_t — unio omnium generum siderum
 *
 * Continet quodlibet genus sideris in acervo.
 * Accessio basis: sidus.sidulum.genus, sidus.sidulum.magnitudo, ...
 * Accessio generis: sidus.vagans.phase, sidus.galaxia.morphologia, ...
 * ================================================================ */

typedef struct {
    sidereus_t qui;

    union {
        nanum_album_t  nanum_album;
        sequentia_t    sequentia;
        gigas_rubrum_t gigas_rubrum;
        supergigas_t   supergigas;
        neutronium_t   neutronium;
        crystallinum_t crystallinum;
        magnetar_t     magnetar;
        galaxia_t      galaxia;
        vagans_t       vagans;
    } ubi;
} sidus_t;

/* ================================================================
 * sidus reddere
 *
 * Reddit sidus in fenestram 64x64.
 * fenestra: RGBA buffer, SIDUS_FENESTRA * SIDUS_FENESTRA * 4 bytes.
 * alpha indicat quantum pixel a sidere afficitur.
 * ================================================================ */

void sidus_reddere(
    unsigned char *fenestra,
    const sidus_t *sidus,
    const instrumentum_t *instrumentum
);

/* sidus ex ISON legere — implet sidus_t ex chorda ISON */
void sidus_ex_ison(sidus_t *sidus, const char *ison);

/* colorem ex temperatura (Kelvin) per Planck approximare */
color_t sidus_temperatura_ad_colorem(double kelvin);

#endif /* SIDUS_H */
