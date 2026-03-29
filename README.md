# sqrt — Flat Torus in Three Dimensions

A C99 renderer that produces images of a flat torus isometrically immersed in Euclidean 3-space via Nash-Kuiper corrugation. No dependencies beyond the C standard library and `libm`. Outputs raw PPM.

## What This Is

A flat torus is a torus with zero Gaussian curvature everywhere — the geometry of a square with opposite edges identified. The standard donut-shaped torus in R3 is not flat; it has regions of positive and negative curvature. For decades it was an open visual question what a truly flat torus embedded in 3-space would look like.

The Nash-Kuiper theorem (1954-1955) guarantees that such an embedding exists as a C1 surface — continuous with continuous first derivatives, but not twice differentiable. The construction proceeds by adding successive layers of corrugations at increasing frequencies and decreasing amplitudes, each layer reducing the metric error introduced by the previous embedding. In the limit, the surface converges to an isometric immersion. The result is a fractal-like object with structure at every scale.

In 2012, a team at the Institut Camille Jordan and Laboratoire Jean Kuntzmann produced the first computer visualizations of this object, which they called the Hévéa torus. sqrt implements the same corrugation scheme: five layers of sinusoidal perturbations alternating between the toroidal and poloidal directions, with frequencies growing geometrically (ratio ~2.8) and amplitudes decaying correspondingly.

## What It Produces

`torus_planus` renders a single high-resolution image (1920×1080) of the corrugated torus under Phong illumination with three directional lights, Fresnel rim effects, and gamma correction. The surface has the characteristic wrinkled appearance of the Hévéa torus — smooth at a distance, increasingly corrugated as you look closer.

`torus_animare` renders a sequence of frames with the camera orbiting the torus, suitable for assembling into a video. Default is 72 frames at 640×480.

## Build

```bash
make
```

Two binaries: `torus_planus` and `torus_animare`. No dependencies. No build system beyond make. Compiles in under a second.

## Usage

```bash
./torus_planus
./torus_animare [num_frames]
```

Output is raw PPM. Convert with ImageMagick, ffmpeg, or anything that reads PPM:

```bash
convert torus_planus.ppm torus_planus.png
ffmpeg -i tabulae/imago_%04d.ppm -c:v libx264 -pix_fmt yuv420p torus.mp4
```

## The Rendering

The surface is tessellated into a grid of 800×400 (static) or 600×300 (animation) quads, each split into two triangles. Rendering uses software rasterization with barycentric coordinate interpolation, a z-buffer for depth, and per-pixel Phong shading with interpolated normals. Surface normals are computed numerically via central finite differences.

The illumination model uses three directional lights with diffuse (Lambert) and specular (Blinn-Phong) components, plus a Fresnel term that brightens the surface at grazing angles. The material is gold-tinted. Background is near-black.

No GPU. No OpenGL. No graphics library. Just trigonometry, linear algebra, and a framebuffer written to disk.

## Why

Because the Nash-Kuiper theorem is one of the most counterintuitive results in differential geometry, and seeing its consequences rendered pixel by pixel from first principles — in a few hundred lines of C with no dependencies — is the kind of thing that justifies writing software.

## License

Free. Use however you like.
