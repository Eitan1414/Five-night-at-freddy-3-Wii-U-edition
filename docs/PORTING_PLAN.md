# Wii U porting plan

## Source audit

The PSX project is the appropriate technical reference because it is written in C and already contains the complete game flow. The audit found:

- `fnaf3.c`: about 905 lines;
- most of the implementation is placed in large headers under `objects/`;
- direct PlayStation APIs for graphics, controller input, CD access, memory cards and SPU audio;
- PSX-native `.TIM`, `.VAG` and XA/CD resources;
- no clear source-code or asset licence in the supplied archive.

The code cannot simply be recompiled with `wut`. Its game logic must be separated from the PSX hardware layer.

## Phase 0 — Wii U bootstrap

- [x] Create a native `wut` project.
- [x] Produce `.elf`, `.rpx` and `.wuhb` outputs.
- [x] Initialize and shut down cleanly through `WHBProc`.
- [x] Display debug text on TV and GamePad.
- [x] Read GamePad buttons with VPAD.
- [x] Add a GitHub Actions build.

## Phase 1 — Platform abstraction

Create portable interfaces instead of calling console APIs from game logic:

```text
platform/input.h
platform/graphics.h
platform/audio.h
platform/storage.h
platform/time.h
```

Initial replacements:

| PSX dependency | Wii U replacement |
| --- | --- |
| `PadRead`, `PAD*` | `VPADRead`, later KPAD support |
| `FntPrint` | Wii U debug/UI text renderer |
| `Gs*`, `DrawOTag`, `LoadImage` | GX2 or SDL2-based renderer |
| `Spu*` | AX/AX2 or SDL2_mixer-compatible audio layer |
| `Cd*` | normal files from bundled content or SD |
| memory-card functions | Wii U save directory / SD fallback |

## Phase 2 — Minimal playable loop

- title/menu state;
- office background;
- horizontal office movement;
- open/close camera panel;
- one selectable camera;
- stable 60 Hz update loop independent of rendering.

## Phase 3 — Game systems

- camera map and vents;
- maintenance panel;
- Springtrap AI;
- phantom encounters and jumpscares;
- time progression and night completion;
- save/load and unlocks.

## Phase 4 — Wii U features

- TV: office and main view;
- GamePad: camera system and touch controls;
- GamePad speaker for radio/static effects;
- optional Wii U Pro Controller support;
- proper icon and TV/GamePad boot splashes;
- release packaging.

## Immediate next task

Define the portable input API and translate the PSX control conditions from `controllerinput()` without importing rendering or audio code.
