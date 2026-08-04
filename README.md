# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port project targeting **Aroma** and the `.wuhb` format.

## Current status

The project has reached its first interactive graphical milestone:

- native C application built with devkitPro `wut`;
- clean Wii U process startup and shutdown;
- software-rendered output on both the TV and Wii U GamePad;
- PSX-inspired warning screen with a short fade-out;
- dark green title screen with a procedural Springtrap silhouette;
- CRT scanlines and intermittent glitch bars;
- interactive **New Game**, **Load Game** and **Extras** choices;
- automatic `.elf`, `.rpx` and `.wuhb` builds with GitHub Actions.

The three menu entries currently lead to temporary status screens. The original gameplay, audio, save system and game assets have not been ported yet.

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
- **A** or **+**: confirm the selected entry.

### Temporary status screens

- **B**: return to the title screen.

The HOME button continues to use the normal Wii U system flow.

## Rendering note

This milestone intentionally uses `OSScreen` and procedurally drawn shapes instead of imported PSX images. That keeps the first graphic build self-contained and makes it easier to validate stability before introducing converted textures and a faster GX2 renderer.

## Porting policy

The PSX archive supplied for analysis does not contain a clear open-source licence for its code or assets. For that reason, the original files are not copied into this repository yet. We will first keep the Wii U platform code clean, then import or reimplement only material that can legally and technically be used.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the migration plan.
