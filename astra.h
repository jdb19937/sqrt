/*
 * astra.h — sidera et planetae, bitmap caeli nocturni
 *
 * Generat campum stellarum cum topologia toroidali.
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

#ifndef ASTRA_H
#define ASTRA_H

#include <math.h>

/* ================================================================
 * constantes
 * ================================================================ */

#define ASTRA_PI        3.14159265358979323846
#define ASTRA_DUO_PI   (2.0 * ASTRA_PI)
#define ASTRA_FENESTRA  64      /* latitudo et altitudo fenestrae sideris */

/* ================================================================
 * typi
 * ================================================================ */

typedef struct { double r, g, b, a; } astra_color_t;

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
    SIDUS_PLANETA,
    SIDUS_NUMERUS
} astra_genus_t;

extern const char *astra_nomina_generum[SIDUS_NUMERUS];

/*
 * Proprietates instrumenti optici — NON sideris ipsius.
 *
 * Spiculae diffractionis oriuntur ex struttura telescopii:
 *   - Newtonianum: 4 spiculae ex araneo secundario (4 bracchia)
 *   - Cassegrain: 4 spiculae (plerumque)
 *   - JWST: 6 spiculae ex segmentis hexagonalibus
 *   - Refractor: nullae spiculae (apertum circulare)
 *
 * Haec effecta sunt functio instrumenti, non stellae.
 * Stella ipsa est punctum (angulus << resolutio).
 * Halo (bloom) ex dispersione in atmosphaera vel opticis oritur.
 *
 * Separatio haec physice iustificata: eadem stella per
 * refractorem nullas spiculas ostendit, per Newtonianum quattuor.
 */
