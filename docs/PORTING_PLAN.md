# Wii U porting plan

## Source audit

The PSX project is the appropriate technical reference because it is written in C and already contains the complete game flow. The audit found:

- `fnaf3.c`: about 905 lines;
- most implementation is placed in large headers under `objects/`;
- direct PlayStation APIs for graphics, controller input, CD access, memory cards and SPU audio;
- PSX-native `.TIM`, `.VAG` and XA/CD resources;
- no clear source-code or asset licence in the supplied archive.

The code cannot simply be recompiled with `wut`; game logic must be separated from the PSX hardware layer.

## Phase 0 — Wii U bootstrap

- [x] Create a native `wut` project.
- [x] Produce `.elf`, `.rpx` and `.wuhb` outputs.
- [x] Initialize and shut down cleanly through `WHBProc`.
- [x] Display output on TV and GamePad.
- [x] Read GamePad buttons with VPAD.
- [x] Add a GitHub Actions build.

## Phase 1 — Platform abstraction

- [x] Add a portable input API.
- [x] Move `VPADRead` logic out of game code.
- [x] Add a graphics API targeting TV, GamePad or both.
- [x] Move `OSScreen` allocation and drawing out of game code.
- [x] Add a frame-clock wrapper.
- [x] Add sprite-mask and row-compressed indexed texture renderers.
- [x] Add a converter for indexed PlayStation `.TIM` files.
- [x] Display converted PSX textures on TV and GamePad.
- [ ] Add portable audio API.
- [ ] Add portable storage/save API.
- [ ] Replace the software backend with a GX2 texture renderer.

| PSX dependency | Wii U replacement |
| --- | --- |
| `PadRead`, `PAD*` | portable input API backed by `VPADRead` |
| `FntPrint` | portable Wii U UI text renderer |
| indexed `.TIM` images | converter plus row-RLE texture API |
| `Gs*`, `DrawOTag`, `LoadImage` | current `OSScreen` backend, later GX2 |
| `Spu*` | future AX/AX2 audio layer |
| `Cd*` | future bundled-content or SD file API |
| memory-card functions | future Wii U save directory / SD fallback |

## Phase 2 — Minimal playable loop

- [x] title/menu state;
- [x] all five real title frames (`MENU1` to `MENU5`);
- [x] real office panorama and horizontal movement;
- [x] separate TV and GamePad views;
- [x] open/close camera panel;
- [x] selectable real CAM 01, CAM 02 and CAM 03 feeds;
- [x] static and glitch overlays on camera feeds;
- [x] maintenance panel opened with Minus;
- [x] camera, audio and ventilation state flags;
- [x] timed prototype failures;
- [x] individual reboot and Reboot All progress;
- [x] camera failure affects the GamePad feed;
- [x] ventilation failure affects the TV office view;
- [ ] fixed-step update timing independent of rendering cost;
- [ ] real camera-map layout and all camera rooms;
- [ ] vent-camera mode.

## Phase 3 — Game systems

- [ ] full camera map and vents;
- [x] first maintenance-panel loop;
- [ ] connect audio-device state to an actual lure mechanic;
- [ ] connect ventilation state to hallucinations and danger;
- [ ] Springtrap AI and room movement;
- [ ] phantom encounters and jumpscares;
- [ ] time progression and night completion;
- [ ] save/load and unlocks.

## Phase 4 — Wii U features

- TV: office and main view;
- GamePad: camera system and touch controls;
- GamePad speaker for radio/static effects;
- optional Wii U Pro Controller support;
- proper icon and TV/GamePad boot splashes;
- release packaging.

## Immediate next task

Connect the system flags to real gameplay. Add a minimal Springtrap state that moves between the three available camera rooms, make the selected camera reveal his current location, and add an audio-lure action whose availability depends on the Audio Devices system. After that, expand the camera map and begin vent-camera mode.
