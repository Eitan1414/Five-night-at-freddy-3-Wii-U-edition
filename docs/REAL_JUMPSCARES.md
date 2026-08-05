# Original PSX jumpscare sequences

The Wii U port generates its jumpscare textures from the original FNaF 3 PSX source at pinned commit `f19e22762f48d4269e07730827d8eaab5995557b`.

Included frame sequences:

- Springtrap left attack: 13 frames;
- Springtrap right attack: 12 frames;
- Phantom Balloon Boy: 9 frames;
- Phantom Chica: 6 frames;
- Phantom Freddy: 7 frames;
- Phantom Foxy: 11 frames.

Every sequence advances at the original PSX cadence of four game ticks per image. The original `vag/screamer.vag` is also converted to the Wii U PCM format during asset preparation.

Phantom Mangle and Phantom Puppet intentionally keep their source-accurate special attacks instead of receiving invented frame sequences. Mangle appears at the office window, plays the garble loop and disables Audio Devices. Puppet covers the view, blocks input and later causes a ventilation failure.

For local builds, extract the PSX source and set `FNAF3_PSX_SOURCE` before preparing assets:

```sh
FNAF3_PSX_SOURCE=/path/to/Five-Night-at-Freddys-3-PSX-main \
  sh tools/prepare_generated_assets.sh
make
```

GitHub Actions fetches only `tim/screamer` and `vag/screamer.vag` from the pinned upstream revision.
