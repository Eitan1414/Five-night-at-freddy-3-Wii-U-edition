# External audio overrides

The Wii U build can load raw signed 16-bit big-endian, 16 kHz, mono PCM files from:

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

Every file is optional. If an override is absent or invalid, the embedded fallback cue is used. The distributed test package contains the recordings supplied for this Wii U edition except `six_am.bin`, which was not included in the received attachments.

Conversion example:

```sh
ffmpeg -i input.mp3 -ar 16000 -ac 1 -f s16be output.bin
```
