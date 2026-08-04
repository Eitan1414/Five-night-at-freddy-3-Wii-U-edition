# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port project targeting **Aroma** and the `.wuhb` format.

## Current status

The project now has a modular Phase 1 foundation, a first playable systems test and its first converted PSX texture:

- native C application built with devkitPro `wut`;
- portable input API instead of direct controller checks in game code;
- shared graphics API targeting the TV, GamePad or both;
- separate game state, platform, sprite-renderer and texture-renderer modules;
- row-compressed indexed texture format with nearest-neighbour scaling;
- conversion tool for indexed PlayStation `.TIM` images;
- the original PSX warning graphic displayed on TV and GamePad;
- PSX-inspired title screen with interactive **New Game**, **Load Game** and **Extras** choices;
- `New Game` opens an office test with horizontal movement;
- the GamePad can display a separate camera-system prototype;
- automatic `.elf`, `.rpx` and `.wuhb` builds with GitHub Actions.

The title, office, camera interface and most sprites are still temporary procedural graphics. Original gameplay, audio, saves and AI have not been integrated yet.

## Project layout

```text
include/
├── assets/
├── game/
├── platform/
└── renderer/
source/
├── assets/
├── game/
├── platform/
├── renderer/
├── main.c
└── warning_texture.c
tools/
└── convert_tim.py
```

`main.c` initializes the platform and runs the update/render loop. Game code no longer calls `VPADRead` or `OSScreen` directly.

## Texture pipeline

The current software texture renderer uses:

- an indexed palette in Wii U `RGBX8` colours;
- row-by-row run-length compression;
- a transparent palette index;
- nearest-neighbour scaling to the logical 854×480 canvas;
- the same texture API for TV, GamePad or both.

Convert a supported 4-bit or 8-bit indexed PlayStation TIM with:

```sh
python3 tools/convert_tim.py input.tim source/generated_texture.c include/assets/generated_texture.h \
  --symbol gGeneratedTexture --sample-step 2
```

Large backgrounds will eventually move to a faster GX2-backed texture path. The current `OSScreen` renderer is intended for validation and small early assets.

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

### Warning screen

- **A** or **+**: skip the warning.

### Title screen

- **D-Pad Up/Down** or left-stick emulation: change selection;
- **A** or **+**: confirm.

### Office test

- **Left/Right**: look around the office;
- **X** or **Y**: open or close the GamePad camera panel;
- **B**: return to the title screen.

### Load Game / Extras placeholders

- **B**: return to the title screen.

The HOME button continues to use the normal Wii U system flow.

## Porting policy

The supplied PSX fan-project archive does not contain a clear source-code or asset licence. The converted warning texture is therefore included only in this private development repository for port testing and should not be redistributed in a public release without confirming permission. The port architecture and conversion tools remain separated from imported game assets so they can be replaced or packaged appropriately later.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the migration plan.
