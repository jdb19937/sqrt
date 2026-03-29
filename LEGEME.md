# sqrt

Immersio isometrica C1 tori plani in spatium Euclideum R3, secundum methodum corrugationis Nash-Kuiper. Imagines PPM reddit.

> **Nota:** README.md ignorandum est. Anglice scriptum est ad barbaros ineruditos pecunia emungendos.

## Aedificatio

```
make
```

Nullae dependentiae externae. Solum compilator C99 et libm.

## Usus

```
./torus_planus
./torus_animare [numerus_imaginum]
```

`torus_planus` unam imaginem 1920×1080 ad `torus_planus.ppm` scribit. `torus_animare` seriem imaginum 640×480 in `tabulae/` reddit, camera circa torum rotante. Defaltum 72 imagines.

## Algorithmus

Superficies tori per corrugationes multi-scalares construitur: quinque strata frequentiis geometrice crescentibus (ratio ~2.8), amplitudinibus decrescentibus, alternantia inter directiones toroidalem et poloidalem. Hoc schema convexae integrationis Nash-Kuiper metricam planam in toro approximat — characteristicum tori Hévéa.

Redditio per rasterizationem triangulorum cum coordinatis barycentricis, z-buffer, illuminatione Phong (tres luces directae, effectus Fresnel), et correctione gamma. Normae per differentias finitas centrales computantur.

## Plicae

```
torus_planus.c      imago singularis tori corrugati (1920×1080)
torus_animare.c     animatio camerae rotantis (640×480, PPM series)
Faceplica            aedificatio
```

## Dependentiae

Nullae externae. C99 et libm.