/*
 * Saturatio coloris:
 *   Oculus humanus: responsio spectrale per curvas CIE 1931 x̄ȳz̄.
 *   CCD (silicon): responsio latior, praesertim in rubro et infrarubro.
 *   Quantum efficiency CCD ~80% vs oculus ~5% in rubro extremo.
 *   Processio astrophotographica semper saturationem auget
 *   ut differentiae spectrales visibiles fiant.
 *   saturatio = 1.0: naturalis (oculus). >1.0: CCD/processita.
 *
 * Aberratio chromatica:
 *   Lentes refringentes indicem refractionis λ-dependentem habent
 *   (dispersio: n(λ) per formulam Cauchy vel Sellmeier).
 *   Lux rubra longius a centro focali cadit quam caerulea.
 *   Effectus: fimbriae coloratae circa stellas lucidas.
 *   Achromaticae (doublet) reducunt sed non eliminant.
 *   Apochromaticae (triplet) fere eliminant.
 *   aberratio = 0: perfectum. >0: pixeles dislocatio per canalem.
 *
 * Visio atmosphaerica ("seeing"):
 *   Turbulentia atmosphaerica (Kolmogorov 1941, "De la structure
 *   locale de la turbulence dans un fluide visqueux") causat
 *   fluctuationes indicis refractionis in cellulis convectivis
 *   (Fried 1966, "Optical resolution through a randomly
 *   inhomogeneous medium"). Parametrus Fried r_0 (diametrus
 *   cohaerens) typice 5-20 cm in situ bono. Seeing disc FWHM:
 *   θ = 0.98 λ/r_0 (Roddier 1981, "The effects of atmospheric
 *   turbulence in optical astronomy"). Situs optimi: Mauna Kea
 *   0.4" (Schöck+ 2009), Paranal 0.66" (Sarazin & Roddier 1990),
 *   Dome C Antarctica 0.27" (Lawrence+ 2004). In imagine,
 *   seeing approximatur per convolutionem Gaussianam cum σ ∝ r_0.
 *   visio = 0: spatium (nulla atmosphaera). >0: radius blur (px).
 *
 * Scintillatio:
 *   Variatio temporalis intensitatis stellae per turbulentem
 *   atmosphaeram. Theoria Tatarskii (1961, "Wave propagation
 *   in a turbulent medium"): σ²_I ∝ C_n² · sec(z)^{11/6}
 *   ubi C_n² = constans structurae indicis refractionis et
 *   z = distantia zenithalis. Dravins+ (1997, 1998, "Atmospheric
 *   Intensity Scintillation of Stars" I-III) mensuraverunt
 *   σ_I/I ~ 0.05-0.15 pro stellis lucidis in situ typico.
 *   Planetae minus scintillant quia discus angularis resolutus
 *   mediat fluctuationes (Young 1967). scintillatio = 0: nulla.
 *   >0: amplitudo variationis lucis.
 *
 * Caeli lumen (pollutio luminosa):
 *   Lumen artificiale dispersum in atmosphaera per moleculas
 *   (Rayleigh, ∝ λ^{-4}) et aerosoles (Mie, debiliter λ-dependens)
 *   caelum illuminat. Garstang (1986, 1989, 1991, "Model for
 *   artificial night-sky illumination") primus modellum quantitativum
 *   dedit. Scala Bortle (2001, Sky & Telescope) 9 gradus definit:
 *   Bortle 1 (0.2 mag/arcsec², excellens) ad Bortle 9 (>19 mag/arcsec²,
 *   centrum urbanum). Cinzano+ (2001, "The first world atlas of the
 *   artificial night sky brightness") demonstraverunt >60% Europaeorum
 *   sub caelo Bortle 5+. Spectro NaD (589 nm) dominante in illuminatione
 *   sodii, nunc LED latum spectrum emittens. caeli_lumen = 0: nullum.
 *   >0: intensitas glow additivi.
 *
 * Refractio (dislocatio spatiosa atmosphaerica):
 *   Fluctuationes indicis refractionis δn(r,t) in cellulis
 *   turbulentis causant deflectionem angularem localem:
 *   α(r) = ∫ ∇⊥ δn(r,z) dz (Roddier 1981, eq. 2.14).
 *   In "short exposure" (~10 ms), stellae non diffunduntur
 *   sed moventur — "image wander" vel "tip-tilt".
 *   Variatio positionis: σ_θ ≈ 0.42 λ/r_0 (Fried 1966).
 *   Pro r_0 = 10 cm et λ = 500 nm: σ_θ ≈ 2.1 arcsec.
 *   In "long exposure", multae dislocationes integrantur in
 *   blur (visio). Sed in imagine instantanea, effectus est
 *   dislocatio spatiosa localis per-pixel — distinctum a blur
 *   (visio) et variatio intensitatis (scintillatio).
 *   Spectrum spatiatum Kolmogorov: Φ_n(κ) = 0.033 C_n² κ^{-11/3}
 *   dat structuram ad scalas 1 cm — 10 m (inner/outer scale,
 *   Tatarskii 1961; Hill 1978). hic simplificamus ad rumorem
 *   pseudo-aleatorium cum coherentia spatiali.
 *   refractio = 0: nulla. >0: amplitudo dislocationis (pixeles).
 *
 * Florescentia (bloom):
 *   Dispersio lucis intra optica (scattering in lentibus, reflexiones
 *   internae) causat halo diffusum circa fontes lucidos. In CCD,
 *   saturatio pixelorum causat "blooming" — cargas in columnas
 *   adjacentes effluere (Janesick 2001, "Scientific Charge-Coupled
 *   Devices"). In processione imaginum, bloom simulatur per
 *   convolutionem selectivam regionum lucidarum cum nucleo Gaussiano
 *   (Reinhard+ 2010, "High Dynamic Range Imaging"). Parametrus
 *   dat radium convolutionis. florescentia = 0: nulla. >0: radius.
 *
 * Acuitas (nitiditas):
 *   Acuificatio per "unsharp masking" — technica photographica
 *   ab Astronomis Regiis (Royal Observatory, 1930s) inventa et
 *   formaliter descripta a Malin (1977, "Unsharp masking" in
 *   AAS Photo-Bulletin). Algorithmus: I_acuta = I + α(I - I_blur)
 *   ubi α = factor acuificationis. Aequivalenter, haec est
 *   convolutio cum nucleo Laplaciano acuificante (Gonzalez & Woods
 *   2018, "Digital Image Processing", 4th ed., §3.6).
 *   acuitas > 0: acuificat. < 0: lenificat. 0: neutra.
 *
 * Vignetta:
 *   Obscuratio marginalis in systematis opticis. Tres causae
 *   (Ray 2002, "Applied Photographic Optics", 3rd ed.):
 *   (a) vignettatio mechanica — obstructio per tubum vel
 *       lentem anteriorem ad angulos magnos;
 *   (b) vignettatio optica — area pupillae effectivae
 *       decrescit ut cos⁴(θ) (lex cos-quarta, Slater 1959);
 *   (c) vignettatio naturalis — illuminatio ∝ cos⁴(θ) a
 *       proiectione areae aperturae et anguli incidentiae.
 *   In telescopiis astronomicis, "flat fielding" (divisio per
 *   imaginem uniformem) corrigit — sed hic effectum aestheticum
 *   servamus. vignetta = 0: nulla. >0: intensitas obscurationis.
 *
 * Distorsio (deformatio lentis):
 *   Aberratio geometrica tertii ordinis (Seidel 1856, "Zur Theorie
 *   der astronomischen Strahlenbrechung"). Distorsio barilis:
 *   r' = r(1 + k₁r² + k₂r⁴ + ...) ubi k₁ > 0 (Brown 1966,
 *   "Decentering distortion of lenses"). In oculis navium
 *   spatialium, distorsio ex fenestra curva (vitrum pressurizatum)
 *   oritur — analogum ad fenestras aeroplani ubi vitrum acrylicum
 *   sub pressione differentiali ~8 psi deformatur. Studium Lewis
 *   & Stenfors (1998, "Aircraft transparency optical quality:
 *   new methods of measurement") demonstravit deviationem opticam
 *   1-4 mrad in fenestris aeroplanicis, maxime ad margines.
 *   Boeing 787 fenestrae electrochromicae (Shannon+ 2007)
 *   addunt distorsionem chromaticam propter stratum liquidum
 *   crystallinum. In fenestris spatialibus (ISS Cupola, BK7
 *   vitrum 25 mm, Pettit 2012), distorsio barilis ~ 0.5-2%
 *   ad margines observata. distorsio > 0: barilis. < 0: pulvinar.
 *
 * Fenestra (lens toroidalis):
 *   Distorsio ex topologia tori plani. Quadratum cum marginibus
 *   oppositis identificatis duo puncta singularia habet:
 *   (1) centrum — punctum maximae regularitatis, lens convexa
 *       levis (inflatio) applicatur;
 *   (2) anguli — omnes quattuor idem punctum in toro, ubi
 *       curvatura in immersione Nash-Kuiper concentratur.
 *   In immersione C1 (Borrelli+ 2012, "Flat tori in three-
 *   dimensional space and convex integration"), corrugationes
 *   altae frequentiae curvatura localem enormiter augent circa
 *   puncta ubi directiones corrugationum concurrunt.
 *   Effectus opticus: lux per superficiem corrugatum transiens
 *   refringitur quasi per vitrum inhomogeneum — analogum ad
 *   distorsionem in fenestris aeroplanicis ubi vitrum acrylicum
 *   sub pressione differentiali curvatur (Lewis & Stenfors 1998,
 *   "Aircraft transparency optical quality", deviatio 1-4 mrad
 *   maxime ad margines). In ISS Cupola (BK7, 25 mm, Pettit 2012),
 *   distorsio ~0.5-2% ad margines observata. Boeing 787 fenestrae
 *   electrochromicae (Shannon+ 2007) addunt distorsionem propter
 *   stratum liquidum crystallinum.
 *   fenestra = 0: nulla. >0: fortitudo effectus.
 */
