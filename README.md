# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port targeting **Aroma** and the `.wuhb` format.

## Current status

The current build contains the first complete Springtrap/vent gameplay pass:

- custom Wii U icon and separate TV/GamePad boot splashes;
- original PSX warning, animated title and office panorama;
- clock progression from **12 AM to 6 AM** and Nights 1–5;
- Game Over, retry, victory and next-night flow;
- a ten-node camera map positioned like the supplied FNaF 3 map;
- random Springtrap movement restricted to adjacent rooms;
- faster movement on later nights and after 4 AM/5 AM;
- five functional ventilation routes with one sealable vent at a time;
- office window, hidden upper hall, left doorway and direct-office states;
- Springtrap remains frozen while the player directly watches him;
- camera, audio and ventilation failures with repairs;
- inactivity-based ventilation failure after roughly ten seconds;
- repeating dim/blackout cycle until ventilation is repaired;
- real Wii U audio playback through `sndcore2`;
- automatic `.elf`, `.rpx` and `.wuhb` builds through GitHub Actions.

## Springtrap camera graph

Springtrap only chooses a room connected to his current room. The ten camera nodes follow the attached map rather than a simple numeric sequence.

Ventilation shortcuts:

- **CAM 09 → CAM 06** (`VENT 11`);
- **CAM 07 → upper CAM 01 hall** (`VENT 12`), then CAM 01 or the office window;
- **CAM 05 → office window** (`VENT 13`);
- **CAM 10 → CAM 02** (`VENT 14`);
- **CAM 02 → directly inside the office** (`VENT 15`).

If the active vent is sealed, Springtrap is forced back to its source camera. An unsealed CAM 02 office vent produces an immediate attack when he exits it.

## Office observation rules

When Springtrap is at the front window or the left maintenance doorway, he cannot advance while directly visible:

- look right to watch the window;
- look left to watch the doorway.

He can move again when the player opens cameras or maintenance, looks in the opposite direction, or is blinded by a ventilation blackout. Phantom encounters are deliberately not enabled yet, but the state transitions are ready to release Springtrap when those encounters are added later.

## Ventilation failure

Remaining inactive in the office for about ten seconds triggers a ventilation error. Moving the office view, opening a panel or interacting with a panel resets the inactivity timer. Watching Springtrap without moving still counts as inactivity.

After the error:

1. the screen begins dimming after about three seconds;
2. it becomes black at about six seconds;
3. it returns after roughly four more seconds;
4. the cycle repeats until Ventilation is repaired.

The alarm loops immediately, and breathing starts when the first blackout approaches.

## Audio cues

The supplied files are included and converted to Wii U PCM during the build:

- `vent_quiet1` / `vent_quiet2`: Springtrap enters a vent;
- `vent_closer1` / `vent_louder2`: Springtrap exits a vent;
- `alarm`: ventilation-error loop;
- `breathing`: player breathing during ventilation failure;
- `static_sound`: camera loop, restarted whenever the selected camera changes;
- `wait`: maintenance/lure feedback;
- `scream3`: Game Over attack.

`danger2b` was not included in the uploaded files. Soft and loud danger states are implemented visually and are ready to receive that sound later. Soft danger corresponds to CAM 05/window proximity; loud danger corresponds to CAM 01, CAM 02 or the left office doorway. Phantom Chica/Freddy danger triggers are reserved but not active.

## Current graphical limitation

CAM 01–03 use their converted PSX feeds. CAM 04–10 have independent labels, positions, AI states and vent routes, but the active build still reuses the nearest existing room backgrounds while their larger PSX texture pack is being moved into the renderer. Springtrap has a camera-specific placement in all ten feeds.

## Controls

### Office

- **Left/Right**: look around;
- **X/Y**: open cameras;
- **−**: open maintenance;
- **B**: return to the title screen.

### Camera map

- **D-pad**: select a camera using its position on the map;
- **A**: Play Audio in the selected room;
- **R**: switch between camera and vent maps;
- **X/Y**: close the panel;
- **−**: open maintenance.

### Vent map

- **D-pad**: select VENT 11–15;
- **A** or **L**: seal/unseal the selected vent;
- **R**: return to the camera map;
- **X/Y**: close the panel.

Only one vent can be sealed at once.

### Maintenance

- **Up/Down**: select Camera System, Audio Devices, Ventilation, Reboot All or Exit;
- **A/+**: reboot;
- **B/−**: close when no reboot is running.

### Testing shortcut

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

## Install on Wii U

Copy the generated WUHB to:

```text
sd:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

## Porting policy

The supplied PSX fan-project archive does not contain a clear source-code or asset licence. Converted assets remain in this private development repository and should not be redistributed publicly without confirming permission.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the remaining work.
