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
- [x] Display output on TV and GamePad.
- [x] Read GamePad buttons with VPAD.
- [x] Add a GitHub Actions build.

## Phase 1 — Platform abstraction

- [x] Add a portable `platform/input.h` API.
- [x] Move all `VPADRead` logic into `input_wiiu.c`.
- [x] Add a graphics API that can target TV, GamePad or both.
- [x] Move `OSScreen` allocation and drawing out of game code.
- [x] Add a frame-clock wrapper.
- [x] Add an initial sprite-mask renderer.
- [x] Add a row-compressed indexed texture renderer.
- [x] Add a converter for indexed PlayStation `.TIM` files.
- [x] Display converted PSX textures on TV and GamePad.
- [x] Render the real PSX Springtrap title image.
- [x] Render and horizontally scroll the real PSX office panorama.
- [x] Render three selectable PSX camera feeds on the GamePad.
- [ ] Add portable audio API.
- [ ] Add portable storage/save API.
- [ ] Replace the software backend with a GX2 texture renderer.

Initial replacements:

| PSX dependency | Wii U replacement |
| --- | --- |
| `PadRead`, `PAD*` | portable input API backed by `VPADRead` |
| `FntPrint` | portable Wii U UI text renderer |
| indexed `.TIM` images | converter plus row-RLE texture API |
| `Gs*`, `DrawOTag`, `LoadImage` | current `OSScreen` backend, later GX2 |
| `Spu*` | future AX/AX2 or SDL2_mixer-compatible audio layer |
| `Cd*` | future bundled-content or SD file API |
| memory-card functions | future Wii U save directory / SD fallback |

## Phase 2 — Minimal playable loop

- [x] title/menu state;
- [x] real title character texture;
- [x] real office panorama;
- [x] horizontal office movement;
- [x] open/close camera panel;
- [x] separate TV and GamePad views;
- [x] first sprite rendered through the renderer module;
- [x] converted warning, title and office textures;
- [x] selectable real CAM 01, CAM 02 and CAM 03 feeds;
- [x] static and glitch overlays on camera feeds;
- [ ] fixed-step update timing independent of rendering cost;
- [ ] animated title frames (`MENU1` to `MENU5`);
- [ ] real camera-map layout and all camera rooms;
- [ ] vent-camera mode.

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

Add the maintenance-panel state and its first controls, then begin connecting the camera system to actual game state. The next visual improvement should also add the remaining title frames (`MENU2` to `MENU5`) so Springtrap glitches between real PSX images instead of only shifting one frame.
