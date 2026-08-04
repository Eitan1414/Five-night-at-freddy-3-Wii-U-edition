# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port project targeting **Aroma** and the `.wuhb` format.

## Current status

The port now has a modular foundation and its first PSX visuals running on Wii U:

- native C application built with devkitPro `wut`;
- portable GamePad input API;
- shared graphics API targeting the TV, GamePad or both;
- separate game-state, platform, sprite and texture modules;
- converter for indexed PlayStation `.TIM` images;
- row-compressed indexed textures with nearest-neighbour scaling;
- original PSX warning graphic on TV and GamePad;
- original PSX Springtrap image on the title screen;
- interactive **New Game**, **Load Game** and **Extras** menu;
- `New Game` opens the converted PSX office panorama;
- left/right scrolls through the office image;
- X or Y opens the separate GamePad camera prototype;
- automatic `.elf`, `.rpx` and `.wuhb` builds with GitHub Actions.

The game logic is still an early systems test. Audio, saves, real camera feeds, Springtrap AI, phantom encounters and night progression have not been ported yet.

## Project layout

```text
include/
├── assets/
├── game/
├── platform/
└── renderer/
source/
├── game/
├── platform/
├── renderer/
├── main.c
├── warning_texture.c
├── menu_springtrap_texture.c
└── office_texture.c
tools/
└── convert_tim.py
```

Game code no longer calls `VPADRead` or `OSScreen` directly. `main.c` currently composes the converted PSX textures over the portable game states while the renderer migration is still in progress.

## Texture pipeline

The temporary software texture path uses:

- Wii U `RGBX8` palettes;
- row-by-row run-length compression;
- a transparent palette index;
- nearest-neighbour scaling;
- the same API for TV, GamePad or both.

Convert a supported 4-bit or 8-bit indexed PlayStation TIM with:

```sh
python3 tools/convert_tim.py input.tim source/generated_texture.c include/assets/generated_texture.h \
  --symbol gGeneratedTexture --sample-step 2
```

The warning uses a small converted texture. Springtrap and the office currently use reduced-resolution RLE versions so they remain practical with the early `OSScreen` backend. A later GX2 renderer will allow higher-resolution assets and faster full-screen updates.

## Build locally

Install the current Wii U development packages through devkitPro, then run:

```sh
make
```

Expected outputs:

```text
fnaf3-wiiu.elf
fnaf3-wiiu.rpx
fnaf3-wiiu.wuhb
```

## Test on Wii U

Copy the generated file to:

```text
sd:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

Launch it from the Aroma Wii U Menu.

## Current controls

### Warning

- **A** or **+**: skip.

### Title screen

- **D-Pad Up/Down** or left-stick emulation: change selection;
- **A** or **+**: confirm.

### Office test

- **Left/Right**: scroll through the office panorama;
- **X** or **Y**: open or close the GamePad camera panel;
- **B**: return to the title screen.

### Load Game / Extras

- **B**: return to the title screen.

The HOME button continues to use the normal Wii U system flow.

## Porting policy

The supplied PSX fan-project archive does not contain a clear source-code or asset licence. Converted game textures are therefore kept in this private development repository for port testing and should not be redistributed publicly without confirming permission. The port architecture and conversion tools remain separated from imported game assets.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the migration plan.
