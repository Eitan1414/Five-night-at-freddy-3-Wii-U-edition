# Automatic save system

The Wii U port automatically saves progression when a night is completed.

## Saved progression

- highest unlocked night, from Night 1 through Night 6;
- a completion bit for each of the six nights, reserved for future Extras and endings;
- a format version and checksum for corruption detection.

The active office state is intentionally not saved. Loading always starts the highest unlocked night from 12:00 AM, so the player cannot resume inside a broken or dangerous mid-night state.

## Files

The game stores its data beside the application on the SD card:

```text
SD:/wiiu/apps/fnaf3-wiiu/progress.dat
SD:/wiiu/apps/fnaf3-wiiu/progress.dat.bak
```

Writes are atomic: a new `progress.dat.tmp` is completed first, the previous save becomes the backup, and only then is the new file promoted to the main save.

At startup the main file is validated. If it is missing or corrupt, the backup is tried automatically. Invalid data never unlocks an out-of-range night.

## Menu behavior

- **NEW GAME** starts Night 1 without erasing unlocked progression.
- **LOAD GAME** starts the highest unlocked night.
- The title shows whether autosave is ready, unavailable, empty, corrupt, or recovered from backup.
- The victory screen confirms whether progression was saved successfully.
