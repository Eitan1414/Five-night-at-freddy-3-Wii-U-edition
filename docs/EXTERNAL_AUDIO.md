# External audio overrides

The Wii U build loads raw signed 16-bit big-endian, 16 kHz, mono PCM files from:

`SD:/wiiu/apps/fnaf3-wiiu/audio/`

Every file is optional. Missing, empty, oversized or invalid files fall back to the embedded cue; new full-pack-only cues fall back to silence.

## Core game cues

- `vent_quiet1.bin`, `vent_quiet2.bin`
- `vent_closer1.bin`, `vent_louder2.bin`
- `alarm.bin`, `breathing.bin`
- `wait.bin`, `static_sound.bin`
- `scream3.bin`, `garble1.bin`, `mask.bin`
- `echo1.bin`, `echo3b.bin`, `echo4b.bin`
- `phone_night1.bin` through `phone_night6.bin`
- `six_am.bin`
- `select.bin`, `end.bin`
- `crank1.bin`, `crank2.bin`
- `lever1.bin`, `lever2.bin`
- `stare.bin`, `titlemusic.bin`, `startday.bin`

## Full original sound-pack cues

- office ambience: `tablefan.bin`, `rainstorm2.bin`
- Springtrap danger: `danger2b.bin`
- maintenance: `scanner4.bin`, `done.bin`
- minigame actions: `collect.bin`, `feed.bin`, `glitch2.bin`, `crush.bin`
- minigame/ending atmosphere: `crowd_children.bin`, `clock_chimes.bin`, `party_favor.bin`, `desolate_underworld.bin`
- secret minigame music: `mb1.bin`, `mb2.bin`, `mb4b.bin`, `mb5.bin`, `mb8.bin`, `mb9.bin`

The complete distributed package contains 49 converted audio files. The six Phone Guy calls use the supplied `night1final.wav` through `night6final.wav` recordings, while `six_am.bin` comes from the separately supplied six-AM track.

The SD card is mounted before the audio engine starts so all override files are available during `audio_init`.

Conversion example:

```sh
ffmpeg -i input.wav -ar 16000 -ac 1 -f s16be output.bin
```
