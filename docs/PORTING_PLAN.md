# Wii U porting plan

## Source audit

The PSX project remains the main technical reference. Its PlayStation-specific graphics, controller, storage, CD and SPU calls are being replaced by portable Wii U layers rather than recompiled directly.

## Phase 0 — Wii U bootstrap

- [x] native `wut` project;
- [x] `.elf`, `.rpx` and `.wuhb` outputs;
- [x] clean `WHBProc` lifecycle;
- [x] TV and GamePad output;
- [x] VPAD input;
- [x] GitHub Actions build;
- [x] custom application icon and TV/GamePad boot splashes.

## Phase 1 — Platform abstraction

- [x] portable input API;
- [x] shared TV/GamePad graphics API;
- [x] frame-clock wrapper;
- [x] sprite-mask and indexed RLE texture renderers;
- [x] PlayStation TIM converter;
- [x] portable `sndcore2` audio API;
- [x] generated MP3 → Wii U PCM pipeline;
- [x] SDL2/GX2 renderer with separate TV and GamePad targets;
- [x] GPU texture cache for indexed RLE assets;
- [ ] portable storage/save API.

## Phase 2 — Playable office loop

- [x] warning and animated title menu;
- [x] real office panorama;
- [x] separate TV office and GamePad systems;
- [x] ten camera-map nodes positioned from the supplied map;
- [x] five selectable and sealable ventilation routes;
- [x] Play Audio with three random echo cues;
- [x] maintenance panel and repair progress;
- [x] camera, audio and ventilation failures;
- [x] inactivity-triggered ventilation failure and blackout cycle;
- [x] 12 AM → 6 AM clock;
- [x] night introduction, victory and Game Over flows;
- [x] sequential session progression through Nights 1–6;
- [ ] fixed-step timing independent of rendering cost;
- [ ] regenerate CAM 04–10 from higher-resolution source frames;
- [ ] GamePad touch selection.

## Phase 3 — Springtrap

- [x] adjacent-room camera graph;
- [x] random stay/move/vent movement opportunities;
- [x] Night 2 six-second movement opportunity baseline;
- [x] no ventilation choice before 1 AM;
- [x] faster opportunities on later nights and late hours;
- [x] five ventilation destinations and one sealed vent;
- [x] office window, left doorway, hidden hall and direct-office states;
- [x] freeze while directly watched;
- [x] observation released by panels, blackout or Phantom events;
- [x] lure accepted only in an adjacent camera;
- [x] 20% valid-lure resistance from Night 3 onward;
- [ ] final per-night opportunity tables and balancing;
- [ ] full Springtrap jumpscare animation.

## Phase 4 — Phantom animatronics

- [x] shared once-per-night attack/effect limit;
- [x] Phantom Foxy monitor-open spawn and toy-box office state;
- [x] Phantom Balloon Boy CAM 01/07/09/10 cycle and 0.55-second reaction window;
- [x] Phantom Freddy hourly checks, forced 4 AM appearance and monitor defence;
- [x] Phantom Chica CAM 07 cycle, forced 5 AM appearance and office-left attack;
- [x] Phantom Mangle CAM 04 cycle, garble loop and Audio Devices failure;
- [x] Phantom Puppet CAM 08 cycle, input-blocking mask and ventilation failure;
- [x] prevent non-Foxy cycles before 1 AM;
- [x] compact PSX textures for camera, office and jumpscare presentation;
- [x] Phantom attacks cause system failures but never player death;
- [ ] restore every original distinct jumpscare frame at optimized resolution;
- [ ] tune exact per-night probabilities after console playtesting;
- [ ] add any remaining Phantom-specific transitions found during testing.

## Phase 5 — Remaining game content

- [ ] persistent save/load and unlocked-night state;
- [ ] phone calls and full ambience;
- [ ] Night 5 ending and minigames;
- [ ] final Night 6/aggressive-mode unlock flow;
- [ ] extras and additional completion rewards;
- [ ] release packaging and external asset policy.

## Immediate next task

Test the SDL2/GX2 renderer on real Wii U hardware. Verify Aroma boot stability, independent TV/GamePad output, transparency, CAM 01–10, Springtrap and Phantom overlays, monitor transitions, frame rate and audio stability. After hardware validation, regenerate the reduced camera sources at a higher optimized resolution and restore the missing distinct jumpscare frames.
