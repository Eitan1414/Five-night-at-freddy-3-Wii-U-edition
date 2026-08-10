# Five Nights at Freddy's 3 — Wii U Edition

An unofficial **Five Nights at Freddy's 3** port made specifically for the **Nintendo Wii U**, with support for **Aroma**, the Wii U GamePad and a number of features created especially for this edition.

> This is a fan-made project. It is not affiliated with, endorsed by, or supported by the original developers, publishers, Nintendo, or the respective rights holders.

## About the Wii U Edition

The goal of this project is to make FNaF 3 feel at home on Wii U rather than simply placing the game on another platform.

The port includes Wii U-specific controls, TV/GamePad rendering, adapted menus, persistent progression, additional Extras content and several quality-of-life improvements while keeping the core FNaF 3 experience recognizable.

## Features

- Full **Night 1–6** progression
- Springtrap AI, ventilation routes, audio lure and system failures
- **Phantom Freddy, Foxy, Chica, Balloon Boy, Mangle and Puppet**
- Camera and maintenance systems adapted for Wii U controls
- TV + **Wii U GamePad** support
- Phone calls and game audio support
- Persistent save data and progression
- Follow Me sequences and secret minigames
- Multiple endings, including the Good Ending progression
- **Extras** menu with additional content
- Animatronic and jumpscare galleries
- Secret Minigame replay menu
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

The additional UI uses a visual style designed to blend with FNaF 3 while keeping the original Extras presentation intact.

## Installation

### Recommended: Aroma / WUHB

You need a Wii U configured to run **Aroma** homebrew.

1. Download the Wii U Edition release package.
2. Open your Wii U SD card on a computer.
3. Copy the provided `fnaf3-wiiu` folder into:

```text
SD:/wiiu/apps/
```

The final structure should contain at least:

```text
SD:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb
```

If the release contains additional files or folders, keep their original structure and copy them alongside the `.wuhb` file.

4. Safely eject the SD card.
5. Insert it into the Wii U.
6. Start the console with Aroma.
7. Launch **Five Nights at Freddy's 3 — Wii U Edition** from the Wii U homebrew environment.

Save data is handled automatically by the port.

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

## How the port was created

This Wii U Edition was developed as a dedicated adaptation for Nintendo Wii U.

Code for platform integration, rendering, input handling, save management, audio playback and Wii U-specific features was written or adapted specifically for this project. Existing versions of FNaF 3 were used as behavioural and visual reference where appropriate.

For copyright and project-safety reasons, the public documentation intentionally does **not** describe proprietary asset extraction procedures, internal game-data layouts, private development resources or other information that is unnecessary for players to install and use the port.

The project is designed to keep the port's own code and Wii U-specific work separate from material that should not be redistributed publicly.

## Building and development

The public project documentation does not provide proprietary game data or instructions intended to extract copyrighted game resources.

Developers working on the project are expected to provide any required legally obtained source material themselves and to respect the rights of the original creators and publishers.

The Wii U-specific codebase is built with the standard Wii U homebrew development toolchain.

## Project status

The port currently includes the complete main gameplay flow, Extras content, secret progression, achievements and Wii U-specific presentation systems.

Development is now primarily focused on final polish, hardware testing, small visual adjustments and release preparation.

## Disclaimer

**Five Nights at Freddy's**, its characters, artwork, audio and other original game material belong to their respective rights holders.

This repository represents an **unofficial fan-made Wii U port** and is not affiliated with or endorsed by the original developers, publishers or Nintendo.

This project does not provide documentation intended to extract or reproduce proprietary game data. Original game material remains the property of its respective rights holders.
