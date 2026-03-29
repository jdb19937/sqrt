# sqrt

**Isometric embeddings of the flat torus in three-dimensional Euclidean space, procedural starfield generation, and a real-time interactive viewer — all in 3,300 lines of zero-dependency C99.**

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

A procedural starfield generator creates toroidal skies populated with physically-typed stellar objects — white dwarfs, main sequence stars, red giants, supergiants, neutron stars, crystalline stars, magnetars, and planets. Star properties are generated from JSON configuration files and emitted as JSONL; a separate renderer applies optical instrument models and produces the final image.

```bash
./caele caelae/terra.json > caelae/terra.jsonl
./redde caelae/terra.jsonl caelae/instrumentum.json output.ppm
```

## Programs

| Program | What It Does |
|---|---|
| `torus_planus` | Renders a single 1920×1080 image of the corrugated torus |
| `torus_animare` | Renders a camera-rotation animation sequence (640×480 PPM series) |
| `torus_specta` | Interactive SDL2 viewer with real-time rotation, multiple themes, and MP4 recording |
| `caele` | Generates starfield data from JSON configuration |
| `redde` | Renders starfield JSONL through an optical instrument model to PPM |

## Building

```bash
make
```

No external dependencies. C99 and libm. The interactive viewer (`torus_specta`) additionally requires SDL2.

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

Free. Public domain.
