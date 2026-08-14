# Five Nights at Freddy's 3 — Wii U Edition v1.0

This is the first **stable v1.0 release** of the Wii U Edition.

The release consolidates the PC/MFA fidelity work, real-Wii-U recovery fixes, final title/Credits integration, PC-only asset guards, rare-event tests and the final runtime stress pass.

## Highlights

- Full **Nights 1–6** progression at the final 60 Hz gameplay pacing
- PC/MFA-derived **Springtrap AI**, routes, vents, aggression and Play Audio behavior
- All six Phantom animatronics: Freddy, Foxy, Chica, Balloon Boy, Mangle and Puppet
- Camera, ventilation, audio and maintenance systems with PC-derived timing
- Wii U TV + **GamePad** rendering modes and native touch support
- Phone calls, ambience, jumpscares and minigame audio from the verified PC sound route
- Persistent progression, Continue, Nightmare and four title stars
- **Follow Me 1–5** reconstructed from the PC/MFA logic and visual data
- Secret minigames and Good Ending progression
- Bad Ending / Good Ending / Night 6 ending flow
- Extras, galleries, replayable minigames and Cheats
- Direct **Credits** entry on the title screen
- 9 achievements plus the special **Utine** trophy/presentation
- PC-only visual/audio provenance audits to prevent obsolete PSX/custom fallbacks from returning

## Validation

Before the v1.0 publication pass, the consolidated runtime on `main` passed:

- Springtrap AI Fidelity Tests
- Phantom AI Fidelity Tests
- Final Fidelity and Stress Tests
- Wii U WUHB/RPX build and output verification
- installable WUP generation and structural validation

The stress harness executes **4,147,200 synthetic 60 Hz state-transition frames across Nights 1–6** with AddressSanitizer and UndefinedBehaviorSanitizer enabled.

The v1.0 publishing workflow only creates the release after a fresh **Build Wii U Homebrew** run on `main` completes successfully.

## Installation — Aroma / WUHB

Recommended for most users.

1. Download `fnaf3-wiiu-v1.0.zip` or the standalone `fnaf3-wiiu.wuhb`.
2. Copy the `fnaf3-wiiu` folder to:

   `SD:/wiiu/apps/`

3. The final path should be:

   `SD:/wiiu/apps/fnaf3-wiiu/fnaf3-wiiu.wuhb`

4. Start Aroma and launch **Five Nights at Freddy's 3 — Wii U Edition**.

The Aroma build stores its progress in the app folder on the SD card.

## Installation — WUP Channel

v1.0 also includes `fnaf3-wiiu-v1.0-wup.zip`.

The archive contains the complete encrypted `.app/.h3` Wii U Channel package together with its ticket/certificate metadata. Keep every file from the archive together and install the package with a WUP installer compatible with your Wii U environment.

The Channel build uses native Wii U save storage and includes migration support for the legacy SD save when available.

The WUP is generated and structurally validated by CI. Because physical installation behavior can still depend on the console environment, keep the Aroma/WUHB build available as the recovery/fallback option and report any Channel-specific issue.

## Included release files

- `fnaf3-wiiu-v1.0.zip` — ready-to-copy Aroma package
- `fnaf3-wiiu.wuhb` — standalone Aroma executable
- `fnaf3-wiiu.rpx` — RPX build
- `fnaf3-wiiu-v1.0-wup.zip` — installable Channel package
- `SHA256SUMS.txt` — release checksums

## Upgrade from v0.90-alpha

Existing progression is preserved by the normal save system. The Channel build also retains the migration path from the legacy SD save where available.

## Disclaimer

This is an unofficial fan-made Wii U port and is not affiliated with or endorsed by the original developers, publishers, Nintendo, or other rights holders. Original game material remains the property of its respective rights holders.
