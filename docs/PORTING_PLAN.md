# Wii U porting plan

## Source audit

The PSX project is the main technical reference because it is written in C and already contains the complete game flow. Its PlayStation-specific graphics, controller, storage and SPU/CD calls must be replaced rather than recompiled directly.

## Phase 0 — Wii U bootstrap

- [x] Native `wut` project;
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
- [ ] portable audio API;
- [ ] portable storage/save API;
- [ ] GX2 texture renderer.

## Phase 2 — Playable office loop

- [x] warning and animated title menu;
- [x] real office panorama;
- [x] separate TV office and GamePad systems;
- [x] CAM 01–03 feeds and Springtrap composites;
- [x] Play Audio lure prototype;
- [x] maintenance panel and repair progress;
- [x] camera, audio and ventilation failures;
- [x] 12 AM → 6 AM clock;
- [x] night introduction screen;
- [x] 5:59 → 6:00 AM victory transition;
- [x] Game Over, retry and menu return;
- [x] sequential session progression through Nights 1–5;
- [x] Night 1 orientation rules with Springtrap inactive;
- [x] per-night Springtrap speed, failure schedule and attack grace;
- [x] increased aggression at 4 AM and 5 AM;
- [ ] fixed-step timing independent of rendering cost;
- [ ] CAM 04–10 and real map layout;
- [ ] vent-camera mode and vent sealing.

## Phase 3 — Complete game systems

- [ ] full Springtrap room graph and decision logic;
- [ ] real jumpscare animation and sound;
- [ ] phantom animatronics and hallucinations;
- [ ] phone calls and ambience;
- [ ] real audio playback for Play Audio;
- [ ] persistent save/load and unlocked-night state;
- [ ] Night 5 ending and minigames;
- [ ] Night 6 and extras/custom-night content.

## Phase 4 — Wii U features and release

- [x] custom icon and separate TV/GamePad boot images;
- [ ] GamePad touch camera map;
- [ ] GamePad speaker effects;
- [ ] Wii U Pro Controller support;
- [ ] optimized GX2 rendering;
- [ ] release packaging and external asset policy.

## Immediate next task

Complete Night 1 presentation and prepare Night 2 gameplay by converting CAM 04–10 plus the real camera-map layout. Then extend Springtrap from the current three-room route to the complete building route and add vent cameras/sealing. In parallel, add the first Wii U audio backend for camera movement, static and Play Audio.