typedef struct {
    int    spiculae;         /* numerus spicularum diffractionis (0 = nullae) */
    double spiculae_long;    /* longitudo spicularum (in pixelis) */
    double spiculae_ang;     /* angulus rotationis spicularum (radianes) */
    double halo_radius;      /* radius hali (bloom) */
    double halo_vis;         /* intensitas hali */
    double amplificatio;     /* zoom: 1.0 = normalis, >1 = propinquior */
    double saturatio;        /* color boost: 1.0 = naturalis, 2.0 = vivida */
    double aberratio;        /* aberratio chromatica (pixeles) */

    /* atmosphaera */
    double visio;            /* seeing blur radius (0 = spatium) */
    double scintillatio;     /* amplitudo scintillationis (0 = nulla) */
    double caeli_lumen;      /* pollutio luminosa additiva (0 = nulla) */
    double refractio;        /* dislocatio spatiosa localis (0 = nulla) */

    /* effectus creativi */
    double florescentia;     /* bloom radius (0 = nulla) */
    double acuitas;          /* acuificatio: >0 acuit, <0 lenit, 0 neutra */

    /* deformatio et maschera */
    double vignetta;         /* obscuratio marginalis cos⁴ (0 = nulla) */
    double distorsio;        /* lens gravitationalis / barilis toroidale (0 = nulla) */
    double fenestra;         /* maschera superelliptica fractio (0 = nulla) */
} astra_instrumentum_t;

/*
 * Morphologia galaxiae — classificatio Hubble (1926).
 * valores in campo "phase" sideris cum genus == SIDUS_GALAXIA.
 */
