# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port targeting **Aroma** and the `.wuhb` format.

## Current development milestone

The current build contains:

- custom Wii U icon and separate TV/GamePad boot splashes;
- original PSX warning, animated title and office panorama;
- progression from **12 AM to 6 AM** through Nights 1–6;
- ten selectable camera nodes and five sealable ventilation routes;
- random Springtrap movement restricted to adjacent rooms;
- Springtrap movement opportunities that become faster on later nights;
- camera, audio and ventilation failures with maintenance reboots;
- inactivity-based ventilation failure and repeating blackout cycle;
- Play Audio with the supplied `echo1`, `echo3b` and `echo4b` cues;
- first complete gameplay pass for Phantom Foxy, Balloon Boy, Freddy, Chica, Mangle and Puppet;
- Wii U audio playback through `sndcore2`;
- automatic `.elf`, `.rpx` and `.wuhb` builds through GitHub Actions.

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

### Phantom Foxy

- active from the beginning of Night 2;
- chance to appear beside the office toy box whenever a monitor is opened;
- disappears when a monitor is reopened;
- attacks if the player turns left toward him while no monitor is raised;
- the attack causes a ventilation failure.

### Phantom Balloon Boy

- from Night 2, checks approximately every 26 seconds;
- can wait in CAM 01, CAM 07, CAM 09 or CAM 10;
- once viewed, the player has about **0.55 seconds** to change camera or close the monitor;
- otherwise BB performs a jumpscare and causes a ventilation failure.

### Phantom Freddy

- can begin crossing the office window when an hour changes;
- always receives an appearance at 4 AM from Night 2 onward unless already spent;
- raising either monitor protects the player;
- without a monitor, he ducks after roughly 2.5 seconds and attacks shortly afterward.

### Phantom Chica

- from Night 2, checks approximately every 35 seconds in CAM 07;
- always receives an appearance at 5 AM unless already spent;
- looking at her arcade image for about **0.25 seconds** moves her to the left side of the office;
- turning left then triggers her jumpscare and a ventilation failure.

### Phantom Mangle

- active from Night 3 in CAM 04;
- checks approximately every 35 seconds and receives a forced 5 AM appearance unless already spent;
- once viewed, the player has about **0.55 seconds** to change camera or close the monitor;
- failure brings Mangle to the window, loops `garble1`, and disables Audio Devices;
- the disturbance lasts longer on later nights.

### Phantom Puppet

- active from Night 3 in CAM 08;
- checks approximately every 30 seconds;
- once viewed, the player has about **0.55 seconds** to escape the camera;
- failure blocks the view and controls while `mask` plays;
- the effect lasts about 10 seconds on Night 3 and grows toward 17 seconds in aggressive Night 6;
- leaving causes a ventilation failure.

Night 6 currently acts as the aggressive development mode: Phantom checks become more frequent, their odds increase, and Springtrap moves faster.

## Audio cues

The build converts the supplied MP3 files to 16 kHz mono signed big-endian PCM before compilation:

- vent entry: `vent_quiet1`, `vent_quiet2`;
- vent exit: `vent_closer1`, `vent_louder2`;
- ventilation failure: `alarm`, `breathing`;
- camera monitor: `static_sound`;
- maintenance feedback: `wait`;
- Springtrap/Phantom jumpscare: `scream3`;
- Phantom Mangle: `garble1`;
- Phantom Puppet: `mask`;
- Play Audio: `echo1`, `echo3b`, `echo4b`.

## Controls

### Office

- **Left/Right**: look around;
- **X/Y**: open cameras;
- **−**: open maintenance;
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

### Development shortcut

Hold **+** and press **Right** in the office with panels closed to advance one hour.

## Build locally

Install the current devkitPro Wii U packages and `ffmpeg`, then run:

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

Copy the WUHB to:

```text
sd:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

## Current limitations

- CAM 04–10 have complete gameplay states and map positions but still reuse reduced room backgrounds in the software renderer;
- Phantom visuals use compact PSX textures; the first jumpscare pass uses zoom/shake animation and not every original frame is distinct yet;
- persistent save data, endings, minigames, full phone calls and final balancing remain unfinished;
- the supplied PSX archive has no clear redistribution licence, so converted assets remain in this private development repository.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the remaining work.
