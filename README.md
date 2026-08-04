# Five Nights at Freddy's 3 — Wii U Edition

Native Wii U homebrew port project targeting **Aroma** and the `.wuhb` format.

## Current status

The port now contains a small interactive Night 1 systems prototype:

- native C application built with devkitPro `wut`;
- portable GamePad input and shared TV/GamePad graphics APIs;
- converter and row-compressed renderer for indexed PlayStation `.TIM` images;
- original PSX warning graphic;
- title screen animated with the five real `MENU1`–`MENU5` Springtrap frames;
- converted PSX office panorama with horizontal movement;
- separate GamePad camera system with CAM 01, CAM 02 and CAM 03;
- scanlines and intermittent camera glitches;
- maintenance panel with camera, audio and ventilation status;
- timed camera, ventilation and audio failures during the office test;
- individual system reboot and **Reboot All** progress sequences;
- camera failure replaces the GamePad feed with a signal-lost screen;
- ventilation failure displays a flashing warning over the office;
- automatic `.elf`, `.rpx` and `.wuhb` builds through GitHub Actions.

This remains a development prototype. Springtrap AI, phantom encounters, complete camera/vent maps, sound, saves and full night progression are not integrated yet.

## Project layout

```text
include/
├── assets/
├── game/
├── platform/
└── renderer/
source/
├── assets/
├── game/
├── main_parts/
├── platform/
├── renderer/
├── main.c
├── menu_springtrap_texture.c
├── menu_springtrap_texture2.c
├── menu_springtrap_texture3.c
├── menu_springtrap_texture4.c
├── menu_springtrap_texture5.c
├── office_texture.c
├── camera01_texture.c
├── camera02_texture.c
└── camera03_texture.c
tools/
└── convert_tim.py
```

`main.c` currently includes several small implementation fragments under `source/main_parts/`. This is only an upload-friendly organization for the current development milestone; it behaves as one C translation unit.

## Texture pipeline

The temporary software texture path uses:

- Wii U `RGBX8` palettes;
- row-by-row run-length compression;
- a transparent palette index;
- nearest-neighbour scaling;
- the same API for TV, GamePad or both.

Convert a supported 4-bit or 8-bit indexed PlayStation TIM with:

```sh
python3 tools/convert_tim.py input.tim source/generated_texture.c include/assets/generated_texture.h \
  --symbol gGeneratedTexture --sample-step 2
```

The large visuals currently use reduced-resolution RLE textures so they remain practical with the early `OSScreen` backend. A later GX2 renderer will enable higher-resolution images and faster full-screen animation.

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

## Current controls

### Warning

- **A** or **+**: skip.

### Title screen

- **Up/Down**: change selection;
- **A** or **+**: confirm.

### Office test

- **Left/Right**: scroll through the office panorama;
- **X** or **Y**: open or close the camera panel;
- **−**: open or close maintenance;
- **B**: return to the title screen.

### Camera panel

- **Left/Right** or **Up/Down**: switch between CAM 01, CAM 02 and CAM 03;
- **X** or **Y**: close the camera panel;
- **−**: open maintenance;
- the TV remains on the office view.

### Maintenance panel

- **Up/Down**: select Camera System, Audio Devices, Ventilation, Reboot All or Exit;
- **A** or **+**: start the selected reboot;
- **B** or **−**: close the panel when no reboot is running.

An individual reboot lasts about two seconds; **Reboot All** lasts about four seconds. For this prototype, the camera failure starts around 12 seconds into the office, ventilation around 24 seconds and audio around 36 seconds, making the repair loop easy to test.

The HOME button continues to use the normal Wii U system flow.

## Porting policy

The supplied PSX fan-project archive does not contain a clear source-code or asset licence. Converted game textures are therefore kept in this private development repository for port testing and should not be redistributed publicly without confirming permission. The port architecture and conversion tools remain separated from imported game assets.

See [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) for the migration plan.
