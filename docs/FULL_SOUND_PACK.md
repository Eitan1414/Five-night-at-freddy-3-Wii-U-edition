# Full original FNaF 3 sound pack

The supplied archive contains 62 WAV files. This port currently converts and routes 48 of those WAV files plus the separately supplied `six_am` track, for 49 active SD overrides.

## Routed gameplay audio

| Game event | Source WAV | SD file |
|---|---|---|
| Office fan | `tablefan.wav` | `tablefan.bin` |
| Rain ambience | `rainstorm2.wav` | `rainstorm2.bin` |
| Springtrap danger | `danger2b.wav` | `danger2b.bin` |
| Repair scanner | `scanner4.wav` | `scanner4.bin` |
| Repair complete | `done.wav` | `done.bin` |
| Camera static | `static_sound.wav` | `static_sound.bin` |
| Jumpscare scream | `scream3.wav` | `scream3.bin` |
| Phantom Mangle | `garble1.wav` | `garble1.bin` |
| Phantom Puppet | `mask.wav` | `mask.bin` |
| Secret collect/feed | `collect.wav`, `feed.wav` | `collect.bin`, `feed.bin` |
| Secret entry | `glitch2.wav` | `glitch2.bin` |
| Secret completion | chimes/crowd/party WAVs | normalized names in `audio/` |
| Follow Me event | `crush.wav` | `crush.bin` |
| Follow Me/Bad Ending atmosphere | `Desolate_Underworld2.wav` | `desolate_underworld.bin` |
| Six secret minigame tracks | `mb1`, `mb2`, `mb4b`, `mb5`, `mb8`, `mb9` | matching `.bin` files |
| Phone calls | `night1final.wav` through `night6final.wav` | `phone_night1.bin` through `phone_night6.bin` |

All previously supported vent, alarm, breathing, lure, UI, title, night-start, Game Over and six-AM cues are also supplied as external overrides.

## Reserved source WAVs

These files are preserved in the user's original archive but are not forced into unrelated events:

- `crazy garble.wav`
- `get.wav`, `get2.wav`
- `insuit.wav`
- `jump.wav`, `jump2.wav`, `jump3.wav`, `jump4.wav`
- `land.wav`, `run.wav`
- `laugh.wav`
- `long glitched2.wav`
- `scare.wav`
- `stop.wav`

They are candidates for a later platforming/movement pass on the secret minigames.

## Runtime safety

Every new full-pack cue has a silent embedded fallback. The game therefore remains bootable when the SD audio folder is incomplete. Existing core cues retain their previous embedded fallbacks.
