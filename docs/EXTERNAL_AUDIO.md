# External audio overrides

The Wii U build loads raw signed 16-bit big-endian, 16 kHz, mono PCM files from:

`SD:/wiiu/apps/fnaf3-wiiu/audio/`

Supported names:

- `phone_night1.bin` through `phone_night6.bin`
- `six_am.bin`
- `select.bin`
- `end.bin`
- `crank1.bin` and `crank2.bin`
- `lever1.bin` and `lever2.bin`
- `stare.bin`
- `titlemusic.bin`
- `startday.bin`

Every file is optional. If an override is absent, empty, too large or invalid, the embedded fallback cue is used. The complete distributed SD package contains the six supplied Phone Guy calls, all supplied interface/music effects and the supplied `six_am` cue.

The SD card is mounted before the audio engine starts so the override files are available during `audio_init`.

Conversion example:

```sh
ffmpeg -i input.mp3 -ar 16000 -ac 1 -f s16be output.bin
```
