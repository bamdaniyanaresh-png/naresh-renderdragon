# How the build works

A RenderDragon shader pack is a set of `*.material.bin` files in `renderer/materials`.
They are made from shader source (`.sc` files) and the game's own material definitions.

## Steps
1. `naresh.py setup` copies shaderc to `tools/` and turns the game's `*.material.bin` files into small
   `*.material.json` files in `vanilla/` (`lazurite serialize`).
2. `naresh.py pack` runs `lazurite build` on the `shaders/` project. For each material, lazurite
   - reads the vanilla definition from `vanilla/` (the merge source),
   - adds our changes (`config.json`, `material.json`, `uniforms/`),
   - compiles `vertex.sc` and `fragment.sc` for every pass and flag combination with shaderc,
   - writes `<Material>.material.bin`.
3. The tool does this once for the main pack and once for every subpack that has materials.
   A subpack is compiled with its own macro (for example `QUALITY_LOW`).
4. It writes `manifest.json` (with the `subpacks` list), copies `assets/` and zips everything to a `.mcpack`.

## Files of a material (`shaders/<Material>/`)
| File | Use |
|---|---|
| `vertex.sc`, `fragment.sc` | shader code |
| `varying.def.sc` | inputs of the vertex shader and values passed to the fragment shader |
| `config.json` | gives the game's flags and passes plain macro names (`ALPHA_TEST`, `INSTANCING`, ...) |
| `material.json`, `uniforms/` | add uniforms such as `TimeOfDay` and connect them to the game values |

`shaders/project.json` lists the materials, the platforms of each profile and where the compiler looks for
`#include` files (`lib/` and `include/`).

## Shader code
Every shader includes `<naresh.sh>`, which pulls in the files of `lib/`:

| File | Content |
|---|---|
| `settings.h` | options and the quality macros |
| `nrs_math.h` | colour space, hash and noise |
| `tone.h` | tone mapping, a mix of four curves, and the grade |
| `daylight.h` | sun and moon direction, colour of the light and of the sky light |
| `sky.h`, `clouds.h` | sky colour, sun, big moon with glow, night sky band, northern lights, clouds, fog colour |
| `stars.h` | three layers of stars for the night sky |
| `torch.h` | torch, lantern and fire light |
| `terrain.h` | world light, block shadows, fog, water, glass and shiny blocks (RenderChunk) |
| `entity.h` | light of mobs, items and weather |
| `endsky.h`, `tornado.h` | cold blue End sky and the turning storm funnel over the main island of the End |

## Checks
`python naresh.py check` finds missing files, wrong json and names in `$input` / `$output` that are not in `varying.def.sc`.
