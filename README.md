# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port project targeting **Aroma** and the `.wuhb` format.

## Current status

The repository currently contains the Phase 0 bootstrap:

- native C application built with devkitPro `wut`;
- clean Wii U process startup and shutdown;
- text output on the TV and GamePad;
- Wii U GamePad button detection through `VPADRead`;
- automatic `.elf`, `.rpx` and `.wuhb` builds with GitHub Actions.

This is not yet the game. It is the stable platform layer on which the PSX game logic will be ported.

## Build locally

Install the current Wii U development packages through devkitPro, then run:

```sh
make
```

Expected outputs:

```text
fnaf3-wiiu.elf
fnaf3-wiiu.rpx
fnaf3-wiiu.wuhb
```

## Test on Wii U

Copy the generated file to:

```text
sd:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

Launch it from the Aroma Wii U Menu.

## Controls in the bootstrap

- **A**: confirms that GamePad input works;
- **B**: displays a back/cancel test message;
- **+**: displays the current port phase;
- **HOME**: return using the normal system flow.

## Porting policy

The PSX archive supplied for analysis does not contain a clear open-source licence for its code or assets. For that reason, the original files are not copied into this repository yet. We will first keep the Wii U platform code clean, then import or reimplement only material that can legally and technically be used.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the migration plan.
