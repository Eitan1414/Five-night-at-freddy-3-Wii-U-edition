# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port project targeting **Aroma** and the `.wuhb` format.

## Current status

The project now has a modular Phase 1 foundation and a first playable systems test:

- native C application built with devkitPro `wut`;
- portable input API instead of direct controller checks in game code;
- shared graphics API targeting the TV, GamePad or both;
- separate game state, platform and sprite-renderer modules;
- PSX-inspired warning and title screens;
- interactive **New Game**, **Load Game** and **Extras** choices;
- `New Game` opens an office test with horizontal movement;
- the GamePad can display a separate camera-system prototype;
- automatic `.elf`, `.rpx` and `.wuhb` builds with GitHub Actions.

The office, camera interface and sprites are still temporary procedural graphics. Original gameplay, audio, saves, AI and converted game assets have not been integrated yet.

## Project layout

```text
include/
├── game/
├── platform/
└── renderer/
source/
├── game/
├── platform/
├── renderer/
└── main.c
```

`main.c` now only initializes the platform and runs the update/render loop. Game code no longer calls `VPADRead` or `OSScreen` directly.

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

## Rendering note

This milestone still uses `OSScreen` and procedural graphics. The abstraction layer means it can later be replaced by a faster GX2 texture renderer without rewriting the game-state code.

## Porting policy

The PSX archive supplied for analysis does not contain a clear open-source licence for its code or assets. Original files are therefore not copied into this repository yet. Only material that can legally and technically be reused will be integrated.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the migration plan.
