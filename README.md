# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port targeting **Aroma** and the `.wuhb` format.

## Current status

The port now has a complete **multi-night gameplay foundation** rather than a single endless office test:

- custom Wii U icon and separate TV/GamePad boot splashes;
- original PSX warning, animated title frames and office panorama;
- separate TV office and GamePad camera/maintenance views;
- CAM 01–03 with real PSX Springtrap appearances;
- Play Audio lure, system failures and timed repairs;
- real clock progression from **12 AM to 6 AM**;
- Night introduction, 5:59 → 6:00 AM transition and completion screen;
- Game Over, retry and return-to-menu flow;
- sequential progression through Nights 1–5 during the current session;
- per-night difficulty configuration for Springtrap speed, system failures and attack time;
- automatic `.elf`, `.rpx` and `.wuhb` builds through GitHub Actions.

### Current night rules

**Night 1** is now faithful to its role as an orientation shift: Springtrap is inactive and the three systems do not fail. The player can learn the office, cameras, audio panel and maintenance controls before reaching 6 AM.

From **Night 2**, Springtrap starts in CAM 03 and advances toward CAM 01. When he remains near the office for too long, the game reaches Game Over unless the player attracts him away with Play Audio. Nights 3–5 progressively shorten his movement and attack timers and trigger system failures earlier. At 4 AM and 5 AM, his movement becomes more aggressive.

This is still a development build. The complete ten-camera map, vent cameras, phantom animatronics, real audio playback, persistent saves, full jumpscare animation and the ending minigames remain to be ported.

## Build locally

Install the current Wii U development packages through devkitPro, then run:

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

GitHub Actions performs generated-asset preparation automatically.

## Install on Wii U

Copy the generated file to:

```text
sd:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

Launch it from the Aroma Wii U Menu.

## Controls

### Warning and night introduction

- **A** or **+**: skip.

### Title screen

- **Up/Down**: change selection;
- **A** or **+**: confirm;
- after completing a night, New Game continues from the newly unlocked night for the current session.

### Office

- **Left/Right**: look around;
- **X** or **Y**: open or close cameras;
- **−**: open or close maintenance;
- **B**: return to the title screen.

### Cameras

- **Left/Right** or **Up/Down**: switch between CAM 01, CAM 02 and CAM 03;
- **A**: play the audio lure in the selected room;
- **X** or **Y**: close cameras;
- **−**: open maintenance.

During Night 1, Play Audio reports that no movement was detected because Springtrap is not active yet.

### Maintenance

- **Up/Down**: select Camera System, Audio Devices, Ventilation, Reboot All or Exit;
- **A** or **+**: begin the selected reboot;
- **B** or **−**: close when no reboot is running.

### Victory and Game Over

- after 6 AM, **A** or **+** starts the next night;
- on Game Over, **A** retries and **B** returns to the menu.

### Development shortcut

- hold **+** and press **Right** in the office, with panels closed, to advance one hour for testing.

The HOME button continues to use the normal Wii U system flow.

## Project structure

```text
include/                 portable interfaces and asset declarations
source/game/             base application states
source/platform/         Wii U input, display and frame clock
source/renderer/         sprite and indexed-texture renderers
source/main_parts/       gameplay implementation fragments
tools/                   TIM converter and generated-asset preparation
```

## Porting policy

The supplied PSX fan-project archive does not contain a clear source-code or asset licence. Converted game textures are kept in this private development repository for port testing and should not be redistributed publicly without confirming permission.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the remaining migration plan.
