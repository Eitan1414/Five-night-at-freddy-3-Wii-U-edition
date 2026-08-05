# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port targeting **Aroma** and the `.wuhb` format.

## Current development milestone

The current build contains:

- custom Wii U icon and separate TV/GamePad boot splashes;
- original PSX warning, animated title and office panorama;
- progression from **12 AM to 6 AM** through Nights 1–6;
- persistent save data with completed-night tracking;
- ten selectable camera nodes and five sealable ventilation routes;
- random Springtrap movement restricted to adjacent rooms;
- Springtrap movement opportunities that become faster on later nights;
- camera, audio and ventilation failures with maintenance reboots;
- inactivity-based ventilation failure and repeating blackout cycle;
- Play Audio with the supplied `echo1`, `echo3b` and `echo4b` cues;
- complete gameplay pass for Phantom Foxy, Balloon Boy, Freddy, Chica, Mangle and Puppet;
- original multi-frame Springtrap and Phantom jumpscare sequences;
- phone-call playback and mute flow for Nights 1–6;
- five playable **Follow Me** chapters between Nights 1–5;
- dedicated Night 5 bad ending, Night 6 unlock and Night 6 fire/newspaper ending;
- Extras menu with animatronic gallery, jumpscare viewer, Follow Me replay and credits;
- separate TV/GamePad rendering through the Wii U SDL2/GX2 backend;
- GPU-cached camera, office, menu, Springtrap and Phantom textures;
- Wii U audio playback through `sndcore2`;
- automatic `.elf`, `.rpx` and `.wuhb` builds through GitHub Actions.

## Finishing flow

Completing a night saves it immediately. Nights 1–5 then enter their matching Follow Me chapter. Completing Night 5 unlocks Night 6 and Extras. Completing Night 6 displays the final Fazbear's Fright fire/newspaper sequence and adds the second completion star to the title screen.

The existing `progress.dat` format remains compatible: its unlocked-night byte and completed-night mask are reused instead of replacing the save file.

## Phone calls and missing audio

The source PSX archive contains an interleaved `inter8.xa` bank, but the available build tools cannot recover valid complete voice tracks from it. The current build therefore uses temporary telephone tones while keeping the full call timing, mute control and per-night routing ready.

Provide these decoded files as WAV or MP3 to replace the temporary cues:

```text
phone_night1.wav
phone_night2.wav
phone_night3.wav
phone_night4.wav
phone_night5.wav
phone_night6.wav
six_am.wav
```

Mono or stereo WAV/MP3 is accepted. The build converts every file to Wii U-ready 16 kHz mono signed big-endian PCM.

## Mini-games and ending assets

The five story chapters currently use a native Wii U pixel-art recreation of **Follow Me**. The PSX archive does not contain the original PC sprite sheets, the original Bad Ending image, the fire newspaper image, or the six secret mini-games.

Optional original visual replacements can be supplied as PNG files:

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

The six secret mini-games are not yet implemented; their exact logic and room layouts will be added after their original assets are available.

## Springtrap rules

### Movement opportunities

On Night 2, Springtrap receives a movement opportunity approximately every six seconds. At each opportunity he can:

1. remain where he is;
2. move to one adjacent camera;
3. enter the ventilation connected to the current camera.

He cannot choose a ventilation route before **1 AM**. Later nights shorten the interval, with additional acceleration after 4 AM, after 5 AM and during a ventilation failure.

### Play Audio

The lure is accepted only when the selected target camera is directly adjacent to Springtrap's current camera. A lure played in any other room produces sound but Springtrap ignores it.

Starting on Night 3, a valid adjacent lure has a **20% chance of being ignored**. One of the three supplied echo sounds is chosen randomly for every activation.

### Ventilation routes

- **CAM 09 → CAM 06** (`VENT 11`);
- **CAM 07 → upper CAM 01 hall** (`VENT 12`);
- **CAM 05 → office window** (`VENT 13`);
- **CAM 10 → CAM 02** (`VENT 14`);
- **CAM 02 → directly inside the office** (`VENT 15`).