typedef enum {
    GALAXIA_ELLIPTICA,          /* E0-E7: spheroidalis */
    GALAXIA_SPIRALIS,           /* Sa-Sd: brachia spiralia */
    GALAXIA_SPIRALIS_BARRATA,   /* SBa-SBd: cum barra centrali */
    GALAXIA_LENTICULARIS,       /* S0: discus sine brachiis */
    GALAXIA_IRREGULARIS,        /* Irr: asymmetrica */
    GALAXIA_NUMERUS
} galaxia_morphologia_t;

/* proprietates sideris.
 * campi "phase" et "angulus_phase" polysemici sunt:
 *   SIDUS_PLANETA:  phase = illuminatio (0=plenus, 1=novus),
 *                   angulus_phase = angulus illuminationis.
 *   SIDUS_GALAXIA:  phase = morphologia (galaxia_morphologia_t),
 *                   angulus_phase = angulus positionis in caelo. */
typedef struct {
    astra_genus_t genus;
    double        magnitudo;     /* magnitudo apparens (0=lucidissimum, 6=vix visibile) */
    double        temperatura;   /* temperatura coloris (Kelvin), 2000-40000 */
    double        phase;         /* phase planetae vel morphologia galaxiae */
    double        angulus_phase; /* angulus illuminationis vel positionis */
} astra_sidus_t;

/* campus stellarum (bitmap toroidalis) */
typedef struct {
    unsigned char *pixels;   /* RGB, latitudo * altitudo * 3 */
    int            latitudo;
    int            altitudo;
} astra_campus_t;

/* ================================================================
 * campus stellarum
 *
 * Fundamenta cosmologica:
 *
 *   Distributio stellarum in caelo non uniformis est sed structuram
 *   hierarchicam reflectit a primordiis cosmicis derivatam:
 *
 *   Fluctuationes quanticae in campo inflatonis (Guth 1981, Linde 1982)
 *   per inflationem cosmicam amplificatae sunt in perturbationes
 *   densitatis macroscopicas. Spectrum potentiae P(k) ∝ k^(n_s-1)
 *   ubi n_s ≈ 0.965 (Planck 2018) — fere invariantem sed non exacte,
 *   indicans campum inflaton non perfecte liberum fuisse.
 *
 *   Hae perturbationes per instabilitatem gravitationalem Jeans
 *   (λ_J = c_s √(π/Gρ)) in structuras hierarchicas collapserunt:
 *   filamenta → nodi → halos → galaxiae → stellae.
 *   Hoc est "cosmic web" (Bond, Kofman & Pogosyan 1996).
 *
 *   In theoria chordarum (string landscape, Bousso & Polchinski 2000),
 *   constantiae physicae (α_EM, m_e/m_p, Λ) ex selectione vacui
 *   in ~10^500 possibilibus dependent. Principium anthropicum
 *   (Weinberg 1987) limitat Λ ad valorem observatum ~10^(-122) M_Pl^4
 *   — si Λ maior esset, nulla structura formaretur.
 *
 *   Nucleosynthesis primordialis (Alpher, Bethe, Gamow 1948):
 *   ratio H/He ~75/25 per massam, determinata per η = n_B/n_γ ≈ 6×10^(-10).
 *   Haec compositio primaeva stellarum spectra et evolutionem regit:
 *   - Populatio III (Z=0): primae stellae, massivissimae, calidissimae
 *   - Populatio II (Z bassa): halo galacticum, globulares
 *   - Populatio I (Z alta): discus, brachia spiralia, Sol
 *
 *   Fascia galatctica (Via Lactea) structuram disci reflectit:
 *   scala altitudinis h_z ≈ 300 pc (discus tenuis), profilo sech²(z/h_z).
 *   Nucleus lucidior quia densitas stellaris ∝ exp(-R/R_d) ubi R_d ≈ 2.6 kpc.
 *   Great Rift ex nubibus molecularibus (H₂, CO, pulvis silicatum)
 *   cum extinctione A_V ≈ 1 mag/kpc in plano.
 *
 *   Topologia toroidalis campi stellarum non est mera commoditas
 *   computationis: cosmologia inflationaria universum spatialiter
 *   planum praedicit (Ω_k = 0.001 ± 0.002, Planck 2018).
 *   Universum finitum cum topologia toroidali T³ cum omnibus
 *   observationibus concordat et suppressionem anomalam potentiae
 *   CMB in scalis magnis naturaliter explicat (Luminet et al. 2003).
 * ================================================================ */

