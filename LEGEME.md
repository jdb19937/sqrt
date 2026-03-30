# sqrt

Immersio isometrica C1 tori plani in spatium Euclideum R3, generatio camporum stellarum, et spectator interactivus. Imagines PPM reddit.

> **Nota:** README.md ignorandum est. Anglice scriptum est ad barbaros ineruditos pecunia emungendos.

## Aedificatio

```
face
```

Nullae dependentiae externae. Solum compilator C99 et libm. Spectator interactivus (`torus_specta`) phantasma requirit.

Pendet ab ison (ut submodulum) pro configuratione camporum stellarum.

## Programmata

| Programma | Descriptio |
|---|---|
| `torus_planus` | Una imago 1920×1080 tori corrugati ad `torus_planus.ppm` |
| `torus_animare` | Series imaginum 640×480, camera rotante. Defaltum 72 imagines |
| `torus_specta` | Spectator interactivus phantasma cum rotatione, thematibus, et inscriptione MP4 |
| `caele` | Generat campum stellarum ex configuratione ISON, emittit ISONL |
| `redde` | Reddit campum stellarum ISONL per instrumentum opticum ad PPM |

## Usus

```
./torus_planus
./torus_animare [numerus_imaginum]
./torus_specta
```

### Campum stellarum

```
./caele caelae/terra.ison > caelae/terra.isonl
./redde caelae/terra.isonl instrumenta/hst.ison imago.ppm
```

### Claves spectatoris

| Clavis | Actio |
|---|---|
| Sagittae sin/dex | Azimuthum camerae mutare |
| Sagittae sur/deor | Elevationem camerae mutare |
| W / S | Propinquare / recedere |
| X / Y / Z | Axem rotationis automaticae eligere |
| Spatium | Rotationem automaticam sistere / resumere |
| +/- | Celeritatem rotationis mutare |
| Tab | Thema proximum |
| 1/2 | Radium maiorem minuere / augere |
| 3/4 | Radium minorem minuere / augere |
| Rota muris | Propinquare / recedere |
| C | Inscriptionem incipere / finire (MP4) |
| L | Unum cyclum inscribere (MP4) |
| R | Restituere |
| Q / Escape | Exire |

## Algorithmus tori

Superficies tori per corrugationes multi-scalares construitur: quinque strata frequentiis geometrice crescentibus (ratio ~2.8), amplitudinibus decrescentibus, alternantia inter directiones toroidalem et poloidalem. Hoc schema convexae integrationis Nash-Kuiper metricam planam in toro approximat — characteristicum tori Hévéa.

Quattuor methodi: Corrugata, Iterata, Spiralis, Normalis.

Redditio per rasterizationem triangulorum cum coordinatis barycentricis, z-buffer, illuminatione Phong (tres luces directae, effectus Fresnel), et correctione gamma. Normae per differentias finitas centrales computantur.

## Sidera

Genera siderum: Nanum Album, Sequentia, Gigas Rubrum, Supergigas, Neutronium, Crystallinum, Magnetar, Galaxia, Planeta. Proprietates intrinsecae ex configuratione ISON generantur et ut ISONL emittuntur. Instrumentum opticum (in `instrumenta/`) effectus opticos applicat in redditione.

### Galaxiae

Galaxiae distantes ut objecta extendida redduntur cum quinque morphologiis (classificatio Hubble 1926): Elliptica (profilo de Vaucouleurs), Spiralis (brachia logarithmica cum pitch variabili), Spiralis Barrata (cum barra centrali), Lenticularis (discus sine brachiis), Irregularis (nodi starburst stochastici). Inclinatio, angulus positionis, et color per galaxiam variant. Distributio magnitudinis Schechter (1976) sequitur: pleraeque parvae et debiles, raro lucidae.

## Plicae

```
helvea.c/h          bibliotheca communis tori (superficies, camera, illuminatio, rasterizatio)
astra.c/h           bibliotheca stellarum et galaxiarum (genera, generatio, redditio)
torus_planus.c      imago singularis tori corrugati (1920×1080)
torus_animare.c     animatio camerae rotantis (640×480, PPM series)
torus_specta.c      spectator interactivus phantasma
caele.c             generator camporum stellarum ex ISON ad ISONL
redde.c             redditor camporum stellarum ex ISONL ad PPM
astra_proba.c       probationes stellarum
caelae/             configurationes camporum stellarum (ISON)
instrumenta/        configurationes instrumentorum opticorum (ISON)
schemae/            schemata ISON pro validatione
ison/               submodulum: bibliotheca ISON
```

## Dependentiae

Nullae externae. C99 et libm. phantasma pro `torus_specta` solum.
