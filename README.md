# Five Nights at Freddy's 3 — Wii U Edition

An unofficial **Five Nights at Freddy's 3** port made specifically for the **Nintendo Wii U**, with support for **Aroma**, the Wii U GamePad and Wii U-specific presentation and control features.

> This is a fan-made project. It is not affiliated with, endorsed by, or supported by the original developers, publishers, Nintendo, or the respective rights holders.

## Stable release

**v1.0 is now available.**

➡️ [Download Five Nights at Freddy's 3 — Wii U Edition v1.0](../../releases/tag/v1.0)

The release provides:

- an **Aroma / WUHB** package, recommended for real Wii U homebrew users;
- an **installable WUP Channel** package generated and structurally validated by CI;
- a dedicated **Cemu** package in decrypted `code/content/meta` format.

The final v1.0 publication build also passed the Springtrap, Phantom and final fidelity/stress validation suite before release.

## About the Wii U Edition

The goal of this project is to make FNaF 3 feel at home on Wii U rather than simply placing the game on another platform.

The port includes Wii U-specific controls, TV/GamePad rendering, persistent progression, adapted menus, Extras content and quality-of-life additions while keeping the main FNaF 3 gameplay aligned with the PC/MFA behavior used as the project's reference.

## Features

- Full **Night 1–6** progression
- PC/MFA-derived Springtrap AI, ventilation routes, audio lure and system failures
- **Phantom Freddy, Foxy, Chica, Balloon Boy, Mangle and Puppet**
- Camera and maintenance systems adapted for Wii U controls
- TV + **Wii U GamePad** support
- Native GamePad touch support and multiple TV/GamePad display modes
- Phone calls and game audio support
- Persistent save data and progression
- Continue and Nightmare/Night 6 progression
- **Follow Me 1–5** sequences
- Secret minigames and Good Ending progression
- Multiple endings
- **Extras** menu with galleries and replayable minigames
- **Credits** directly accessible from the title screen
- **Cheats** menu with:
  - Fast Nights
  - Radar
  - Aggressive
  - No Errors
- Four persistent title stars

### Achievements

The Wii U Edition also includes its own achievement system:

- **9 normal achievements**
- Locked and unlocked achievement artwork
- Achievement notifications with dedicated sound effects
- Persistent achievement progress
- `Extras > Achievements` menu
- Progress counter from `0/9` to `9/9`
- A special final trophy called **Utine** after completing all nine achievements
- Dedicated Utine unlock presentation, animation and sound

## Installation

### Recommended — Aroma / WUHB

You need a Wii U configured to run **Aroma** homebrew.

1. Open the [v1.0 release](../../releases/tag/v1.0).
2. Download `fnaf3-wiiu-v1.0.zip`.
3. Extract the archive.
4. Copy the included `fnaf3-wiiu` folder to:

```text
SD:/wiiu/apps/
```

The final executable path should be:

```text
SD:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

5. Insert the SD card into the Wii U.
6. Start the console with Aroma.
7. Launch **Five Nights at Freddy's 3 — Wii U Edition**.

The Aroma build stores its progress in the app folder on the SD card.

### Installable WUP Channel

The v1.0 release also contains:

```text
fnaf3-wiiu-v1.0-wup.zip
```

This archive contains the encrypted Wii U Channel package (`.app`, `.h3`, ticket, certificate and TMD files). Keep every extracted file together and install the folder with a WUP installer compatible with your Wii U environment.

The Channel build uses native Wii U save storage and includes migration support for the legacy SD save when available.

The WUP package is generated and structurally validated by GitHub Actions. The Aroma/WUHB build remains the recommended fallback if a console-specific Channel installation issue is encountered.

### Cemu

The v1.0 release also provides:

```text
fnaf3-wiiu-v1.0-cemu.zip
```

This is a **decrypted Wii U title layout** made specifically for Cemu. After extraction it contains:

```text
fnaf3-wiiu-cemu/
├── code/
│   ├── fnaf3-wiiu.rpx
│   ├── app.xml
│   └── cos.xml
├── content/
└── meta/
    ├── meta.xml
    ├── iconTex.tga
    ├── bootTvTex.tga
    └── bootDrcTex.tga
```

Recommended installation:

1. Extract `fnaf3-wiiu-v1.0-cemu.zip`.
2. Open Cemu.
3. Choose **File → Install game title, update or DLC**.
4. Select:

```text
fnaf3-wiiu-cemu/meta/meta.xml
```

5. Launch **Five Nights at Freddy's 3 — Wii U Edition** from Cemu's game list.

A direct RPX launch is also possible through **File → Load** and selecting:

```text
fnaf3-wiiu-cemu/code/fnaf3-wiiu.rpx
```

The installed-title method is recommended because it gives the port the normal Wii U title context and native save path inside Cemu's MLC. The Cemu package uses the same title ID as the Wii U Channel: `000500001337F3A3`.

### Release checksums

The v1.0 release includes `SHA256SUMS.txt` for the Wii U packages and `SHA256SUMS-CEMU.txt` for the dedicated Cemu package.

## Controls

### Office

- **Left / Right:** look around
- **X / Y:** open or close cameras
- **−:** open maintenance
- **R:** mute an active phone call
- **B:** return when available

### Cameras

- **D-pad:** select camera
- **A:** use Play Audio
- **R:** switch camera / vent view
- **X / Y:** close monitor
- **−:** open maintenance

### Maintenance

- **Up / Down:** select system
- **A / +:** confirm or reboot
- **B / −:** close when available

### Minigames

- **D-pad / Left Stick:** move
- **A:** interact
- **B:** return when available

### Extras / Achievements / Cheats

- **Up / Down:** select
- **Left / Right:** browse supported pages
- **A / +:** open, replay or toggle
- **B:** back

## Validation

The v1.0 release path includes automated checks for:

- Springtrap AI fidelity
- Phantom AI fidelity
- PC-only visual/audio routing
- rare/random MFA-derived Phantom events
- Wii U WUHB/RPX compilation
- Cemu `code/content/meta` package generation and layout validation
- installable WUP generation and structural validation

The final host stress harness executes **4,147,200 synthetic 60 Hz state-transition frames across Nights 1–6** with AddressSanitizer and UndefinedBehaviorSanitizer enabled.

Automated validation does not replace every possible physical-console or emulator scenario, so hardware- and Cemu-specific bug reports remain useful.

## How the port was created

This Wii U Edition was developed as a dedicated adaptation for Nintendo Wii U.

Code for platform integration, rendering, input handling, save management, audio playback and Wii U-specific features was written or adapted specifically for this project. Existing versions of FNaF 3 were used as behavioral and visual reference where appropriate.

For copyright and project-safety reasons, the public documentation intentionally does **not** describe proprietary asset extraction procedures, internal game-data layouts, private development resources or other information that is unnecessary for players to install and use the port.

The project is designed to keep the port's own code and Wii U-specific work separate from material that should not be redistributed publicly.

## Building and development

The public project documentation does not provide proprietary game data or instructions intended to extract copyrighted game resources.

Developers working on the project are expected to provide any required legally obtained source material themselves and to respect the rights of the original creators and publishers.

The Wii U-specific codebase is built with the standard Wii U homebrew development toolchain.

## Project status

**Five Nights at Freddy's 3 — Wii U Edition v1.0 is released and is the current stable version.**

The stable release supports real Wii U through Aroma/WUHB or WUP Channel packaging and also provides a dedicated Cemu distribution.

Future development can focus on bug fixes, hardware/emulator-specific improvements and optional post-v1.0 enhancements without reopening the completed core porting milestone.

## Disclaimer

**Five Nights at Freddy's**, its characters, artwork, audio and other original game material belong to their respective rights holders.

This repository represents an **unofficial fan-made Wii U port** and is not affiliated with or endorsed by the original developers, publishers or Nintendo.

This project does not provide documentation intended to extract or reproduce proprietary game data. Original game material remains the property of its respective rights holders.
