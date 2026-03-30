# sqrt

**Isometric embeddings of the flat torus in three-dimensional Euclidean space, procedural starfield and galaxy generation, and a real-time interactive viewer — all in zero-dependency C99.**

## What This Is

The flat torus — a square with opposite edges identified — cannot be embedded in R³ without distorting distances. Or so everyone believed, until Nash and Kuiper proved otherwise in the 1950s, and the Hévéa team finally visualized the result in 2012. sqrt computes and renders these extraordinary surfaces: smooth, corrugated objects that preserve the flat metric of the torus through layers of increasingly fine wrinkles.

This is not an approximation or an artistic interpretation. This is the real thing — multi-scale corrugations with geometrically increasing frequencies and decreasing amplitudes, alternating between toroidal and poloidal directions, converging on a C1 isometric embedding. The mathematics is exact. The rendering is publication-quality.

## Embedding Methods

Four distinct methods for constructing the embedded surface:

| Method | Description |
|---|---|
| **Corrugata** | Multi-scale corrugations with five frequency layers |
| **Iterata** | Iterative convergence toward isometry |
| **Spiralis** | Spiral corrugation pattern |
| **Normalis** | Normal-direction perturbation |

## Rendering

Phong illumination with three directional lights, Fresnel effects, gamma correction, and triangle rasterization with barycentric coordinates and z-buffering. Surface normals computed via central finite differences. Output to PPM.

## Starfields

A procedural starfield generator creates toroidal skies populated with physically-typed stellar objects — white dwarfs, main sequence stars, red giants, supergiants, neutron stars, crystalline stars, magnetars, distant galaxies, and planets. Star properties are generated from ISON configuration files and emitted as ISONL; a separate renderer applies optical instrument models and produces the final image.

Distant galaxies are rendered as extended objects with five Hubble (1926) morphological types: elliptical (de Vaucouleurs 1948 R¹ᐟ⁴ profile), spiral (Lin & Shu 1964 density wave theory, logarithmic arms per Ringermacher & Mead 2009), barred spiral (Athanassoula 2003 orbital resonance model), lenticular, and irregular (Hunter & Elmegreen 2004 stochastic star formation). Morphological fractions follow Galaxy Zoo (Lintott+ 2008). Spiral arm pitch angles follow Kennicutt (1981), disk profiles follow Freeman (1970), and edge-on dust lanes follow van der Kruit & Searle (1981). Inclination, position angle, and color vary per galaxy. The magnitude distribution follows the Schechter (1976) luminosity function — most galaxies are small and faint, with only occasional bright ones like Andromeda. Flat rotation curves (Rubin & Ford 1970) imply the dark matter halos that make all of this possible.

```bash
./caele campi/terra.ison > caelae/terra.isonl
./redde caelae/terra.isonl instrumenta/oculus.ison imago.ppm
```

### Optical Instruments

Seven instrument models with ten post-processing effects applied in physically-motivated order:

| Instrument | Character |
|---|---|
| `hst` | Hubble Space Telescope — 4 diffraction spikes, enhanced saturation |
| `jwst` | James Webb — 6 spikes from hexagonal segments, deep color boost |
| `oculus` | Ideal refractor — no spikes, minimal halo, clean image |
| `refractor` | Classical refractor — chromatic aberration (Cauchy dispersion) |
| `terrestre` | Ground observatory — atmospheric seeing (Kolmogorov 1941, Fried 1966), scintillation (Tatarskii 1961, Dravins+ 1997), light pollution (Garstang 1986, Bortle 2001) |
| `navis` | Spacecraft window — vignetting (cos⁴ law, Slater 1959), barrel distortion (Brown 1966), toroidal lens (Borrelli+ 2012 Nash-Kuiper curvature concentration, window distortion per Lewis & Stenfors 1998) |
| `astrophoto` | Astrophotography — bloom (Janesick 2001 CCD saturation), unsharp masking (Malin 1977), aggressive saturation |

Post-processing pipeline: atmospheric blur → scintillation → atmospheric refraction (Roddier 1981, Fried 1966 local displacement with Kolmogorov spatial coherence) → sky glow → bloom → sharpening → chromatic aberration → gravitational lensing / barrel distortion (Bartelmann & Schneider 2001) → color saturation → vignetting → toroidal lens (Borrelli+ 2012). All coordinate transforms wrap toroidally. Dynamic effects (scintillation, refraction) can be animated via `astra_animare`, producing seamlessly looping GIFs with toroidal wrap-around translation (`-wx`/`-wy`).

## Programs

| Program | What It Does |
|---|---|
| `torus_planus` | Renders a single 1920×1080 image of the corrugated torus |
| `torus_animare` | Renders a camera-rotation animation sequence (640×480 PPM series) |
| `torus_specta` | Interactive phantasma viewer with real-time rotation, multiple themes, and MP4 recording |
| `caele` | Generates starfield data from ISON configuration |
| `redde` | Renders starfield ISONL through an optical instrument model to PPM |
| `astra_animare` | Animated starfield with dynamic atmospheric effects to MP4/GIF |

## Building

```bash
make
```

No external dependencies. C99 and libm. The interactive viewer (`torus_specta`) and starfield animator (`astra_animare`) additionally require phantasma.

## Usage

```bash
./torus_planus
./torus_animare [num_frames]
./torus_specta
```

### Interactive Controls

| Key | Action |
|---|---|
| Arrow keys | Rotate camera azimuth and elevation |
| W / S | Zoom in / out |
| X / Y / Z | Select auto-rotation axis |
| Space | Pause / resume auto-rotation |
| +/- | Adjust rotation speed |
| Tab | Next color theme |
| 1/2 | Decrease / increase major radius |
| 3/4 | Decrease / increase minor radius |
| Mouse wheel | Zoom |
| C | Start / stop recording (MP4) |
| L | Record one full rotation loop (MP4) |
| R | Reset view |
| Q / Escape | Quit |

## License

Free. Public domain. Use however you like.
