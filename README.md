# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port targeting **Aroma** and the `.wuhb` format.

## Complete-current milestone

The current build contains:

- custom Wii U icon and separate TV/GamePad boot splashes;
- original PSX warning, animated title, office panorama and camera material;
- progression from **12 AM to 6 AM** through Nights 1–6;
- versioned 16-byte autosave with backup recovery, completed nights, six secret-minigame flags and four-star progress;
- ten selectable camera nodes and five sealable ventilation routes;
- Springtrap adjacency movement, audio lure, vents, office observation and jumpscare logic;
- camera, audio and ventilation failures with maintenance reboots;
- complete Phantom Foxy, Balloon Boy, Freddy, Chica, Mangle and Puppet gameplay pass;
- original multi-frame Springtrap and Phantom jumpscare sequences;
- real supplied Phone Guy calls for Nights 1–6 through optional SD audio overrides;
- supplied menu, monitor, maintenance, Game Over, night-start, 6 AM and post-night sounds;
- five playable **Follow Me** story chapters after Nights 1–5;
- six playable Good Ending minigames with both Extras replay and hidden in-night triggers;
- original supplied Bad Ending artwork, Night 6 fire/newspaper ending and Good Ending;
- Extras with Animatronics, Jumpscares, Secret Minigames, Cheats and Credits;
- original four cheat options: Fast Nights, Radar, Aggressive and No Errors;
- four persistent title stars, including Aggressive-only Nightmare completion;
- separate TV/GamePad rendering through Wii U SDL2/GX2;
- GPU-cached RLE textures and Wii U `sndcore2` audio;
- automatic `.elf`, `.rpx` and `.wuhb` builds through GitHub Actions.

## Story and unlock flow

Completing a night saves immediately. Nights 1–5 enter their matching Follow Me chapter. Completing Night 5 displays the Bad Ending, unlocks Night 6 and unlocks Extras. Completing Night 6 displays the Fazbear's Fright fire/newspaper ending.

Completing all six secret minigames unlocks Happiest Day and the Good Ending. The title stars are:

1. Night 5 complete;
2. Night 6 complete;
3. Good Ending complete;
4. Night 6 complete with **Aggressive enabled and every other cheat disabled**.

The Cheats page unlocks after Night 6 and the Good Ending are complete.

## External audio package

The game first mounts `SD:/wiiu/apps/fnaf3-wiiu/`, then loads optional raw PCM replacements from:

```text
SD:/wiiu/apps/fnaf3-wiiu/audio/
```

The distributed package supplies:

```text
phone_night1.bin ... phone_night6.bin
six_am.bin
select.bin
end.bin
crank1.bin
crank2.bin
lever1.bin
lever2.bin
stare.bin
titlemusic.bin
startday.bin
```

The format is signed 16-bit big-endian, 16 kHz, mono PCM. Every external file is optional; a missing or invalid file falls back to the embedded cue.

See [`docs/EXTERNAL_AUDIO.md`](docs/EXTERNAL_AUDIO.md).

## Hidden minigame triggers

The six minigames remain directly replayable from Extras. They can also be found during nights with Wii U controller equivalents of the original triggers:

- **BB's Air Adventure:** CAM 08, hold **L**, press **A**;
- **Mangle's Quest:** from Night 2, CAM 07, hold **X/Y**, then enter upper-left, lower-left, upper-right, lower-right;
- **Chica's Party:** from Night 3, hold **L** and press **A** on CAM 02, 03, 04 and 06;
- **Stage 01:** from Night 4, pan fully left, hold **L** and press **A** for `3-9-5-2-4-8`: upper-right, lower-right, centre, up, left, down;
- **Shadow Bonnie:** from Night 5, pan fully right, hold **L**, press **A**;
- **Happiest Day:** CAM 03, hold **R**, press **A**.

A hidden minigame pauses the night and returns to the same office when left or completed. See [`docs/SECRET_MINIGAMES.md`](docs/SECRET_MINIGAMES.md).

## Cheats

- **Fast Nights:** doubles clock progression;
- **Radar:** marks Springtrap's real camera or vent source;
- **Aggressive:** gives Springtrap a second AI update every gameplay frame;
- **No Errors:** prevents camera, audio and ventilation failures while retaining Phantom encounters.

Cheat selections last until the application closes and are not written to the save. See [`docs/CHEATS.md`](docs/CHEATS.md).

## Controls

### Office

- **Left/Right:** look around;
- **X/Y:** open cameras;
- **−:** open maintenance;
- **R:** mute the active phone call;
- **B:** return to title.

### Camera map

- **D-pad:** select a camera;
- **A:** Play Audio;
- **R:** switch to vent map;
- **X/Y:** close cameras;
- **−:** open maintenance.

### Vent map

- **D-pad:** select VENT 11–15;
- **A/L:** seal or unseal;
- **R:** return to cameras;
- **X/Y:** close the monitor.

### Maintenance

- **Up/Down:** select a system;
- **A/+**: reboot;
- **B/−:** close when no reboot is active.

### Follow Me and secret minigames

- **D-pad / left stick:** move;
- **A:** interact or change Shadow Bonnie room;
- **R:** alternate Shadow Bonnie room control;
- **B:** leave a replay or unfinished hidden minigame.

### Extras and Cheats

- **Up/Down:** select;
- **Left/Right:** change gallery, jumpscare or chapter;
- **A/+**: open, replay or toggle;
- **B:** return.

### Development shortcut

Hold **+** and press **Right** in the office with panels closed to advance one hour.

## Installation

Copy the package's `SD` folder contents to the root of the Wii U SD card. The final layout must be:

```text
SD:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
SD:/wiiu/apps/fnaf3-wiiu/audio/*.bin
```

Launch **FNaF3 Wii U** from Aroma's homebrew menu.

## Build locally

Install the current devkitPro Wii U packages, Wii U SDL2, Python 3 and `ffmpeg`, then run:

```sh
sh tools/prepare_generated_assets.sh
make
```

Expected outputs:

```text
fnaf3-wiiu.elf
fnaf3-wiiu.rpx
fnaf3-wiiu.wuhb
```

## Current limitations

- the six secret minigames and Follow Me use native Wii U pixel-art gameplay recreations based on the supplied sheets rather than full frame-for-frame PC scene conversion;
- the supplied original Bad Ending is integrated, while the Night 6 newspaper/fire scene remains a native Wii U recreation;
- several camera source frames remain reduced compared with the PC release;
- the separate corrected full sound-effects archive was not available to the build environment, so the package contains every individually accessible supplied cue plus the six calls and `six_am`;
- CI validates conversion, compilation, linking and package outputs; complete controller timing, old-save migration, SD audio memory use and TV/GamePad synchronization still require testing on real Wii U hardware;
- converted copyrighted assets remain in this private development repository and test package.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for remaining visual and hardware polish.