/* parametri generationis campi stellarum */
typedef struct {
    int    numerus_stellarum;     /* stellae totales tentandae */
    double densitas_galaxiae;     /* 0..1: intensitas concentrationis in fascia */
    double inclinatio_galaxiae;   /* angulus fasciae (radianes) */
    double latitudo_galaxiae;     /* latitudo fasciae (fractio altitudinis) */
    unsigned int semen;           /* semen aleatorium */

    /* via lactea — glow diffusum */
    double galaxia_glow;          /* 0..1: intensitas glow (0 = nullum) */
    double galaxia_rift;          /* 0..1: intensitas fasciae pulveris */
    int    galaxia_nebulae;       /* numerus nebulosarum emissionis */

    /* limites per genus (0 = illimitatum) */
    int    max_supergigantes;
    int    max_gigantes;
    int    max_exotica;           /* crystallinum + magnetar + neutronium */

    /* planetae */
    int    numerus_planetarum;
    double planetae_temp_min;     /* temperatura minima planetarum */
    double planetae_temp_max;

    /* galaxiae distantes */
    int    numerus_galaxiarum;    /* galaxiae totales tentandae */
    int    max_galaxiae;          /* maximum galaxiarum positarum (0 = illimitatum) */
} astra_parametri_t;

/* campum creare et destruere */
astra_campus_t *astra_campum_creare(int latitudo, int altitudo);
void astra_campum_destruere(astra_campus_t *c);

/* campum stellarum generare — instrumentum separatum a campo */
void astra_campum_generare(astra_campus_t *c, const astra_parametri_t *p,
                           const astra_instrumentum_t *instrumentum);

/*
 * astra_ex_isonl_reddere — campum stellarum ex ISONL et instrumento reddit.
 * Legit ISONL (stellae fixae ex caele), applicat instrumentum opticum,
 * reddit campum paratum. Vocans campum per astra_campum_destruere liberet.
 * Reddit NULL si error.
 */
astra_campus_t *astra_ex_isonl_reddere(const char *via_isonl,
                                        const char *via_instrumentum);

/*
 * astra_tabulam_dynamicam — applicat effectus dynamicos ad campum.
 *
 * Sumit campum stellarum staticum (basis) et reddit novum campum
 * cum effectibus temporalibus applicatis pro tabula f:
 *   - scintillatio: semen mutatur per tabulam → stellae micant
 *   - refractio: semen mutatur → stellae tremunt (image wander)
 *
 * basis non mutatur. vocans reddit campum per astra_campum_destruere.
 * instrumentum dat parametros effectuum (scintillatio, refractio, etc.).
 * scala: factor reductionis (1 = plena, 2 = dimidia, etc.).
 * dx, dy: translatio toroidalis in pixelis pro hac tabula.
 */
astra_campus_t *astra_tabulam_dynamicam(const astra_campus_t *basis,
                                         const astra_instrumentum_t *inst,
                                         int tabula,
                                         int scala,
                                         double dx, double dy);

/* ================================================================
 * sidus reddere
 *
 * Reddit sidus in fenestram 64x64.
 * fenestra: RGBA buffer, ASTRA_FENESTRA * ASTRA_FENESTRA * 4 bytes.
 * alpha indicat quantum pixel a sidere afficitur.
 * ================================================================ */

void astra_sidus_reddere(unsigned char *fenestra,
                         const astra_sidus_t *sidus,
                         const astra_instrumentum_t *instrumentum);

/* ================================================================
 * auxiliaria
 * ================================================================ */

/* colorem ex temperatura (Kelvin) per Planck approximare */
astra_color_t astra_temperatura_ad_colorem(double kelvin);

/* pixel in campum toroidalem scribere (coordinatae modulantur) */
void astra_pixel_scribere(astra_campus_t *c, int x, int y,
                          unsigned char r, unsigned char g, unsigned char b);

/*
 * astra_regio_vacua — inspicit an regio circa (cx,cy) satis obscura sit.
 * radius: distantia minimalis in pixelis.
 * reddit 1 si regio vacua, 0 si iam stella praesens.
 */
int astra_regio_vacua(const astra_campus_t *c, int cx, int cy, int radius);

/* sidus in campum inserere ad positionem (x,y) — toroidale */
void astra_sidus_in_campum(astra_campus_t *c, int cx, int cy,
                           const unsigned char *fenestra);

#endif /* ASTRA_H */
