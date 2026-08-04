# Wii U porting plan

## Source audit

The PSX project remains the main C reference. PlayStation graphics, controller, storage and SPU/CD calls are replaced by portable Wii U layers rather than compiled directly.

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
- [x] indexed RLE texture renderer and TIM converter;
- [x] portable `sndcore2` audio API;
- [x] build-time MP3 → signed big-endian PCM conversion;
- [ ] portable storage/save API;
- [ ] GX2 texture renderer.

## Phase 2 — Playable office loop

- [x] warning, animated title and office panorama;
- [x] separate TV office and GamePad systems;
- [x] ten selectable camera nodes positioned like the supplied map;
- [x] real CAM 01–03 feeds;
- [ ] finish integrating the real CAM 04–10 backgrounds into the active renderer;
- [x] camera map and vent-map modes;
- [x] one sealable vent at a time;
- [x] maintenance panel and system repair;
- [x] 12 AM → 6 AM clock and Nights 1–5;
- [x] victory, Game Over and retry flow;
- [x] inactivity-triggered ventilation error;
- [x] dim/blackout/recovery cycle;
- [ ] fixed-step timing independent of rendering cost.

## Phase 3 — Complete game systems

- [x] ten-room adjacent-camera Springtrap graph;
- [x] per-night random movement speed;
- [x] five vent routes and blocked-vent return behavior;
- [x] CAM 02 direct-office vent attack;
- [x] hidden upper-hall and office-window routes;
- [x] freeze Springtrap while directly observed at the window/left doorway;
- [x] release him when panels open, the player looks away or blackout occurs;
- [x] real vent entry/exit, alarm, breathing, static and jumpscare audio;
- [x] Play Audio restricted to an adjacent target room;
- [ ] add the missing `danger2b` sound to the implemented soft/loud danger states;
- [ ] full jumpscare animation;
- [ ] phantom animatronics and hallucinations;
- [ ] phone calls and general ambience;
- [ ] persistent save/load and unlocked-night state;
- [ ] Night 5 ending and minigames;
- [ ] Night 6 and extras/custom-night content.

## Phase 4 — Wii U features and release

- [x] custom icon and separate TV/GamePad boot images;
- [ ] GamePad touch camera map;
- [ ] separate GamePad speaker routing for selected effects;
- [ ] Wii U Pro Controller support;
- [ ] optimized GX2 rendering;
- [ ] release packaging and external asset policy.

## Immediate next task

Test the new ten-camera/vent build on real hardware, then finish activating the converted CAM 04–10 image pack. After visual validation, add the complete Springtrap office jumpscare animation and begin the Phantom systems without enabling unfinished encounters in normal play.
