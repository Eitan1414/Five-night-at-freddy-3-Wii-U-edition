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
- [x] portable storage/save API;
- [x] completed-night mask and legacy-compatible progress migration.

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
- [x] original left and right multi-frame jumpscare sequences;
- [ ] final per-night opportunity tables and balancing.

## Phase 4 — Phantom animatronics

- [x] shared once-per-night attack/effect limit;
- [x] Phantom Foxy monitor-open spawn and toy-box office state;
- [x] Phantom Balloon Boy CAM 01/07/09/10 cycle and reaction window;
- [x] Phantom Freddy hourly checks, forced 4 AM appearance and monitor defence;
- [x] Phantom Chica CAM 07 cycle, forced 5 AM appearance and office-left attack;
- [x] Phantom Mangle CAM 04 cycle, garble loop and Audio Devices failure;
- [x] Phantom Puppet CAM 08 cycle, input-blocking mask and ventilation failure;
- [x] prevent non-Foxy cycles before 1 AM;
- [x] original camera and office Phantom textures regenerated from PSX TIM files;
- [x] original distinct BB, Chica, Freddy, Foxy and Puppet animation frames;
- [x] Phantom attacks cause system failures but never player death;
- [ ] tune exact per-night probabilities after console playtesting;
- [ ] add remaining Phantom-specific transitions found during testing.

## Phase 5 — Story and completion content

- [x] persistent save/load and unlocked-night state;
- [x] per-night phone-call playback, timing and mute interface;
- [x] temporary fallback audio when original calls cannot be decoded;
- [x] five playable Follow Me story chapters;
- [x] Night 5 bad-ending flow;
- [x] clean Night 6 and Extras unlock after Night 5 completion;
- [x] Night 6 fire/newspaper ending;
- [x] title completion stars;
- [x] Extras animatronic gallery;
- [x] Extras jumpscare viewer;
- [x] Extras Follow Me replay menu;
- [x] Extras credits;
- [ ] replace temporary phone cues with `phone_night1.wav` through `phone_night6.wav`;
- [ ] replace temporary 6 AM cue with `six_am.wav`;
- [ ] replace native Follow Me recreation with original sprite material when supplied;
- [ ] replace native Bad Ending and Night 6 newspaper recreations with original images;
- [ ] implement BB's Air Adventure;
- [ ] implement Mangle's Quest;
- [ ] implement Chica's Party;
- [ ] implement Stage01;
- [ ] implement Shadow Bonnie;
- [ ] implement Happiest Day;
- [ ] completion rewards tied to secret-minigame progression;
- [ ] release packaging and external asset policy.

## Required finishing assets

The PSX `inter8.xa` bank did not produce valid complete audio in CI. Supply decoded WAV or MP3 files using these exact names:

```text
phone_night1.wav
phone_night2.wav
phone_night3.wav
phone_night4.wav
phone_night5.wav
phone_night6.wav
six_am.wav
```

The PSX project also lacks the original PC story/ending and secret-minigame graphics. Preferred replacement names are:

```text
follow_me_sprites.png
bad_ending.png
night6_newspaper.png
minigame_bb_air_adventure.png
minigame_mangles_quest.png
minigame_chicas_party.png
minigame_stage01.png
minigame_shadow_bonnie.png
minigame_happiest_day.png
```

## Immediate next task

Test the finishing build on real Wii U hardware. Verify phone overlay and mute control, transition from every victory into Follow Me, Night 5 save/unlock behavior, Night 6 ending, Extras navigation, replay return paths, old-save compatibility, TV/GamePad synchronization and audio stability. After that test, replace the seven temporary audio cues and implement the six secret mini-games from supplied original assets.