Only one vent can be sealed at a time. A sealed route returns Springtrap to its source room. An unsealed CAM 02 office route causes a direct Springtrap attack.

When Springtrap is visible at the front window or the left maintenance doorway, he cannot move while the player directly watches him. Looking away, opening a panel, a Phantom encounter or a ventilation blackout releases him.

## Phantom animatronics

Every Phantom can complete its attack/effect only once per night. Phantom attacks do **not** kill the player; they cause system problems and give Springtrap an opportunity to advance.

Except for Foxy, Phantom spawn cycles do not begin before **1 AM**.

- **Foxy:** monitor-open spawn, office-left reaction and ventilation failure.
- **Balloon Boy:** CAM 01/07/09/10 appearance, short escape window and ventilation failure.
- **Freddy:** office-window walk, monitor defence and forced 4 AM opportunity.
- **Chica:** CAM 07 arcade appearance, office-left attack and forced 5 AM opportunity.
- **Mangle:** CAM 04 appearance, window disturbance, `garble1` loop and Audio Devices failure.
- **Puppet:** CAM 08 appearance, control-blocking mask effect and ventilation failure.

Night 6 is the aggressive mode: Phantom checks become more frequent, their odds increase, and Springtrap moves faster.

## Audio cues

The build converts source audio to 16 kHz mono signed big-endian PCM before compilation:

- vent entry: `vent_quiet1`, `vent_quiet2`;
- vent exit: `vent_closer1`, `vent_louder2`;
- ventilation failure: `alarm`, `breathing`;
- camera monitor: `static_sound`;
- maintenance feedback: `wait`;
- Springtrap/Phantom jumpscare: `scream3`;
- Phantom Mangle: `garble1`;
- Phantom Puppet: `mask`;
- Play Audio: `echo1`, `echo3b`, `echo4b`;
- phone system: `phone_night1` through `phone_night6`;
- victory: `six_am`.

## Controls

### Office

- **Left/Right**: look around;
- **X/Y**: open cameras;
- **−**: open maintenance;
- **R**: mute the active phone call;
- **B**: return to title.

### Camera map

- **D-pad**: select a camera by map position;
- **A**: Play Audio;
- **R**: switch to the vent map;
- **X/Y**: close cameras;
- **−**: open maintenance.

### Vent map

- **D-pad**: select VENT 11–15;
- **A** or **L**: seal/unseal the selected vent;
- **R**: return to cameras;
- **X/Y**: close the panel.

### Maintenance

- **Up/Down**: select a system;
- **A/+**: reboot;
- **B/−**: close when no reboot is running.

### Follow Me

- **D-pad**: move the character;
- follow the purple shadow through four checkpoints;
- **B**: return to Extras during a replay.

### Extras

- **Up/Down**: select a category;
- **Left/Right**: change character, jumpscare or chapter;
- **A/+**: open or replay;
- **B**: return.

### Development shortcut

Hold **+** and press **Right** in the office with panels closed to advance one hour.

## Build locally

Install the current devkitPro Wii U packages, the `wiiu-sdl2` port, Python 3 and `ffmpeg`, then run:

```sh
sh tools/prepare_generated_assets.sh
make
```

GitHub Actions builds the official SDL2 Wii U port from its source package and devkitPro patch before compiling the game.

Expected outputs:

```text
fnaf3-wiiu.elf
fnaf3-wiiu.rpx
fnaf3-wiiu.wuhb
```

Copy the WUHB to:

```text
sd:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

## Current limitations

- the seven original phone/6 AM tracks are not decodable from the supplied XA bank and currently use explicit temporary cues;
- the six secret PC mini-games are not implemented because their sprites, maps and event graphics are absent from the PSX source;
- Follow Me and both endings currently use native Wii U recreations rather than the original PC artwork;
- CAM 01–10 are GPU-rendered and hardware-scaled, but several camera source frames remain reduced;
- CI validates conversion, compilation and linking only; complete story flow, controls, save migration and audio stability still require testing on real Wii U hardware;
- the supplied PSX archive has no clear redistribution licence, so converted assets remain in this private development repository.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the remaining work.
