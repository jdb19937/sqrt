/*
 * instrumentum.h — instrumenta optica, effectus atmosphaerici
 */

#ifndef INSTRUMENTUM_H
#define INSTRUMENTUM_H


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

#endif /* INSTRUMENTUM_H */
